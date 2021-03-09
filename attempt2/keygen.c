#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

int main(int argc, char** argv) {
   int keylength = 0;
   
   if (argc < 2) {
      printf("Not enough command-line arguments\n");
      fflush(stdout);
      exit(1);
   }

   keylength = atoi(argv[1]); //Get the desire key length from command line arguments array
   char key[(keylength + 1)]; //Make buffer to hold the key

   char * alphaNum = "ABCDEFGHIJKLMNOPQRSTUVWXYZ "; //array to hold the characters to be randomly selected for the key

   srand(time(NULL));

   for(int i = 0; i < keylength; i++) {
      int num = random() % 27; //Get a random number between 1-26
      key[i] = alphaNum[num]; //Copy random character into key buffer
   }
   key[keylength] = 0;

   printf("%s\n", key);

   return 0;
}
