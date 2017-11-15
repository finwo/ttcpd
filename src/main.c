#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

int error( char *str ) {
  printf("%s\n", str);
  exit(1);
}

int main( int argc, char **argv ) {
  if(argc<3) error("Not enough arguments");
  int i, port = 8080;
  char *cmd = 0;
  char **args = 0;
  for(i=0;i<4&&i<argc;i++) {
    switch(i) {
      case 0: break;
      case 1: port = atoi(*(argv+i)); break;
      case 2: cmd=*(argv+i); break;
      case 3: args=argv+i; break;
    }
  }

  int sockfd = socket( AF_INET, SOCK_STREAM, 0 );
  if(sockfd<0) error("socket");
  struct sockaddr_in saddr, caddr;
  bzero((char *) &saddr, sizeof(saddr));
  saddr.sin_family      = AF_INET;
  saddr.sin_addr.s_addr = INADDR_ANY;
  saddr.sin_port        = htons(port);

  if(bind(sockfd,(struct sockaddr *) &saddr, sizeof(saddr)) < 0) {
    error("bind");
  }

  listen(sockfd,5);
  printf("Listening on port %d\n", port);
  socklen_t clen = sizeof(caddr);
  int nsock;
  while( (nsock=accept(sockfd,(struct sockaddr *) &caddr,&clen)) >= 0 ) {
    if(fork()) {
      close(nsock);
      waitpid(-1,NULL,WNOHANG);
    } else {
      dup2(nsock,0);
      dup2(nsock,1);
      close(nsock);
      execvp( cmd, args );
      return 3;
    }
  }

  return 0;
}
