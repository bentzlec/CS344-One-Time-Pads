#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ctype.h>

int getFileLength(char * fileName) {
   int curr, numChars = 0;
   
   FILE* fd = fopen(fileName, "r"); //Open given filename

   curr = fgetc(fd); //Get the very first character in the file

   while (curr != EOF || curr != '\n') { //Read until end of file or newline
      if(!isupper(curr) && curr != ' ') { //Check for bad characters
	    fprintf(stderr, "File contains bad characters!");
	    fflush(stdout);
	    exit(1);
      }
      curr = fgetc(fd); //Get the next character

      numChars++; //Iterate counter
   } 
   fclose(fd); //Close file

   return numChars;
}

int main (int argc, char * argv[]) {
   int sockFD, portNum, charWrittenIdx, charReadIdx, bytesRead = 0;
   struct sockaddr_in sockAdd;
   struct hostent* serverHost;
   
   char buff[1024];
   char cipherBuff[100000];

   memset((char *) &sockAdd, '\0', sizeof(sockAdd)); //Clear struct
   portNum = atoi(argv[3]); //Get port num from command line arguments
   sockAdd.sin_family = AF_INET; //Make it accept connections
   sockAdd.sin_port = htons(portNum); //Put port in struct
   serverHost = gethostbyname("localhost"); //Get the address
   memcpy((char*)&sockAdd.sin_addr.s_addr, (char*)serverHost->h_addr, serverHost->h_length); //Copy address into struct


   sockFD = socket(AF_INET, SOCK_STREAM, 0); //Make the socket
   //Error check socket
   if(sockFD < 0) {
      printf("Socket error!\n");
      fflush(stdout);
      exit(1);
   }
   
   //Make connection to server
   if(connect(sockFD, (struct sockaddr*)&sockAdd, sizeof(sockAdd)) < 0) {
      printf("Client side error!\n");
      fflush(stdout);
      exit(1);
   }

   long fileLen = getFileLength(argv[1]); //Get file lengths
   long keylength = getFileLength(argv[2]);

   if(fileLen > keylength) { //Error handle when key is too short
      printf("Error! Key too short!\n");
      fflush(stdout);
      exit(1);
   }

   char msg[] = "Handshake"; //Create handshake message
   
   while(charWrittenIdx == 0) {
      charWrittenIdx = recv(sockFD, buff, sizeof(buff) - 1, 0);
   }

   if(strcmp(buff, "Bad Port") == 0) { //Error handle bad port
      printf("Bad port number!\n");
      fflush(stdout);
      exit(1);
   }

   memset(buff, '\0', sizeof(buff)); //Clear buffer
   sprintf(buff, "%d", fileLen); //put the file length in the buffer
   charWrittenIdx = send(sockFD, buff, sizeof(buff), 0); //Send the file len
   memset(buff, '\0', sizeof(buff)); //Clear buffer again

   int fd = fopen(argv[1], "r"); //Open the file
   if(fd < 0) { //Error handling for bad file open
      printf("Error opening file!\n");
      fflush(stdout);
      exit(1);
   }

   int i = 0;
   while(i <= fileLen) { //Read to end of file, track for both file read and file send
      bytesRead = read(fd, buff, sizeof(buff) - 1);
      i += send(sockFD, buff, strlen(buff), 0);
   }

   memset(buff, '\0', sizeof(buff)); //Clear buffer

   i = 0;
   while(i <= fileLen) {
      i += recv(sockFD, buff, sizeof(buff) - 1, 0); //Receive and store in buffer and print to screen
      strcat(cipherBuff, buff);
   }
   strcat(cipherBuff, "\n");
   printf("%s", cipherBuff);

   close(sockFD);

   return 0;
}
