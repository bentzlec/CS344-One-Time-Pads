#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ctype.h>

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
   //Error check here?
   memcpy((char*)&sockAdd.sin_addr.s_addr, (char*)serverHost->h_addr, serverHost->h_length); //Copy address into struct

   sockFD = socket(AF_INET, SOCK_STREAM, 0); //Make the socket
   //Error check socket?
   
   if(connect(sockFD, (struct sockaddr*)&sockAdd, sizeof(sockAdd)) < 0) {
      error("Client error!\n");
   }





   return 0;
}
