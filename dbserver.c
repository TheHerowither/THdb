#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <ctype.h>
#include <string.h>

#define CLOG_IMPLEMENTATION
#include "clog.h"

#define PORT_DEFAULT 8888
#define uint unsigned int
#define cstr char *
#define SOCKET unsigned int

#define TODO() printf("TODO: %s()\n", __FUNCTION_NAME__)

#ifndef __FUNCTION_NAME__
    #ifdef WIN32   //WINDOWS
        #define __FUNCTION_NAME__   __FUNCTION__  
    #else          //*NIX
        #define __FUNCTION_NAME__   __func__ 
    #endif
#endif


uint port = PORT_DEFAULT;
bool verbose = false;

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
          port = convert(argv[idx+1]);
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
          port = convert(argv[idx+1]);
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

int main(int argc, cstr *argv) {
  //const cstr program_name = argv[0];
  clog_fmt = "%c%L:%r %m";
  for (uint i = 1; i < (uint)argc; i++) {
    const char *arg = argv[i];
    if (arg[0] == '-') {
      parse_dash(arg+1, argv, i, argc);
    }
  }
  if (!verbose) clog_mute_level(CLOG_WARNING);

  clog(CLOG_TRACE, "Preparing to initialize server");
  printf("Starting server with:\n");
  printf("  Port: %u\n", port);
  return 0;
}
