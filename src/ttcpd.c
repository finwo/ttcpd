#ifdef __cplusplus
extern "C" {
#endif

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void usage( char *cmd ) {
  fprintf(stderr, "\n");
  fprintf(stderr, "Usage:\n");
  fprintf(stderr, "  %s -h\n", cmd);
  fprintf(stderr, "  %s [-p port] [-c command] [-- arg arg ...]\n", cmd);
  fprintf(stderr, "\n");
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "  -h          Show this help\n");
  fprintf(stderr, "  -a address  Address to listen on (default: any)\n");
  fprintf(stderr, "  -p port     Set the port to listen on (default: 8080)\n");
  fprintf(stderr, "  -c command  The command to run\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "About:\n");
  fprintf(stderr, "%s handles network communications for basic commands, attaching\n", cmd);
  fprintf(stderr, "a connection's i/o to the command's stdin and stdout.\n");
  fprintf(stderr, "\n");
}

int main( int argc, char *argv[] ) {
  char * inaddr = "any";

  // Initialize
  int opt=0,port = 8080;
  char *cmd = 0;
  char **args = calloc(argc,sizeof(void*));

  // Detect port by environment variables
  if(getenv("PORT")) {
    port = atoi(getenv("PORT"));
  }

  // Parse arguments
  while((opt = getopt(argc, argv, "a:c:p:")) != -1) {
    switch(opt) {
      case 'a':
        inaddr = optarg;
        break;
      case 'c':
        cmd = optarg;
        break;
      case 'p':
        port = atoi(optarg);
        break;
      case 'h':
        usage(argv[0]);
        exit(EXIT_SUCCESS);
      default:
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }
  }

  // Catch arguments to child
  if(strcmp(argv[optind-1],"--")==0) {
    args    = argv + (optind-1);
    args[0] = cmd;
  }

  // Make sure we have a command set
  if(!cmd) {
    fprintf( stderr, "Required argument not given: command\n" );
    exit(EXIT_FAILURE);
  };

  // Start listening socket
  int sockfd = socket( AF_INET, SOCK_STREAM, 0 );
  if(sockfd<0) {
    fprintf( stderr, "Could not initialize socket\n" );
    exit(EXIT_FAILURE);
  }

  // Setup server address
  struct sockaddr_in saddr, caddr;
  bzero((char *) &saddr, sizeof(saddr));
  saddr.sin_family      = AF_INET;
  saddr.sin_addr.s_addr = INADDR_ANY;
  saddr.sin_port        = htons(port);

  inet_aton(inaddr, &saddr.sin_addr);

  // Allow socket re-use
  int optval = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));

  // Bind socket to port
  if(bind(sockfd,(struct sockaddr *) &saddr, sizeof(saddr))<0) {
    fprintf( stderr, "Could not bind to %s:%d\n", inet_ntoa(saddr.sin_addr), port );
    exit(EXIT_FAILURE);
  }

  // Start listening
  if(listen(sockfd,5)) {
    fprintf( stderr, "Could not start listening\n" );
    exit(EXIT_FAILURE);
  } else {
    printf( "Listening on %s:%d\n", inet_ntoa(saddr.sin_addr), port );
  }

  // Prepare for the client
  socklen_t clen = sizeof(caddr);
  int nsock;

  // Start accepting connections
  while( (nsock=accept(sockfd,(struct sockaddr *) &caddr,&clen)) >= 0 ) {

    // Spawn a clone
    if(fork()) {

      // We're the parent, we don't need the socket anymore
      close(nsock);

      // Don't wait for childs, ever
      waitpid(-1,NULL,WNOHANG);
    } else {

      // We're the child, bind the socket to stdio
      dup2(nsock,0);
      dup2(nsock,1);

      // Close the old reference
      close(nsock);

      // Start the given command
      execvp( cmd, args );

      // Being here is a major failure
      // Simply exit with an error code
      return 3;
    }
  }

  // We should never reach this
  // Maybe a SIGHUP received?
  return 0;
}

#ifdef __cplusplus
} //extern "C"
#endif
