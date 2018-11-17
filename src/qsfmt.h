/*
	qsfmt
	mperron (2018)

	Text output formatting functions for Quiet Space.
*/
#ifndef QS_FMT_H
#define QS_FMT_H

extern void draw_borders(FILE*, int, int, int, int);
extern int text_type(FILE*, const char*, ...);

extern void cursor_position_response(FILE *stream);

extern ssize_t read_cmd(
	char **str_p, size_t *n_p, FILE *stream,
	const char *ps, const char *ps_perm, const char *ps_col
);

extern ssize_t read_cmd_bounded(
	char **str_p, size_t *n_p, FILE *stream,
	const char *ps, const char *ps_perm, const char *ps_col,
	const char *msg_base, const char *msg_error,
	const regex_t *expr
);

#endif
