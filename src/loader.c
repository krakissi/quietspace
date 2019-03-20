#include <stdio.h>
#include <sys/types.h>
#include <regex.h>

#include "base64.h"

#include "loader.h"

char *assets[] = {
	#include "assets_enc/scene_dorm"
	,
	#include "assets_enc/scene_lift_doors"
	,
	NULL
};

// Match lines that look like comments.
int rx_comment(char *str){
	static regex_t *rx = NULL;

	if(!rx){
		rx = malloc(sizeof(regex_t));
		regcomp(rx, "^\\s*#.*$", REG_NOSUB);
	}

	return regexec(rx, str, 0, NULL, 0);
}

// Match lines that are empty.
int rx_space(char *str){
	static regex_t *rx = NULL;

	if(!rx){
		rx = malloc(sizeof(regex_t));
		regcomp(rx, "^\\s*$", REG_NOSUB);
	}

	return regexec(rx, str, 0, NULL, 0);
}

// Match lines that look like key = value.
int rx_kv(regmatch_t pmatch[], char *str){
	static regex_t *rx = NULL;
	size_t nmatch = 3;

	if(!rx){
		rx = malloc(sizeof(regex_t));
		regcomp(rx, "^\\s*\\(.*\\)\\s*=\\s*\\(.*\\)$", 0);
	}

	int rc = regexec(rx, str, nmatch, pmatch, 0);

	return rc;
}

// Copy the word that was matched from the regular expression.
char *get_rx_match_str(char *str, regmatch_t match){
	int len = match.rm_eo - match.rm_so - 1;
	char *ret = calloc(len + 1, sizeof(char));

	strncpy(ret, str + match.rm_so, len);
	ret[len] = 0;

	return ret;
}

// Load scene data into a buffer.
char *get_scene_str(FILE *stream){
	size_t n = 4096;
	char *ret = calloc(n, sizeof(char)), *buf = calloc(n, sizeof(char)), *s;
	FILE *dest = fmemopen(ret, n, "w");

	while(fgets(buf, n, stream)){
		// Remove line endings that may presently exist.
		if((s = strpbrk(buf, "\r\n")))
			*s = 0;

		// End of the scene when a dot is alone on a line.
		if(!strcmp(buf, "."))
			break;

		// Write the line out with internet standard cr-lf.
		fprintf(dest, "%s\r\n", buf);
	}

	fclose(dest);
	free(buf);
	return ret;
}

// Add the blob kv to the kv_tree in the next open space.
asset_kv *kv_tree_add(asset_kv *kv_tree, asset_kv *kv){
	// Walk the tree and find a place for this key name.
	if(strcmp(kv_tree->key, kv->key) < 0){
		// Left.
		if(kv_tree->l)
			return kv_tree_add(kv_tree->l, kv);
		else
			return kv_tree->l = kv;
	} else {
		// Right.
		if(kv_tree->r)
			return kv_tree_add(kv_tree->r, kv);
		else
			return kv_tree->r = kv;
	}
}

// Find the kv blob with specified key name.
asset_kv *kv_tree_find(asset_kv *kv_tree, char *key){
	if(!kv_tree)
		return kv_tree;

	int cmp = strcmp(kv_tree->key, key);

	if(!cmp)
		return kv_tree;

	if(cmp < 0)
		return kv_tree_find(kv_tree->l, key);

	return kv_tree_find(kv_tree->r, key);
}

// Add the asset to the asset_tree in the next open space.
asset *asset_tree_add(asset *asset_tree, asset *asset){
	// Walk the tree and find a place for this name.
	if(strcmp(asset_tree->name, asset->name) < 0){
		// Left.
		if(asset_tree->l)
			return asset_tree_add(asset_tree->l, asset);
		else
			return asset_tree->l = asset;
	} else {
		// Right.
		if(asset_tree->r)
			return asset_tree_add(asset_tree->r, asset);
		else
			return asset_tree->r = asset;
	}
}

// Find the asset with the specified name.
asset *asset_tree_find(asset *asset_tree, char *name){
	if(!asset_tree)
		return asset_tree;

	int cmp = strcmp(asset_tree->name, name);

	if(!cmp)
		return asset_tree;

	if(cmp < 0)
		return asset_tree_find(asset_tree->l, name);

	return asset_tree_find(asset_tree->r, name);
}

asset *asset_load(){
	asset *as, *as_tree = malloc(sizeof(asset));
	asset_kv *kv;
	char **b64 = assets;

	as_tree->name = NULL;
	as_tree->l = as_tree->r = NULL;

	size_t n = 4096;
	char *s = calloc(n, sizeof(char));

	while(*b64){
		asset_kv *kv_tree = malloc(sizeof(asset_kv));
		char *gfx = base64_dec(*b64, strlen(*b64));

		FILE *source = fmemopen(gfx, strlen(gfx), "r");
		regmatch_t match_kv[3];

		if(!source)
			continue;

		kv_tree->l = kv_tree->r = NULL;
		kv_tree->key = NULL;
		while(fgets(s, n, source)){
			// Don't read comments
			if(!rx_comment(s) || !rx_space(s))
				continue;

			// Read each line, looking for "\s*key\s*=\s*value"
			if(!rx_kv(match_kv, s)){
				char *key = get_rx_match_str(s, match_kv[1]);
				// If kv_tree is NULL, this is the first element we've seen.
				if(!kv_tree->key){
					kv = kv_tree;
					kv->key = key;
				} else {
					// Create a new node.
					kv = malloc(sizeof(asset_kv));
					kv->l = kv->r = NULL;
					kv->key = key;

					// Walk the tree and find a place for this key name.
					kv_tree_add(kv_tree, kv);
				}

				char *val = get_rx_match_str(s, match_kv[2]);
				char *a;

				if(*val == '"'){
					// A literal string object, for descriptions and names.
					kv->type = AVT_STRING;

					kv->value.str = calloc(strlen(val), sizeof(char));
					strcpy(kv->value.str, val + 1);
					if((a = strrchr(kv->value.str, '"')))
						*a = 0;
				} else if(!strcmp(val, "scene")){
					// Scene object, meant to be drawn on screen as graphics.
					kv->type = AVT_SCENE;

					// Parse scene object
					kv->value.str = get_scene_str(source);
				} else if(!strcmp(val, "overlay")){
					// Scene overlay, a scene object where spaces are rendered invisible.
					kv->type = AVT_SCENE_OVERLAY;

					// Parse scene overlay object
					kv->value.str = get_scene_str(source);
				} else if(strchr(val, '.')){
					// Looks like a floating point number.
					kv->type = AVT_FLOAT;
					kv->value.f = atof(val);
				} else {
					// Integer values are the last guess.
					kv->type = AVT_INTEGER;
					kv->value.i = atoi(val);
				}

				free(val);
			}
		}

		fclose(source);
		free(gfx);
		b64++;

		kv = kv_tree_find(kv_tree, "name");
		if(!kv)
			continue;

		if(!as_tree->name){
			as = as_tree;

			as->name = kv->value.str;
			as->kv_tree = kv_tree;
		} else {
			as = malloc(sizeof(asset));
			as->l = as->r = NULL;

			as->name = kv->value.str;
			as->kv_tree = kv_tree;

			asset_tree_add(as_tree, as);
		}
	}
	free(s);

	return as_tree;
}
