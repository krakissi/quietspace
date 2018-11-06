#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <netinet/in.h>
#include <sys/wait.h>

#include "generic.h"

#include "handler.h"

int handle_connection(FILE *request_stream, struct sockaddr_in socket_addr_client, int port){
	pid_t pid = 0;

	if(!(pid = fork())){
		char *str = NULL;
		size_t n;
		ssize_t rd;

		fprintf(
			request_stream,
			"Type \"exit\" to disconnect.\r\n"
			"\r\n"
			"Quiet Space\r\n"
			"-----------\r\n"
			"\r\n"
			"Warmth washes over you as your body exits cryostasis.\r\n"
		);

		// Go interactive.
		goto prompt;
		while((rd = getline(&str, &n, request_stream)) != -1){
			// Remove trailing return/feed characters.
			sanitize_str(str);

			// Ignore empty command lines.
			if(!*str)
				goto prompt;

			// Exit immediately.
			if(!strcmp(str, "exit"))
				break;

			// Command processing
			// TODO

			// Unknown command text. FIXME
			fprintf(request_stream, "... what?\r\n");
prompt:
			// Print out the prompt and await a command.
			fprintf(request_stream, "\r\n> ");
		}

		fprintf(request_stream, "\r\nA rushing cold sensation overcomes you.\r\n");

		// End of handler process.
		kws_fclose(&request_stream);

		free(str);
		exit(0);
	}
	if(pid > 0)
		waitpid(pid, NULL, 0);

	return 1;
}
