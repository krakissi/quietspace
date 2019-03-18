#include <stdio.h>
#include <sys/types.h>
#include <regex.h>

#include "base64.h"

/*
	Decode the base64 graphics data and return a newly allocated buffer which must be freed.
*/
char *load_graphics(char *graphics){
	return base64_dec(graphics, strlen(graphics));
}

char *scene_desk = (
#include "assets_enc/scene_desk"
);

int rx_comment(char *str){
	static regex_t *rx = NULL;
	size_t nmatch = 1;
	regmatch_t pmatch[1];

	if(!rx){
		rx = malloc(sizeof(regex_t));
		regcomp(rx, "^\\s*#.*$", 0);
	}

	return regexec(rx, str, nmatch, pmatch, 0);
}

int main(){
	
	char *gfx = load_graphics(scene_desk);

	size_t n = 4096;
	char *s = calloc(n, sizeof(char));

	FILE *source = fmemopen(gfx, strlen(gfx), "r");

	if(!source){
		fprintf(stderr, "Failed to open decoded data as a file.\n");
		return 1;
	}

	while(fgets(s, n, source)){
		// Don't read comments
		if(!rx_comment(s))
			continue;

		// Read each line, looking for "\s*key\s*=\s*value"
		// TODO

		// Store the values in some kind of map for this object.
		// TODO

		// If value is scene or overlay, process subsequent lines as data for
		// that key, until a line with just a period appears. (Email style
		// termination.)
		// TODO

		// FIXME debug
		printf("%s", s);
	}

	fclose(source);

	return 0;
}
