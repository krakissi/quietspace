#define _GNU_SOURCE

#include <stdio.h>

#include "dbpersistence.h"

#include "qsmap.h"

const char map_station[QS_MAP_H][QS_MAP_W] = {
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	{ -1, -1, -1, -1,  2,  2,  2,  2, -1, -1, -1,  6,  6, -1 },
	{ -1, -1, -1, -1,  2, 52,  2,  2, -1, -1,  6, 56,  6, -1 },
	{ -1, -1,  3,  3, -1,  2,  2,  0,  0,  0,  6,  6, -1, -1 },
	{ -1,  3, 53,  3, -1, -1, -1,  0, -1,  6,  6, -1, -1, -1 },
	{ -1,  3,  3,  3,  0,  1,  1,  1, -1, -1, -1, -1, -1, -1 },
	{ -1, -1,  3,  3, -1,  1,  1,  1,  0,  0,  5,  5, -1, -1 },
	{ -1,  3,  3,  3, -1, 51,  1,  1, -1,  5, 55,  5, -1, -1 },
	{ -1, -1,  3,  0, -1, -1, -1, -1, -1, -1,  5,  0, -1, -1 },
	{ -1, -1, -1,  0, -1,  4,  4, -1,  4,  4, -1,  0, -1, -1 },
	{ -1, -1, -1,  4,  4,  4,  4,  4,  4,  4,  4,  4, -1, -1 },
	{ -1, -1, -1,  4, 54,  4,  4,  4,  4,  4,  4,  4, -1, -1 },
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 }
};

void map_draw(FILE *stream, player *pl){
	int row = 1, col = 50;
	int i, j;
	char c, label;

	char col_bg = 47, col_bg_prev = 0;

	for(i = 0; i < QS_MAP_H; i++){
		fprintf(stream, "\033[%d;%dH", i + row, col);

		for(j = 0; j < QS_MAP_W; j++){
			c = map_station[i][j];
			label = 0;

			if(c == -1)
				col_bg = 47;
			else {
				if(c > 50)
					label = c = (c - 50);

				col_bg = (c + 40);
			}

			if(col_bg != col_bg_prev){
				fprintf(stream, "\033[0;30;%dm", col_bg);
				col_bg_prev = col_bg;
			}

			if(label)
				fprintf(stream, "%2d", label);
			else
				fputs("  ", stream);
		}
	}

	fputs("\033[0m", stream);
}
