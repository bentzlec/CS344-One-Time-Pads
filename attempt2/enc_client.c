#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

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

   return 0;
}
