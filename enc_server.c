#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>



#define SIZE 100000

int removeChar (int idx) {
   if (idx == 26) {
      return ' ';
   } else {
      return (idx + 'A');
   }

}

void encrypt(char input[], char key[]) {
   int i = 0;
   char currChar;

   while(input[i] != '\n') {
      currChar = input[i];
      currChar = (removeChar(input[i]) + removeChar(key[i])) % 27;
      input[i] = removeChar(currChar);
      i++;
   }
   input[i] = '\0';
   return;
}

int main(int argc, char * argv[]) {
   if (argc < 2) {
      printf("Error!, not enough arguments\n");
      fflush(stdout);
      exit(1);
   }

   struct sockaddr_in servAdd, clientAdd; 
   memset((char *) &servAdd, '\0', sizeof(servAdd)); //Clear the struct
   int portNum = atoi(argv[1]); //Get port number from arguments
   int i = 0;
   servAdd.sin_family = AF_INET; 
   servAdd.sin_port = htons(portNum); //Set port num
   servAdd.sin_addr.s_addr = INADDR_ANY; //Set to allow connection from any address

   int listenSock = socket(AF_INET, SOCK_STREAM, 0); //Make a socket to listen on
   if (listenSock < 0) { //Error check
      printf("Socket error!\n");
      fflush(stdout);
      exit(1);
   }

   if(bind(listenSock, (struct sockaddr*)&servAdd, sizeof(servAdd)) < 0) { //Bind listening socket and server address
      printf("Error on socket bind!\n");
      fflush(stdout);
      exit(1);
   }

   socklen_t size;
   int connectFD = 0; //Variable to hold size of address and file descriptor for connection made
   while (1) {
      listen(listenSock, 5); //Listen on our socket for 5 connections
      size = sizeof(clientAdd);
      connectFD = accept(listenSock, (struct sockaddr*)&clientAdd, &size); //Accept anyincoming data
      if(connectFD < 0) {
	 printf("Error on accepting connection!\n");
	 fflush(stdout);
	 exit(1);
      }

      pid_t pid = fork();
      if (pid == -1) { //Error handling
	 printf("Error on fork!\n");
	 fflush(stdout);
	 exit(1);
      }else if (pid == 0) { //if child
	 char buff[1024];
	 char message[10000];
	 char key[1000];

	 memset(buff, '\0', sizeof(buff));

	 while(i == 0) {
	    i = recv(connectFD, buff, sizeof(buff) - 1, 0);
	 }

	 if(strcmp(buff, "Handshake") != 0) {
	    i = send(connectFD, "Bad Port", strlen("Bad Port"), 0);
	    exit(1);
	 }else {
	    printf("Handshake received!\n");
	    fflush(stdout);
	    memset(buff, '\0', sizeof(buff));

	    i = send(connectFD, "Handshake", 9, 0);
	    i = 0;

	    while(i == 0) { 
	       i = recv(connectFD, buff, sizeof(buff) - 1, 0);
	    }

	    int len = atoi(buff);
	    printf("Serverside file length: %i\n\n", len);

	    memset(buff, '\0', sizeof(buff));

	    i = 0;
	    while(i <= len) {
	       i += recv(connectFD, buff, sizeof(buff) - 1, 0);
	       strcat(key, buff);
	       printf("Key received from client: %s\n", key);
	    }
	    printf("Key received from client: %s\n", key);


	    memset(buff, '\0', sizeof(buff));
	    
	    i = 0;
	    while(i <= len) {
	       i += recv(connectFD, buff, sizeof(buff) - 1, 0);
	       strcat(message, buff);
	    }
	    //printf("Message received from client: %s\n", message);
	    printf("Key received from client: %s\n", key);

	    memset(buff, '\0', sizeof(buff));

	    encrypt(message, key);

	    printf("Encrytpyed message: %s\n\n", message);
	    i = 0;
	    while(i < len) {
	       i += send(connectFD, message, sizeof(message), 0);
	    }

	    exit(0);
	 }
      }else {
	 int status = 0;
	 pid_t parent_pid = waitpid(pid, &status, WNOHANG);
      }
   } 
   close(connectFD);
   close(listenSock);

   return 0;
}
