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

void getFile(int sock) {
   int n;
   char buff[10000];
   memset(buff, '\0', sizeof(buff)); //Make and clear buffer

   //Infinite while loop
   while(1) {
      int bytesRead; //Keep index of bytes read
      bytesRead = read(sock, buff, sizeof(buff)); //Read from socket
      if(bytesRead == 0) { //Finished reading file
	 break;
      }else if(bytesRead < 0) { //If there's an error reading 
	 printf("Error reading (clientside)\n");
	 fflush(stdout);
	 exit(1);
      }else { 
	 n = write(1, buff, bytesRead); //Retrieve
	 if(n < 0) {
	    printf("Error writing! (Clientside)\n");
	    fflush(stdout);
	    exit(1);
	 }
      }
   }
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

int main(int argc, char *argv[]) {
   //Variable declarations
   int sock, portNum, n, optval, fd;
   struct sockaddr_in serverAddress;
   struct hostent *server;
   FILE *fp;

   //Check command line arguments
   if(argc < 4) {
      printf("Error: not enough command line arguments\n");
      fflush(stdout);
      exit(1);
   }

   //Create buffer and clear it
   char buff[10000];
   memset(buff, '\0', sizeof(buff));

   //Get port number from command line   
   portNum = atoi(argv[3]);
   sock = socket(AF_INET, SOCK_STREAM, 0); //Make the socket
   if(sock < 0) { 
      printf("Error creating socket! (clientside)\n");
      fflush(stdout);
      exit(1);
   }

   //Get hostname "localhost"
   server = gethostbyname("localhost");
   if(server == NULL) {
      printf("Error getting host name!\n");
      fflush(stdout);
      exit(1);
   }
   optval = 1;
   setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)); //Set for reuse
   memset((char*)&serverAddress, '\0', sizeof(serverAddress)); //Clear the struct
   serverAddress.sin_family = AF_INET; 
   memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)server->h_addr, server->h_length); 
   serverAddress.sin_port = htons(portNum); //Put port num in

   //Attempt connection
   if(connect(sock, (struct sockaddr*) &serverAddress, sizeof(serverAddress)) < 0) {
      printf("Error on connection! (clientside)\n");
      fflush(stdout);
      exit(1);
   }

   //Handshake
   char handshake[] = "client";
   write(sock, handshake, sizeof(handshake)); //Write handshake message to server
   read(sock, buff, sizeof(buff)); //Hopefully get handshake message back
   if(strcmp(buff, "server") != 0) { //If handshake doesn't work
      printf("Error on handshake! (Clientside)\n");
      fflush(stdout);
      exit(1);
   }

   //Read in file names
   long plainTextLen = getFileLen(argv[1]);
   long keyLen = getFileLen(argv[2]);

   //If the key length isn't long enough, exit
   if(plainTextLen > keyLen) {
      printf("Key is too short!\n");
      fflush(stdout);
      exit(1);
   }

   //Send files to server
   sendFile(argv[1], sock, plainTextLen);
   sendFile(argv[2], sock, keyLen);

   //Read from server
   n = read(sock, buff, sizeof(buff));
   if(n < 0) {
      printf("Error reading from server!\n");
      exit(1);
   }

   //Print the decrypted message and close socket
   printf("%s\n", buff);
   close(sock);

   return 0;
}
