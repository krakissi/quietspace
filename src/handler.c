#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include <netinet/in.h>
#include <sys/wait.h>

#include "defs.h"

#include "generic.h"
#include "qsfmt.h"
#include "dbpersistence.h"

#include "handler.h"

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

void cursor_position_response(FILE *stream){
	fputs("\033[18H\033[J", stream);
}

int handle_connection(FILE *request_stream, struct sockaddr_in socket_addr_client, int port){
	pid_t pid = 0;

	if(!(pid = fork())){
		char *ps = calloc(64, sizeof(char));
		char *ps_perm = calloc(5, sizeof(char));
		char *ps_col = calloc(16, sizeof(char));
		char *str = NULL, *a;
		size_t n;
		ssize_t rd;
		char cmd_found = 0;

		player pl;

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

				text_type(request_stream, "Choose a name. It may be no more than eight characters long and should consist only of letters ([A-Z][a-z]).");
				strcpy(ps, "join");

				if((rd = read_cmd(&str, &n, request_stream, ps, ps_perm, ps_col) == -1))
					break;

				while(strlen(str) > (QS_LEN_NAME - 1)){
					cursor_position_response(request_stream);
					text_type(request_stream, "That name is too long.");

					if((rd = read_cmd(&str, &n, request_stream, ps, ps_perm, ps_col) == -1))
						break;
				}
				if(rd == -1)
					break;

				strcpy(pl.nick, str);
				for(a = str; *a; a++)
					*a = tolower(*a);
				strcpy(pl.name, str);

				cursor_position_response(request_stream);
				text_type(request_stream, "Ok \033[0;31m%s\033[0m, what's your password?", pl.nick);

				strcat(ps, "/pass");
				strcat(ps_col, "\033[8m");

				if((rd = read_cmd(&str, &n, request_stream, ps, ps_perm, ps_col) == -1))
					break;

				while(strlen(str) > (QS_LEN_PASS - 1)){
					cursor_position_response(request_stream);
					text_type(request_stream, "\033[0mThat password is too long.");

					if((rd = read_cmd(&str, &n, request_stream, ps, ps_perm, ps_col) == -1))
						break;
				}
				if(rd == -1)
					break;

				strcpy(pl.pass, str);

				// Clear prompt and color.
				*ps = *ps_col = 0;

				// FIXME debug
				cursor_position_response(request_stream);
				debug_print_character(request_stream, &pl);
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
		exit(0);
	}
	if(pid > 0)
		waitpid(pid, NULL, 0);

	return 1;
}
