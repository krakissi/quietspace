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

void kv_set_value(FILE*, union asset_value*, enum asset_type, char*);

// Match lines that look like comments.
int rx_comment(char *str){
	static regex_t *rx = NULL;

	if(!rx){
		rx = malloc(sizeof(regex_t));
		regcomp(rx, "^\\s*#.*$", REG_NOSUB);
	}

	return regexec(rx, str, 0, NULL, 0);
}

// End of a nested k-v map.
int rx_brace_close(char *str){
	static regex_t *rx = NULL;

	if(!rx){
		rx = malloc(sizeof(regex_t));
		regcomp(rx, "^\\s*}\\s*$", REG_NOSUB);
	}

	return regexec(rx, str, 0, NULL, 0);
}

// End of nested array.
int rx_bracket_close(char *str){
	static regex_t *rx = NULL;

	if(!rx){
		rx = malloc(sizeof(regex_t));
		regcomp(rx, "^\\s*]\\s*$", REG_NOSUB);
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

// Match lines that look like a value.
int rx_v(regmatch_t pmatch[], char *str){
	static regex_t *rx = NULL;
	size_t nmatch = 2;

	if(!rx){
		rx = malloc(sizeof(regex_t));
		regcomp(rx, "^\\s*\\(.*\\)\\s*$", 0);
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

// Figure out the type of the value string.
int kv_typeof(char *val){
	switch(*val){
		case '"':
			// A literal string object, for descriptions and names.
			return AVT_STRING;

		case '{':
			// Nested key-value pairs.
			return AVT_KV;

		case '[':
			// Nested array of values.
			return AVT_ARR;
	}

	if(!strcmp(val, "scene"))
		// Scene object, meant to be drawn on screen as graphics.
		return AVT_SCENE;

	if(!strcmp(val, "overlay"))
		// Scene overlay, a scene object where spaces are rendered invisible.
		return AVT_SCENE_OVERLAY;

	if(strchr(val, '.'))
		// Looks like a floating point number.
		return AVT_FLOAT;

	// Integer values are the last guess.
	return AVT_INTEGER;
}

asset_arr *read_arr(FILE *source){
	asset_arr *arr, *arr_root = malloc(sizeof(asset_arr));
	regmatch_t match_arr[2];

	size_t n = 4096;
	char *s = calloc(n, sizeof(char));

	arr_root->type = AVT_UNSET;
	arr_root->n = NULL;
	arr = arr_root;

	while(fgets(s, n, source)){
		// Don't read comments
		if(!rx_comment(s) || !rx_space(s))
			continue;

		// Close bracket means stop parsing.
		if(!rx_bracket_close(s))
			break;

		// Read each line, looking for "\s*value\s*
		if(!rx_v(match_arr, s)){
			if(arr_root->type == AVT_UNSET){
				arr = arr_root;
			} else {
				arr = arr->n = malloc(sizeof(asset_arr));
				arr->n = NULL;
			}

			char *val = get_rx_match_str(s, match_arr[1]);

			arr->type = kv_typeof(val);
			kv_set_value(source, &(arr->value), arr->type, val);

			free(val);
		}
	}

	free(s);

	return arr_root;
}

asset_kv *read_kv(FILE *source){
	asset_kv *kv, *kv_tree = malloc(sizeof(asset_kv));
	regmatch_t match_kv[3];

	size_t n = 4096;
	char *s = calloc(n, sizeof(char));

	kv_tree->l = kv_tree->r = NULL;
	kv_tree->key = NULL;
	while(fgets(s, n, source)){
		// Don't read comments
		if(!rx_comment(s) || !rx_space(s))
			continue;

		// Close brace means stop parsing.
		if(!rx_brace_close(s))
			break;

		// Read each line, looking for "\s*key\s*=\s*value
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

			kv->type = kv_typeof(val);
			kv_set_value(source, &(kv->value), kv->type, val);

			free(val);
		}
	}

	free(s);

	return kv_tree;
}

void kv_set_value(FILE *source, union asset_value *value, enum asset_type type, char *val){
	char *a;

	switch(type){
		case AVT_STRING:
			value->str = calloc(strlen(val), sizeof(char));
			strcpy(value->str, val + 1);

			// Remove a close double-quote if it's present.
			if((a = strrchr(value->str, '"')))
				*a = 0;
			break;

		case AVT_KV:
			value->kv = read_kv(source);
			break;

		case AVT_ARR:
			value->arr = read_arr(source);
			break;

		case AVT_SCENE:
		case AVT_SCENE_OVERLAY:
			value->str = get_scene_str(source);
			break;

		case AVT_FLOAT:
			value->f = atof(val);
			break;

		case AVT_INTEGER:
			value->i = atoi(val);
			break;

		case AVT_UNSET:
			// Shouldn't be used, but we'll zero the value just in case.
			value->i = 0;
			break;
	}
}


// Load all the assets and return a binary tree of them.
asset *asset_load(){
	asset *as, *as_tree = malloc(sizeof(asset));
	asset_kv *kv;
	char **b64 = assets;

	as_tree->name = NULL;
	as_tree->l = as_tree->r = NULL;

	while(*b64){
		char *gfx = base64_dec(*b64, strlen(*b64));
		FILE *source = fmemopen(gfx, strlen(gfx), "r");

		if(!source)
			continue;

		// Read source as a file and parse key-value pairs.
		asset_kv *kv_tree = read_kv(source);

		fclose(source);
		free(gfx);
		b64++;

		// Add the parsed asset key-value pair to this asset.
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

	return as_tree;
}
