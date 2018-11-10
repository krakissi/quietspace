#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <netinet/in.h>
#include <sys/wait.h>

#include "defs.h"

#include "generic.h"
#include "qsfmt.h"
#include "dbpersistence.h"

#include "handler.h"

int handle_connection(FILE *request_stream, struct sockaddr_in socket_addr_client, int port){
	pid_t pid = 0;

	if(!(pid = fork())){
		char *ps = calloc(64, sizeof(char));
		char *str = NULL;
		size_t n;
		ssize_t rd;
		char cmd_found = 0;

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
		*ps = 0;
		goto prompt;
		while((rd = getline(&str, &n, request_stream)) != -1){
			// Remove trailing return/feed characters.
			sanitize_str(str);

			// Exit immediately.
			if(!strcmp(str, CMD_QUIT) || !strcmp(str, CMD_QUIT_ALT1))
				break;

			// Ignore empty command lines.
			if(!*str)
				goto prompt;

			*ps = 0;
			cmd_found = 0;
			fputs("\033[18H\033[J", request_stream);

			// Command processing
			if(!strcmp(str, CMD_JOIN)){
				cmd_found = 1;

				text_type(request_stream, "Choose a name. It may be no more than eight characters long and should consist only of letters ([A-Z][a-z]).");
				strcpy(ps, "join");

				// TODO - accept a password. Use color 8m to make text invisible.
				strcpy(ps, "pass\033[8m");
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
prompt:
			// Print out the prompt and await a command.
			fprintf(request_stream, "\033[23H\033[J%s> ", ps);
		}

		// Exit message.
		fprintf(request_stream, "\033c\033[H\033[JA rushing cold sensation overcomes you.\r\n\r\n");

		// FIXME debug
		db_dump_users(request_stream);

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
