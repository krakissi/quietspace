#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#include "qsfmt.h"

/*
	Draw borders, probably for demonstration's sake, using ANSI escape
	sequences.
*/
void draw_borders(FILE *stream, int row, int col, int w, int h){

	if(row > 0){
		fprintf(stream, "\033[%d;%dH╒", row, col);

		for(int i = 1; i < (w - 1); i++)
			fputs("═", stream);

		fputs("╕", stream);
	}

	for(int i = 1; i < (h - 1); i++)
		fprintf(stream, "\033[%d;%dH│\033[%dC│", (row + i), col, (w - 2));

	fprintf(stream, "\033[%d;%dH╘", (row + h - 1), col);

	for(int i = 1; i < (w - 1); i++)
		fputs("═", stream);

	fputs("╛", stream);
}

int text_type(FILE *stream, const char *fmt, ...){
	char *buffer = calloc(2048, sizeof(char)), *a, *b;
	va_list va;
	int i = 0, wrap_min = 72, wrap_max = 78;

	va_start(va, fmt);
	int total = vsprintf(buffer, fmt, va);
	va_end(va);

	struct timespec t = {
		0,
		(7 * 1000 * 1000)
	};

	for(a = buffer; *a; a++){
		if(i == 0)
			fprintf(stream, "\033[0m");

		// Skip spaces at the start of a line.
		while((i == 0) && (*a == ' '))
			a++;

		fputc(*a, stream);
		fflush(stream);
		nanosleep(&t, NULL);

		if((++i >= wrap_min) && (*a != '\\') && (*(a - 1) != '\\')){
			// Break on the first space after the minimum wrap width.
			if(*a == ' '){
				// If there's another space before the maximum wrap width, delay break until then.
				for(b = a + 1; *b && (b < a + (wrap_max - i)); b++)
					if(*b == ' ')
						break;

				// No second space, insert the break.
				if(*b != ' '){
					fputs("\r\n", stream);
					i = 0;
				}
			} else if(i >= wrap_max){
				if(*(a + 1) == ' ')
					a++;
				else if(*(a + 2) == ' ')
					fputc(*(++a), stream);
				else
					fputc('-', stream);

				fputs("\r\n", stream);
				i = 0;
			}
		}
	}

	free(buffer);

	return total;
}
