#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <regex.h>

#include <netinet/in.h>
#include <sys/wait.h>

#include "defs.h"

#include "generic.h"
#include "qsfmt.h"
#include "dbpersistence.h"

#include "handler.h"

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

player *game_join(FILE *stream){
	player *pl = malloc(sizeof(player));

	const char *prompt_choose = GAME_JOIN_CHOOSE GAME_JOIN_BASE;
	const char *prompt_toolong = GAME_JOIN_TOOLONG GAME_JOIN_BASE;
	const char *prompt_pass = "Ok \033[0;31m%s\033[0m, what's your password?";
	const char *ps_perm = "$";
	char ps[16], ps_col[16];
	char *str = NULL, *a = NULL;
	size_t n;

	// Rules defining valid names and passwords.
	regex_t regex_name, regex_pass;
	regcomp(&regex_name, "^[A-Za-z]\\{2,8\\}$", 0);
	regcomp(&regex_pass, "^.\\{4,64\\}$", 0);

	*ps_col = 0;
	strcpy(ps, "join");

	if(read_cmd_bounded(
		&str, &n, stream,
		ps, ps_perm, ps_col,
		prompt_choose, prompt_toolong,
		&regex_name
	) == -1)
		goto fail;

	strcpy(pl->nick, str);
	for(a = str; *a; a++)
		*a = tolower(*a);
	strcpy(pl->name, str);

	a = calloc(strlen(prompt_pass) + QS_LEN_NAME, sizeof(char));
	sprintf(a, prompt_pass, pl->nick);
	strcpy(ps_col, "\033[8m");
	strcat(ps, "/pass");

	if(read_cmd_bounded(
		&str, &n, stream,
		ps, ps_perm, ps_col,
		a, "Your password should be at least five, but not more than 64 characters.",
		&regex_pass
	) == -1)
		goto fail;

	strcpy(pl->pass, str);

	if(read_cmd_bounded(
		&str, &n, stream,
		ps, ps_perm, ps_col,
		"Enter your password again to confirm.", NULL,
		NULL
	) == -1)
		goto fail;

	if(strcmp(pl->pass, str)){
		cursor_position_response(stream);
		text_type(stream, "Your password didn't match! Failed to create player.");
		goto fail;
	}

	cursor_position_response(stream);
	text_type(stream, "Welcome to Quiet Space \033[0;31m%s\033[0m!", pl->nick);
out:
	free(a);
	return pl;
fail:
	free(pl);
	pl = NULL;
	goto out;
}

int handle_connection(FILE *request_stream, struct sockaddr_in socket_addr_client, int port){
	pid_t pid = 0;

	if(!(pid = fork())){
		char *ps = calloc(64, sizeof(char));
		char *ps_perm = calloc(5, sizeof(char));
		char *ps_col = calloc(16, sizeof(char));
		char *str = NULL;
		size_t n;
		ssize_t rd;
		char cmd_found = 0;

		player *pl = NULL;

		fprintf(
			request_stream,
			/* "Elite" font */
			"\033[2J\033c" /* Reset and clear display */
			"\r\n\033[0;31m" /* RED */
			"                        .▄▄▄  ▄• ▄▌▪  ▄▄▄ .▄▄▄▄▄       \r\n"
			"                        ▐▀•▀█ █▪██▌██ ▀▄.▀·•██         \r\n"
			"                        █▌·.█▌█▌▐█▌▐█·▐▀▀▪▄ ▐█.▪       \r\n"
			"                        ▐█▪▄█·▐█▄█▌▐█▌▐█▄▄▌ ▐█▌·       \r\n"
			"                        ·▀▀█.  ▀▀▀ ▀▀▀ ▀▀▀  ▀▀▀        \r\n"
			"                            .▄▄ ·  ▄▄▄· ▄▄▄·  ▄▄· ▄▄▄ .\r\n"
			"                            ▐█ ▀. ▐█ ▄█▐█ ▀█ ▐█ ▌▪▀▄.▀·\r\n"
			"                            ▄▀▀▀█▄ ██▀·▄█▀▀█ ██ ▄▄▐▀▀▪▄\r\n"
			"                            ▐█▄▪▐█▐█▪·•▐█ ▪▐▌▐███▌▐█▄▄▌\r\n"
			"                             ▀▀▀▀ .▀    ▀  ▀ ·▀▀▀  ▀▀▀ \r\n"
			"\r\n\r\n\033[0m"
			"  Type \033[1;31mjoin\033[0m to get started.\r\n"
			"  Type \033[1;31mlogin\033[0m to resume.\r\n"
			"  Type \033[1;31mquit\033[0m to disconnect.\r\n"
			"\033[0m"
		);
		draw_borders(request_stream, 1, 21, 39, 12);
		draw_borders(request_stream, 0, 1, 79, 18);

		// Go interactive.
		*ps = *ps_perm = *ps_col = 0;
		strcat(ps_perm, "$");

		while((rd = read_cmd(&str, &n, request_stream, ps, ps_perm, ps_col)) != -1){
			// Exit immediately.
			if(!strcmp(str, CMD_QUIT) || !strcmp(str, CMD_QUIT_ALT1))
				break;

			*ps = 0;
			cmd_found = 0;
			cursor_position_response(request_stream);

			// Command processing
			if(!strcmp(str, CMD_JOIN)){
				cmd_found = 1;

				// Prevent memory leak if join is called multiple times.
				if(pl)
					free(pl);

				pl = game_join(request_stream);
			} else if(!strcmp(str, CMD_LOGIN)){
				cmd_found = 1;

				text_type(request_stream, "Enter your username.");
				strcpy(ps, CMD_LOGIN);
			} else if(!strcmp(str, "demotext")){
				cmd_found = 1;

				text_type(request_stream, "This is an extremely long and uninteresting block of text, containing a multitude of sentences and words of varying lengths and complexities. This block is designed to test the rebustness of the text wrapping/typing function. Hopefully this all looks OK.");
			}

			// Unknown command text. FIXME
			if(!cmd_found)
				text_type(request_stream, "... what?\r\n");
		}

		// Exit message.
		fprintf(request_stream, "\033c\033[H\033[JA rushing cold sensation overcomes you.\r\n\r\n");

		// End of handler process.
		kws_fclose(&request_stream);

		free(str);
		free(ps);
		free(pl);
		exit(0);
	}
	if(pid > 0)
		waitpid(pid, NULL, 0);

	return 1;
}
