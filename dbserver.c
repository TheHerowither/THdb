#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

#include <sys/socket.h>
#include <netinet/in.h>

#define CLOG_IMPLEMENTATION
#include "clog.h"

#define PORT_DEFAULT 8888
#define uint unsigned int
#define cstr char *
#define BUF_MAX 1024

#define TODO() printf("TODO: %s()\n", __FUNCTION_NAME__)

#ifndef __FUNCTION_NAME__
    #ifdef WIN32   //WINDOWS
        #define __FUNCTION_NAME__   __FUNCTION__  
    #else          //*NIX
        #define __FUNCTION_NAME__   __func__ 
    #endif
#endif


typedef struct SOCKET {
	int fd;
	int opt;
	struct sockaddr_in address;
	socklen_t addrlen;
} SOCKET;

const ClogLevel RECV = CLOG_REGISTER_LEVEL("RECV", "", 3);
uint server_port = PORT_DEFAULT;
bool verbose = false;
bool running = true;

unsigned int convert(char *st) {
  	char *x;
  	for (x = st ; *x ; x++) {
    	if (!isdigit(*x))
      		return 0L;
  	}
  	return (strtoul(st, 0L, 10));
}

void help() {
  	TODO();
}

void parse_verbose_arg(const cstr arg, cstr *argv, uint idx, uint argc) {
  	if (strcmp(arg, "help") == 0) {
    	help();
  	}
  	else if (strcmp(arg, "verbose") == 0) {
    	verbose = true;
  	}
  	else if (strcmp(arg, "port") == 0) {
        if (argc > idx+1) {
          server_port = convert(argv[idx+1]);
        }
        else {
          	fprintf(stderr, "ERROR: Option --port requires one argument: <port>\n");
          	exit(1);
    	}

  	}
  	else {
    	fprintf(stderr, "Unknown verbose option '%s'\n", arg);
    	exit(1);
  	}
}

void parse_dash(const cstr arg, cstr *argv, uint idx, uint argc) {
	for (uint i = 0; i < strlen(arg); i++) {
	    switch (arg[i]) {
    	  case '-':
	        parse_verbose_arg(arg+1, argv, idx, argc);
        	return;
    	  case 'h':
	        help();
        	break;
    	  case 'v':
	        verbose = true;
        	break;
    	  case 'p':
	        if (argc > idx+1) {
        	  server_port = convert(argv[idx+1]);
    	    }
	        else {
          		fprintf(stderr, "ERROR: Option -p requires one argument: <port>\n");
          		exit(1);
        	}
        	return;
      	default:
        	fprintf(stderr, "ERROR: Unknown option '%c'\n", arg[i]);
    	}
  	}
}

SOCKET create_socket(uint port) {
	SOCKET sock = {0};
	sock.opt = 1;

	if ((sock.fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		clog(CLOG_ERROR, "Failed to create socket");
		exit(1);
	}

	if (setsockopt(sock.fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &sock.opt, sizeof(sock.opt))) {
		clog(CLOG_ERROR, "Failed to set socket opt");
		exit(1);
	}

	sock.address.sin_family = AF_INET;
	sock.address.sin_addr.s_addr = INADDR_ANY;
	sock.address.sin_port = htons(port);

	clog(CLOG_TRACE, "Binding socket to port %u", port);
	if (bind(sock.fd, (struct sockaddr*)&sock.address, sizeof(sock.address)) < 0) {
		clog(CLOG_ERROR, "Bind failes");
		exit(1);
	}
	clog(CLOG_INFO, "Socket bound to port %u", port);

	return sock;
}

SOCKET socket_accept(SOCKET *server) {
	SOCKET client = {0};
	client.addrlen = sizeof(client.address);

	if ((client.fd = accept(server->fd, (struct sockaddr*)&(client.address), &(client.addrlen))) < 0) {
		clog(CLOG_ERROR, "Accept failed");
	}

	return client;
}

void sigint(int dummy) {
	(void)dummy;
	running = false;
}

void *handle_request(void *arg) {
	SOCKET *client = (SOCKET*)arg;
	char *buffer = malloc(BUF_MAX * sizeof(char));

	ssize_t bytes_recv = recv(client->fd, buffer, BUF_MAX, 0);

	if (bytes_recv > 0) {
		clog(RECV, "Recieved %u bytes: \n%s", bytes_recv, buffer);
	}

	free(buffer);
	close(client->fd);
	return NULL;
}

int main(int argc, cstr *argv) {
  	//const cstr program_name = argv[0];
  	clog_fmt = "%L: %m";
  	for (uint i = 1; i < (uint)argc; i++) {
    	const char *arg = argv[i];
    	if (arg[0] == '-') {
      		parse_dash(arg+1, argv, i, argc);
    	}
	}
	if (!verbose) clog_mute_level(CLOG_WARNING);

	signal(SIGINT, sigint);
	signal(SIGKILL, sigint);

	clog(CLOG_TRACE, "Preparing to initialize server");
	printf("Starting server with:\n");
	printf("  Port: %u\n", server_port);

	SOCKET server = create_socket(server_port);
	printf("Server started on port %u\n", server_port);

	if (listen(server.fd, 10) < 0) {
		clog(CLOG_ERROR, "Failed to listen on socket");
		return 1;
	}

	while (running) {
		SOCKET client = socket_accept(&server);

		pthread_t client_thread;
		pthread_create(&client_thread, NULL, handle_request, (void*)&client);
		pthread_detach(client_thread);
	}
	
	printf("Closing server...\n");
	close(server.fd);
  	return 0;
}
