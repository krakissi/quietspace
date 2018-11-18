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

#include "qsgame.h"

int handle_connection(FILE *request_stream, struct sockaddr_in socket_addr_client, int port){
	pid_t pid = 0;

	if(!(pid = fork())){
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
		while((rd = read_cmd(&str, &n, request_stream, "", "$", "")) != -1){
			// Exit immediately.
			if(!strcmp(str, CMD_QUIT) || !strcmp(str, CMD_QUIT_ALT1))
				break;

			cmd_found = 0;
			lower_str(str);

			if(!strcmp(str, CMD_JOIN)){
				// Create a character and start playing.

				cmd_found = 1;

				// Prevent memory leak if join is called multiple times.
				if(pl)
					free(pl);

				pl = game_join(request_stream);
			} else if(!strcmp(str, CMD_LOGIN)){
				// Login with an existing account.

				cmd_found = 1;

				if(pl)
					free(pl);

				pl = game_login(request_stream);
			}

			if(pl){
				// Once the player is logged in (or newly created via join) we can start the game.
				game_start(request_stream, pl);

				// When the game_start function returns, it's time to quit.
				break;
			}

			if(!cmd_found){
				cursor_position_response(request_stream);
				text_type(request_stream, "... what? Try \033[0;31mjoin\033[0m or \033[0;31mlogin\033[0m.\r\n");
			}
		}

		// Exit message.
		fprintf(request_stream, "\033c\033[H\033[JA rushing cold sensation overcomes you.\r\n\r\n");

		// End of handler process.
		kws_fclose(&request_stream);

		free(str);
		free(pl);
		exit(0);
	}
	if(pid > 0)
		waitpid(pid, NULL, 0);

	return 1;
}
