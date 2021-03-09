#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

void sendFile(char *filename, int sock, int length) {
   //Open file
   int fd = open(filename, 'r');
   char buff[10000];
   memset(buff, '\0', sizeof(buff)); //Clear buffer

   int bytesRead, bytesWritten = 0; //Indexes to hold how far read/written
   while(length > 0) { //Infinited loop to read
      bytesRead = read(fd, buff, sizeof(buff));
      if(bytesRead == 0) { //All content has been read
	 break;
      }else if(bytesRead < 0) { //Error handling
	 printf("Error reading from file! (Clientside in sendFile())\n");
	 fflush(stdout);
	 exit(1);
      }
      length = (length - bytesRead); //Subtract how much we have read
   }
   char * curr;
   curr = buff;
   while(bytesRead > 0) { //Infinite while loop to read
      bytesWritten = write(sock, curr, bytesRead);

      if(bytesWritten < 0) { 
	 printf("Error writing to server! (Clientside)\n");
	 fflush(stdout);
	 exit(1);
      }

      bytesRead = (bytesRead - bytesWritten); //Track how far read
      curr = (curr + bytesWritten);
   }

   return;
}

long getFileLen(char *filename) {
   FILE * fd = fopen(filename, "r"); //Open filename passed to it
   fpos_t idx; 
   long len;

   fgetpos(fd, &idx); //Get index of last item in file
   if(fseek(fd, 0, SEEK_END) || (len = ftell(fd)) == -1) { //Read to end of file
      printf("File does not exist!\n");
      fflush(stdout);
      exit(1);
   }
   fsetpos(fd, &idx);

   return len; 
}

int main(int argc, char * argv[]) {
   //Variable declarations
   int clientSock, portNum, n, plaintextFD = 0;
   struct sockaddr_in serverAddress;
   struct hostent *server;
   FILE * fd;

   //Checking for enough command line arguments
   if(argc < 4) {
      printf("Not enough arguments!\n");
      fflush(stdout);
      exit(1);
   }

   //Creating and clearing buffer
   char buff[10000];
   memset(buff, '\0', sizeof(buff));

   //Get port number from command line
   portNum = atoi(argv[3]);
   clientSock = socket(AF_INET, SOCK_STREAM, 0); //Make the clientside socket
   //Error handle the sock
   if(clientSock < 0) {
      printf("Error creating the clientside socket!\n");
      fflush(stdout);
      exit(1);
   }

   //Get server name "Localhost"
   server = gethostbyname("localhost");
   //Error handle
   if(server == NULL) {
      printf("Error getting hostname!\n");
      fflush(stdout);
      exit(1);
   }
   
   //Set the port for reuse
   int optval = 1;
   setsockopt(clientSock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));

   //Clear the struct
   memset((char*)&serverAddress, '\0', sizeof(serverAddress));
   serverAddress.sin_family = AF_INET;
   //Copy address into strcut
   serverAddress.sin_port = htons(portNum);
   memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)server->h_addr, server->h_length);

   //Attempt connection to server socket
   if(connect(clientSock, (struct sockaddr*) &serverAddress, sizeof(serverAddress)) < 0) {
      printf("Error attempting to connect!\n");
      fflush(stdout);
      exit(1);
   }

   //Make handshake
   char handshake[] = "client";
   write(clientSock, handshake, sizeof(handshake));
   read(clientSock, buff, sizeof(buff));
   if(strcmp(buff, "server") != 0) {
      printf("Error on handshake! (Clientside)\n");
      fflush(stdout);
      exit(1);
   }

   //Get length of key and plaintext
   long plainTextLen = getFileLen(argv[1]);
   long keylength = getFileLen(argv[2]);
   if(plainTextLen > keylength) {
      printf("Key length not long enough!\n");
      fflush(stdout);
      exit(1);
   }

   //Check file for in-valid chars
   int fd1 = open(argv[1], 'r');
   while(read(fd1, buff, 1) != 0) {
      if(buff[0] != ' ' && (buff[0] < 'A' || buff[0] > 'Z')) {
	 if(buff[0] != '\n') {
	    printf("Plaintext file contains invalid characters!\n");
	    fflush(stdout);
	    exit(1);
	 }
      }
   }

   //Clear buffer
   memset(buff, '\0', sizeof(buff));

   //Send plaintext
   sendFile(argv[1], clientSock, plainTextLen);

   //Send key
   sendFile(argv[2], clientSock, keylength);

   //Rsponse from server
   n = read(clientSock, buff, sizeof(buff) - 1);

   //Error handling when can't read from socket
   if(n < 0) {
    printf("Error reading from server\n");
    fflush(stdout);
    exit(1);
   }

   printf("%s\n", buff);
   close(clientSock);
   return 0;
}
