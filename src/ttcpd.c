#ifdef __cplusplus
extern "C" {
#endif

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main( int argc, char *argv[] ) {

  // Initialize
  int opt=0,port = 8080;
  char *cmd = 0;
  char **args = 0;

  // Detect port by environment variables
  if(getenv("PORT")) {
    port = atoi(getenv("PORT"));
  }

  // Parse arguments
  while((opt = getopt(argc, argv, "c:p:")) != -1) {
    switch(opt) {
      case 'c':
        cmd = optarg;
        break;
      case 'p':
        port = atoi(optarg);
        break;
      default:
        fprintf( stderr, "Usage: %s [-p port] [-c command] [-- arg arg ...]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
  }

  // Catch arguments to child
  if(strcmp(argv[optind-1],"--")==0) {
    args = argv + optind;
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

  // Bind socket to port
  if(bind(sockfd,(struct sockaddr *) &saddr, sizeof(saddr))<0) {
    fprintf( stderr, "Could not bind to port %d\n", port );
    exit(EXIT_FAILURE);
  }

  // Start listening
  if(listen(sockfd,5)) {
    fprintf( stderr, "Could not start listening\n" );
    exit(EXIT_FAILURE);
  } else {
    printf( "Listening on port %d\n", port );
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
