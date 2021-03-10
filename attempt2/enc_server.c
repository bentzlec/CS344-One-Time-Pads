#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

//Function that returns ASCII value when given a character
int charToInt(char c) {
   int i = 0;
   if(c == ' ') {
      i = 26;
   }else {
      i = (c - 'A');
   }
   return i;
}

//Functio that returns char when given ASCII value
char intToChar(int i) {
   if(i == 26) {
      return ' ';
   }else {
      return (i + 'A');
   }
}

void encrypt(char msg[], char key[]) {
   int i = 0; //Set index outside of for-loop so we can set last char to null
   char n; //Hold transformed char
   for(i = 0; msg[i] != '\n'; i++) {
      char curr = msg[i]; //point at current char in msg to be encrypted
      n = (charToInt(msg[i]) + charToInt(key[i])) % 27; 
      msg[i] = intToChar(n); //Set to encrypted char
   }
   msg[i] = '\0'; //Set last char to null

   return;
}

int main(int argc, char *argv[]) {
   int sock, newSock, portNum, optval, n, status = 0;
   socklen_t clientSock;
   char buff[100000];
   char key[100000];

   struct sockaddr_in serverAddress, clientAddress;
   pid_t pid, sid;

   if(argc < 2) {
      printf("Not enough command line arguments!\n");
      fflush(stdout);
      exit(1);
   }

   //Creat the socket
   sock = socket(AF_INET, SOCK_STREAM, 0);
   if(sock < 0) { //Error check the socket
      printf("Error creating socket! (Serverside)\n");
      fflush(stdout);
      exit(1);
   }

   //Set socket for reuse
   optval = 1;
   setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));
   memset((char*)&serverAddress, '\0', sizeof(serverAddress)); //Clear the struct
   
   //Get portnumber and set up serv addr struct
   portNum = atoi(argv[1]);
   serverAddress.sin_family = AF_INET;
   serverAddress.sin_addr.s_addr = INADDR_ANY;
   serverAddress.sin_port = htons(portNum);

   //Attempt connection
   if(bind(sock, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
      printf("Error binding sock to listen! (Serverside)\n");
      fflush(stdout);
      exit(1);
   }

   //Listen for 5 connections
   listen(sock, 5);

   while(1) {
      clientSock = sizeof(clientAddress);
      newSock = accept(sock, (struct sockaddr *) &clientAddress, &clientSock);
      if(newSock < 0) {
	 printf("Error accepting connection from client\n");
	 fflush(stdout);
	 exit(1);
      }

      //Fork the process
      pid = fork();

      if(pid < 0) { //Error handling for fork error
	 printf("Error on fork!\n");
	 fflush(stdout);
	 exit(1);
      }else if(pid == 0) { //Child process
	 memset(buff, '\0', sizeof(buff));
	 //bzero(buff, sizeof(buff));
	 int bytesLeft = sizeof(buff);
	 int bytesRead = 0;
	 char * curr = buff;
	 char * currKey;
	 int lines = 0;
	 int i;

	 //Handshake
	 read(newSock, buff, sizeof(buff)-1);
	 if(strcmp(buff, "client") != 0) {
	    printf("Handshake error! (serverside)\n");
	    fflush(stdout);
	    char bad[] = "Bad Connection";
	    write(newSock, bad, sizeof(bad));
	    exit(1);
	 }else {
	    char handshake[] = "server";
	    write(newSock, handshake, sizeof(handshake));
	 }

	 //Clear buffer again
	 memset(buff, '\0', sizeof(buff));
	 
	 //Infinite while loop
	 while(1) {
	    //Read from socket 
	    bytesRead = read(newSock, curr, bytesLeft);
	    
	    if(bytesRead < 0) {
	       printf("Error reading on serverside!\n");
	       fflush(stdout);
	       exit(1);
	    }else if(bytesRead == 0) { //Done reading so break
	       break;
	    }

	    for(i = 0; i < bytesRead; i++) {
	       if(curr[i] == '\n') {
		  lines++; //Count number of lines
		  if(lines == 1) {
		     currKey = (curr + i + 1); //Where key index is
		  }
	       }
	    }

	    if(lines == 2) { //We have read the key and message
	       break;
	    }

	    //Iterate indexes
	    curr = (curr + bytesRead);
	    bytesLeft = (bytesLeft - bytesRead);
	 }
	 //buffer to hold the encrypted message
	 char encrypted[10000];
	 memset(encrypted, '\0', sizeof(encrypted)); //Clear buffer

	 strncpy(encrypted, buff, (currKey - buff)); //copy message into buffer to be encrypted

	 encrypt(encrypted, currKey); //Encrypt the message

	 write(newSock, encrypted, sizeof(encrypted)); //Write encrypted msg to client
      }
      close(newSock); //Close socket

      while(pid > 0) {
	 pid = waitpid(-1, &status, WNOHANG); //Wait for child proc
      }
   }
   close(sock);

   return 0;
}
