#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "qsfmt.h"

/*
	Draw borders, probably for demonstration's sake, using ANSI escape
	sequences.
*/
void draw_borders(FILE *stream, int row, int col, int w, int h){
	fprintf(stream, "\033[%d;%dH", row, col);

	fputs("╒", stream);
	for(int i = 1; i < (w - 1); i++)
		fputs("═", stream);
	fputs("╕", stream);

	for(int i = 1; i < (h - 1); i++)
		fprintf(stream, "\033[%d;%dH│\033[%dC│", (row + i), col, (w - 2));

	fprintf(stream, "\033[%d;%dH", (row + h - 1), col);

	fputs("╘", stream);
	for(int i = 1; i < (w - 1); i++)
		fputs("═", stream);
	fputs("╛", stream);
}
