// Name: Steinar Jennings
// Class: CS344
// Assignment: One Time Pads
// Key Generator

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// define --
int main(int argc, char *argv[]) {
    int keyLength = 0;
    int randomNum;
    int idx = 0;
    srand(time(NULL));
    if (argc < 2) { 
        fprintf(stderr,"To run the keygen executable"); 
        exit(0); 
    } 
    keyLength = atoi(argv[1]);
    if (keyLength < 1) {
        fprintf(stderr, "Key Length of '0' results does not encode");
        exit(1);
    }
    // 
    for (idx = 0; idx < keyLength; idx++) {
        char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
        char randomChar = alphabet[rand() % 27];
        printf("%c", randomChar);
    }
    // prints the newline character after encoding the string
    printf("\n");
}