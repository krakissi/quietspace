#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <netinet/in.h>
#include <sys/wait.h>

#include "generic.h"
#include "qsfmt.h"

#include "handler.h"

int handle_connection(FILE *request_stream, struct sockaddr_in socket_addr_client, int port){
	pid_t pid = 0;

	if(!(pid = fork())){
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
			"\r\n\r\n"
			"  Type \"\033[1;31mjoin\033[0;31m\" to get started.\r\n"
			"  Type \"\033[1;31mlogin\033[0;31m\" to resume.\r\n"
			"  Type \"\033[1;31mquit\033[0;31m\" to disconnect.\r\n"
			"\033[0m"
		);
		draw_borders(request_stream, 1, 21, 39, 12);
		draw_borders(request_stream, 0, 1, 79, 18);

		// Go interactive.
		goto prompt;
		while((rd = getline(&str, &n, request_stream)) != -1){
			// Remove trailing return/feed characters.
			sanitize_str(str);

			// Exit immediately.
			if(!strcmp(str, "quit") || !strcmp(str, "q"))
				break;

			cmd_found = 0;

			fputs("\033[18H\033[J", request_stream);

			// Ignore empty command lines.
			if(!*str)
				goto prompt;


			// Command processing
			// TODO

			// Unknown command text. FIXME
			if(!cmd_found)
				fprintf(request_stream, "... what?\r\n");
prompt:
			// Print out the prompt and await a command.
			fprintf(request_stream, "\033[23H\033[J> ");
		}

		// Exit message.
		fprintf(request_stream, "\033c\033[H\033[JA rushing cold sensation overcomes you.\r\n\r\n");

		// End of handler process.
		kws_fclose(&request_stream);

		free(str);
		exit(0);
	}
	if(pid > 0)
		waitpid(pid, NULL, 0);

	return 1;
}
