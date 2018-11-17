/*
	generic
	Written by Mike Perron (2012-2013)

	This Source Code Form is subject to the terms of the Mozilla Public
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at http://mozilla.org/MPL/2.0/.

	General functions to patch things together for krakws.
*/
#ifndef KWS_GENERIC_F
#define KWS_GENERIC_F

#define WORDS_DELIMINATOR " \t\r\n"

extern int error_code(int code, const char *msg, ...);

// String functions
extern char **chop_words(const char *src);
extern void sanitize_str(char *str);
extern void unquote_str(char *str);

// Misc.
extern int kws_fclose(FILE **stream);

// Compute the SHA512 hash of a string. Allocates a buffer that should be freed.
extern char *hash_compute(FILE *stream, char *pass);

#endif
