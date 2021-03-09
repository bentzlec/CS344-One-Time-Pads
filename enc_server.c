#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char argv**) {
   if (argc < 2) {
      printf("Error!, not enough arguments\n");
      fflush(stdout);
      exit(1);
   }

   struct sockaddr_in servAdd, clientAdd; 
   memset((char *) &servAdd, '\0', sizeof(servAdd)); //Clear the struct
   int portNum = atoi(argv[1]); //Get port number from arguments
   servAdd.sin_family = AF_INET; 
   servAdd.sin_port = htons(portNum); //Set port num
   servAdd.sin_addr.s_addr = INADDR_ANY; //Set to allow connection from any address

   int listenSock = socket(AF_INET, SOCK_STREAM, 0); //Make a socket to listen on
   if (listenSock < 0) { //Error check
      printf("Socket error!\n");
      fflush(stdout);
      exit(1);
   }



  

   return 0;
}
