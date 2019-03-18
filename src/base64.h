/*
	Encodes data, using a modified base-64 algorithm which uses the 64 ASCII
	characters from 0x20 to 0x5F (excluding double-quote and backslash which
	are replaced with open and close curly braces, respectively). Excess bytes
	are padded with tildes.
	
	This mechanism requires no lookup table, but produces non-standard base64
	data.
*/

#include <stdlib.h>
#include <string.h>

static inline char byte_enc(char in){
	char out = in + 32;

	switch(out){
		case '"':
			return '{';
		case '\\':
			return '}';
	}

	return out;
}
static inline char byte_dec(char in){
	char out = in;

	switch(out){
		case '~':
			return 0;
		case '{':
			out = '"';
			break;
		case '}':
			out = '\\';
			break;
	}

	return out - 32;
}

/*
	Writes out base64-encoded data (or any normal readable string) in
	72-character quoted sections.
*/
void base64_toquoted(char *data, FILE *stream){
	int count = 0;

	fputc('"', stream);

	while(*data){
		fputc(*(data++), stream);

		if(++count == 72){
			fputs("\"\n", stream);
			count = 0;

			if(*data)
				fputc('"', stream);
		}
	}
	if(count > 0)
		fputs("\"\n", stream);
}

char *base64_enc(unsigned char *data, size_t len_in){
	char *data_enc = NULL, *out;
	int i;

	out = data_enc = calloc(4 * ((len_in + 2) / 3) + 1, sizeof(char));

	for(i = 0; i < len_in;){
		unsigned long bytes[3] = { 0, 0, 0 }, triple;

		for(int j = 0; j < 3; j++, i++)
			if(i < len_in)
				bytes[j] = *(data++);

		triple = (bytes[0] << 0x10) + (bytes[1] << 0x8) + bytes[2];

		for(int j = 3; j >= 0; j--)
			*(out++) = byte_enc((triple >> j * 6) & 0x3f);
	}
	*out = 0;
	while((i-- - len_in) > 0)
		*(--out) = '~';

	return data_enc;
}

char *base64_dec(char *data, size_t len_in){
	char *data_dec = NULL, *out;
	size_t len_out = len_in / 4 * 3;

	out = data + len_in;
	while(*(out--) == '~')
		len_out--;

	out = data_dec = calloc(len_out + 1, sizeof(char));

	for(int i = 0; i < len_in;){
		unsigned long bytes[4] = { 0, 0, 0, 0 }, triple;

		for(int j = 0; j < 4; j++, i++, data++)
			bytes[j] = byte_dec(*data);

		triple = 0;
		for(int j = 0; j < 4; j++)
			triple += bytes[j] << (3 - j) * 6;

		for(int j = 2; j >= 0; j--)
			if(out < data_dec + len_out)
				*(out++) = (triple >> j * 8) & 0xff;
	}

	*out = 0;

	return data_dec;
}
