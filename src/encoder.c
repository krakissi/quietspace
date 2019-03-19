/*
	encoder.c
	mperron (2019)

	Reads from the file specified in the first argument and writes base64 to
	stdout in a format that can be #included into C source.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base64.h"

#define READ_BLOCK_SIZE 256

int main(int argc, char **argv){
	char buffer[READ_BLOCK_SIZE];
	unsigned char *data = NULL;
	size_t n = 0, r;

	if(argc < 2){
		fprintf(stderr, "Usage:\n\t%s <file>\n", *argv);
		return 0;
	}

	FILE *source = fopen(argv[1], "r");

	if(!source){
		fprintf(stderr, "File not found: %s\n", argv[1]);
		return 1;
	}

	while((r = fread(buffer, sizeof(char), READ_BLOCK_SIZE, source))){
		n += r;
		data = realloc(data, n * sizeof(char));

		if(!data){
			fprintf(stderr, "Failed to realloc input holder!\n");
			return 2;
		}

		memcpy(data + n - r, buffer, r);
	}

	fclose(source);

	char *enc = base64_enc(data, n);

	base64_toquoted(enc, stdout);
	free(enc);

	return 0;
}
