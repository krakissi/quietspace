#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <regex.h>

#include "qsfmt.h"
#include "generic.h"

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
	const long sleep_regular = (5 * 1000 * 1000), sleep_medium = (100 * 1000 * 1000), sleep_long  = (250 * 1000 * 1000);

	va_start(va, fmt);
	int total = vsprintf(buffer, fmt, va);
	va_end(va);

	struct timespec t = {
		0,
		sleep_regular
	};

	for(a = buffer; *a; a++){
		if(i == 0)
			fprintf(stream, "\033[0m");

		// Skip spaces at the start of a line.
		while((i == 0) && (*a == ' '))
			a++;

		fputc(*a, stream);
		fflush(stream);

		// Pause differently for punctuation.
		if((*a == '.') || (*a == '!') || (*a == '?'))
			t.tv_nsec = sleep_long;
		else if(*a == ',')
			t.tv_nsec = sleep_medium;
		else
			t.tv_nsec = sleep_regular;

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

void cursor_position_response(FILE *stream){
	fputs("\033[18H\033[J\033[0m", stream);
}

ssize_t read_cmd(char **str_p, size_t *n_p, FILE *stream, const char *ps, const char *ps_perm, const char *ps_col){
	ssize_t rd;

	goto prompt;
	while((rd = getline(str_p, n_p, stream)) != -1){
		// Remove trailing return/feed characters.
		sanitize_str(*str_p);

		// Ignore empty command lines.
		if(**str_p){
			fputs("\033[0m", stream);
			break;
		}

prompt:
		// Print out the prompt and await a command.
		fprintf(stream, "\033[23H\033[J\033[0m/%s %s%s ", ps, ps_perm, ps_col);
	}

	return rd;
}

ssize_t read_cmd_bounded(
	char **str_p, size_t *n_p, FILE *stream,
	const char *ps, const char *ps_perm, const char *ps_col,
	const char *msg_base, const char *msg_error,
	const regex_t *expr
){
	ssize_t rd;

	cursor_position_response(stream);
	text_type(stream, msg_base);

	if((rd = read_cmd(str_p, n_p, stream, ps, ps_perm, ps_col) == -1))
		return rd;

	if(expr)
		while(regexec(expr, *str_p, 0, NULL, 0)){
			cursor_position_response(stream);
			text_type(stream, msg_error);

			if((rd = read_cmd(str_p, n_p, stream, ps, ps_perm, ps_col) == -1))
				break;
		}

	return rd;
}

