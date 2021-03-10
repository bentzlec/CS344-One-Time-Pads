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
      return 26;
   }else {
      return(c - 'A');
   }
   return 0;
}

//Functio that returns char when given ASCII value
char intToChar(int i) {
   if(i == 26) {
      return ' ';
   }else {
      return (i + 'A');
   }
}

void decrypt(char msg[], char key[]) {
   int i; //Keep index
   char n; //Keep curr char

   //printf("Message: %s\n", msg);
   //printf("Key: %s\n", key);
   for (i = 0; msg[i] != '\n'; i++) { //iterate through message to end of line
      n = (charToInt(msg[i]) - charToInt(key[i])); //Get ascii value
      if(n < 0) {
	 n += 27;
      }
      msg[i] = intToChar(n); 
   }
   msg[i] = '\0'; //Set end of message to null
   return;
}

int main(int argc, char *argv[]) {
   //Variable declarations
   int sock, newSock, portNum, optval, n, status = 0;
   socklen_t len;
   char buff[10000];
   char key[10000];
   struct sockaddr_in serverAddress, clientAddress;
   pid_t pid;

   //Check to see if suff. command line args
   if(argc < 2) {
      printf("Not enough command line arguments!\n");
      fflush(stdout);
      exit(1);
   }

   //Create socket
   sock = socket(AF_INET, SOCK_STREAM, 0);
   if(sock < 0) {
      printf("Error making socket!\n");
      fflush(stdout);
      exit(1);
   }
   optval = 1;
   setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)); //Set socket to reuse port

   //Clear struct
   memset((char*) &serverAddress, '\0', sizeof(serverAddress));
   portNum = atoi(argv[1]);
   serverAddress.sin_family = AF_INET;
   serverAddress.sin_addr.s_addr = INADDR_ANY;
   serverAddress.sin_port = htons(portNum);

   //Bind address to socket to listen
   if(bind(sock, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
      printf("Error on socket binding (Serverside)\n");
      fflush(stdout);
      exit(1);
   }
   listen(sock, 5); //Listen for 5 connections

   //Infinite loop
   while(1) {
      //Attempt connection to client
      len = sizeof(clientAddress);
      newSock = accept(sock, (struct sockaddr*) &clientAddress, &len);
      if(newSock < 0) { //If connection accept fails
	 printf("Error on accepting connection! (serverside)\n");
	 fflush(stdout);
	 exit(1);
      }

      //Fork process
      pid = fork();
      
      //Fork error
      if(pid < 0) {
	 printf("Error on fork! (serverside)\n");
	 fflush(stdout);
	 exit(1);
      }else if(pid == 0) {
	 memset(buff, '\0', sizeof(buff)); //Clear buffer
	 int bytesLeft = sizeof(buff);
	 int bytesRead = 0;

	 char *currKey;
	 int lines, i = 0;

	 //Handshake
	 read(newSock, buff, sizeof(buff) - 1); //Read handshake message
	 if(strcmp(buff, "client") != 0) { //Handle if handshake is incorrect
	    char bad[] = "Bad Connection";
	    write(newSock, bad, sizeof(bad));
	    exit(1);
	 }else {
	    char handshake[] = "server";
	    write(newSock, handshake, sizeof(handshake)); //Write back if the handshake is good
	 }
	 memset(buff, '\0', sizeof(buff)); //Clear the buffer

	 //Hold index of where the key is in the buffer
	 char *keyIdx = buff;

	 //Infinite loop
	 while(1) {
	    bytesRead = read(newSock, keyIdx, bytesLeft); //Read from client and track bytes read
	    
	    if(bytesRead == 0) { //End of file
	       break;
	    }else if(bytesRead < 0) { //Error case
	       printf("Error reading from clientside! (Serverside)\n");
	       fflush(stdout);
	       exit(1);
	    }

	    //Count the number of lines
	    for(i = 0; i < bytesRead; i++) {
	       if(keyIdx[i] == '\n') {
		  lines++;
		  if(lines == 1) {
		     currKey = (keyIdx + i + 1);
		  }
	       }
	    }

	    if(lines == 2) { //Done reading
	       break;
	    }

	    keyIdx = (keyIdx + bytesRead); //Move idx to where key is located 
	    bytesLeft = (bytesLeft - bytesRead); //change idx of where we are in file read
	 }
	 //Buffer to hold the message
	 char message[10000]; 
	 memset(message, '\0', sizeof(message)); //clear the buffer

	 //Copy into the buffer
	 strncpy(message, buff, (currKey - buff));
	 decrypt(message, currKey); //Decrypt the message

	 write(newSock, message, sizeof(message)); //Send to client
      }
      close(newSock);

      while(pid > 0) {
	 pid = waitpid(-1, &status, WNOHANG); //Wait for child processes
      }
   }
   close(sock);

   return 0;
}
