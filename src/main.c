/*
	quiet space
	mperron (2018)

	A cute telnet adventure for NaNoGaMo 2018.
*/
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <netinet/in.h>
#include <sys/wait.h>
#include <mysql/mysql.h>

#include "generic.h"
#include "handler.h"

#define KWS_DEFAULT_PORT 10421

void request_timeout(int);
FILE **request_stream = NULL;

/*
 * Write the process ID to a file, which the init_ws script is expecting to
 * find it in later.
 */
void pid_write(int port, int pid){
	char path[256];
	FILE *out = NULL;

	sprintf(path, "/tmp/quietspace_%d.pid", port);
	if((out = fopen(path, "w"))){
		fprintf(out, "%d", pid);
		fclose(out);
	}
}

void close_mysql_library(){
	mysql_library_end();
}

int main(int argc, char**argv){
	int sockfd, sockfd_client, client_length;
	struct sockaddr_in socket_addr_client;
	struct sockaddr_in socket_addr;
	int port = KWS_DEFAULT_PORT;
	int optval = 2;
	pid_t pid;

	// Log startup
	error_code(0, "--");
	error_code(0, "Server is starting up.");

	// Get a port number from argv if provided.
	if((argc > 1) && !(port = atoi(argv[1])))
		port = KWS_DEFAULT_PORT;

	// Fork the process and start the server.
	if(!(pid = fork())){
		if(mysql_library_init(0, NULL, NULL))
			return error_code(-10, "MySQL library init failed.");

		atexit(close_mysql_library);

		// Acquire INET socket.
		if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
			return error_code(1, "Socket not got.");

		memset(&socket_addr, 0, sizeof(socket_addr));
		socket_addr.sin_family = AF_INET;
		socket_addr.sin_port = htons(port);
		socket_addr.sin_addr.s_addr = htonl(INADDR_ANY);

		/*	SO_REUSEADDR is set so that we don't need to wait for all the
		 *	existing connections to close when restarting the server. */
		setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));


		if(bind(sockfd, (struct sockaddr*)&socket_addr, sizeof(socket_addr)) == -1)
			return error_code(1, "Couldn't bind.");

		if(listen(sockfd, 64) == -1)
			return error_code(1, "Deaf.");


		// Let init handle the children.
		signal(SIGCHLD, SIG_IGN);

		request_stream = malloc(sizeof(FILE*));

		while(1){
			client_length = sizeof(socket_addr_client);
			memset(&socket_addr_client, 0, client_length);
			sockfd_client = accept(sockfd, (struct sockaddr*)&socket_addr_client, (socklen_t*)&client_length);

			// Fork if a client connection accepts successfully.
			if(sockfd_client < 0)
				printf("No connection.\n");
			else if(!(pid = fork())){
				close(sockfd);

				// Bind a file stream to the socket. This makes everything easy.
				if(!(*request_stream = fdopen(sockfd_client, "w+")))
					return error_code(1, "Couldn't get a stream.");

				// Timeout handler
				signal(SIGALRM, request_timeout);
				signal(SIGUSR1, request_timeout);

				// Handle this connection, close stream when done.
				while(!handle_connection(*request_stream, socket_addr_client, port));

				// End of child process
				kws_fclose(request_stream);
				free(request_stream);
				close(sockfd_client);
				exit(0);
			}

			// Close handle to the child's socket in parent.
			close(sockfd_client);
		}

		// End of server process
		exit(0);
	}

	// PID info returned when starting server.
	if(pid > 0){
		printf("Server started on port %d. [%d]\n", port, pid);

		// Write the PID out to a file, so the init_ws script can kill it later on stop.
		pid_write(port, pid);
	} else printf("Failed\n");

	return 0;
}


/*	If the user is taking too long to make another request, we'll time out
	and kill the handler. */
void request_timeout(int i){
	if(request_stream)
		kws_fclose(request_stream);
}
