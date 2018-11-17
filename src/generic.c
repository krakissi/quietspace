/*
	generic
	Written by Mike Perron (2012-2013)

	This Source Code Form is subject to the terms of the Mozilla Public
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at http://mozilla.org/MPL/2.0/.

	General functions to patch things together for krakws.

	unescape_url and x2c are borrowed from the NCSA HTTPD server example
	CGI application.
*/
#define _GNU_SOURCE

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "generic.h"

/*	Prints the message with a timestamp to stderr. Returns the code value
	specified. */
int error_code(int code, const char *msg, ...){
	va_list va;

	va_start(va, msg);
	if(msg == strstr(msg, "--")){
		vfprintf(stderr, msg + 2, va);
	} else {
		fprintf(stderr, "error: ");
		vfprintf(stderr, msg, va);
	}
	fputc('\n', stderr);
	va_end(va);

	fflush(stderr);
	return code;
}

/*	Splits words at spaces and tabs. This function creates a copy of the
	original string. The first word may be the same as the second, and should
	only be used to free memory. The last pointer will be NULL, signifying the
	end of the string. */
char **chop_words(const char *src){
	char **v = NULL, *str, *r;
	int count = 2;

	if(!src)
		return NULL;

	str = calloc(1 + strlen(src), sizeof(char));
	*(v = malloc(count * sizeof(char**))) = str;
	strcpy(str, src);

	*(v + count - 1) = strtok_r(str, WORDS_DELIMINATOR, &r);
	do v = realloc(v, ++count * sizeof(char**));
	while((*(v + count - 1) = strtok_r(NULL, WORDS_DELIMINATOR, &r)));

	return v;
}

void sanitize_str(char *str){
	if((str = strpbrk(str, "\r\n")))
		*str = 0;
}

void unquote_str(char *str){
	size_t l;

	if(!str || ((l = strlen(str)) < 2))
		return;

	if(*str == '\"')
		memmove(str, str + 1, l--);

	if(str[--l] == '\"')
		str[l] = 0;
}

int kws_fclose(FILE **stream){
	FILE *f = *stream;

	*stream = NULL;
	return f ? fclose(f) : 0;
}

char *hash_compute(FILE *stream, char *pass){
	char *hash = NULL, cmd[1024], *a, *b;
	size_t n;
	FILE *p;

	const char *cmd_pre = "echo '";
	const char *cmd_post = "' | openssl sha512";

	strcpy(cmd, cmd_pre);
	for(a = pass, b = cmd + strlen(cmd_pre); *a; a++){
		if(*a == '\''){
			strcpy(b, "'\"'\"'");
			b += 5;
		} else
			*(b++) = *a;
	}
	strcpy(b, cmd_post);

	if(!(p = popen(cmd, "r")))
		goto fail;

	getline(&hash, &n, p);
	sanitize_str(hash);

	// Skip "(stdin)= " returned by openssl tool.
	for(a = (hash + strlen("(stdin)= ")), b = hash; *a; a++)
		*(b++) = *a;
	*b = 0;

	pclose(p);
out:
	return hash;
fail:
	free(hash);
	hash = NULL;
	goto out;
}
