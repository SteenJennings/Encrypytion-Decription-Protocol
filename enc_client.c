// Name: Steinar Jennings
// Class: CS344
// Assignment: One Time Pads
// Encryption Client

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>  // ssize_t
#include <sys/socket.h> // send(),recv()
#include <netdb.h>      // gethostbyname()

/**
* Client code
* 1. Create a socket and connect to the server specified in the command arugments.
* 2. Prompt the user for input and send that input as a message to the server.
* 3. Print the message received from the server and exit the program.
*/

// Error function used for reporting issues
void error(const char *msg) { 
  perror(msg); 
  exit(0); 
} 

// Set up the address struct
void setupAddressStruct(struct sockaddr_in* address, 
                        int portNumber, 
                        char* hostname){
 
  // Clear out the address struct
  memset((char*) address, '\0', sizeof(*address)); 

  // The address should be network capable
  address->sin_family = AF_INET;
  // Store the port number
  address->sin_port = htons(portNumber);

  // Get the DNS entry for this host name
  struct hostent* hostInfo = gethostbyname(hostname); 
  if (hostInfo == NULL) { 
    fprintf(stderr, "CLIENT: ERROR, no such host\n"); 
    exit(0); 
  }
  // Copy the first IP address from the DNS entry to sin_addr.s_addr
  memcpy((char*) &address->sin_addr.s_addr, 
        hostInfo->h_addr_list[0],
        hostInfo->h_length);
}

char *readFile(char *fileName, size_t *size)
{
    char *data;
    FILE *inputFile = fopen(fileName,"r");
    if(inputFile == NULL)
    {
        fprintf(stderr, "Could not open %s\n",fileName);             // move to STDERR
        return NULL;
    }

    fseek(inputFile,0L,SEEK_END);   // Set file pointer at end of the file
    *size = ftell(inputFile);       // Query value of file postion indicator (# of bytes into file)
    fseek(inputFile,0L,SEEK_SET);   // Set file pointer back to start of file

    data = malloc(*size+1);

    size_t readBytes = fread(data, 1, *size, inputFile );

    if(readBytes != *size)
    {
        printf("I only read %ld bytes, I wanted to read %ld bytes\n",readBytes, *size);
    }

    fclose(inputFile);

    return data;
}

int validateFile(char *data, size_t size)
{
    for(size_t i=0; i<size; i++)
    {
      // make sure data[i] == ' ' or is in range 'A' thru 'Z'
      if(!(data[i] == ' ' || (data[i] >= 65 && data[i] <= 90)))            
      {
          fprintf(stderr, "enc_client error: input contains bad characters\n");
          return 1;         
      }
    }
    return 0;
}


int main(int argc, char *argv[]) {
  int socketFD, portNumber, charsWritten, charsRead;
  struct sockaddr_in serverAddress;
  size_t plainTextSize, keySize;
  char *plainTextData, *keyData;
  char buffer[1024];
  // Check usage & args
  if (argc < 4) { 
    fprintf(stderr,"USAGE: %s plaintext key port\n", argv[0]); 
    exit(0); 
  } 

  plainTextData = readFile(argv[1], &plainTextSize);

  // Strip trailing newline
  if(plainTextData[plainTextSize-1] == '\n')
  {
      plainTextData[plainTextSize-1] = '\0';
      plainTextSize--;
  }


  keyData = readFile(argv[2], &keySize);

  // Strip trailing newline
  if(keyData[keySize-1] == '\n')
  {
      keyData[keySize-1] = '\0';
      keySize--;
  }

  if(keySize < plainTextSize)
  {
      // key too short fail
      fprintf(stderr, "Error: key '%s' is too short\n", argv[2]);
      free(plainTextData);
      free(keyData);
      return -1;
  }
  keyData[plainTextSize] = '\0';  // NULL terminate our key data at the plainText Message Size

  if(validateFile(plainTextData,plainTextSize) || validateFile(keyData,plainTextSize))
  {
      // Bad data in plaintext file or the keyFile
      return -1;
  }

  // Create a socket
  socketFD = socket(AF_INET, SOCK_STREAM, 0); 
  if (socketFD < 0){
    error("CLIENT: ERROR opening socket");
  }

   // Set up the server address struct
  setupAddressStruct(&serverAddress, atoi(argv[3]), "localhost"); // "localhost"

  // Connect to server
  if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
    error("CLIENT: ERROR connecting");
  }

   sprintf(buffer,"PLAINTEXT %ld",plainTextSize);
   charsWritten = send(socketFD, buffer, strlen(buffer)+1, 0); 
   if (charsWritten < 0){
      error("CLIENT: ERROR writing to socket");
   }

   int n = 0;
   while(n <= plainTextSize) 
   {
       int len = 1024;
       if(n+len > plainTextSize)
       {
           len = plainTextSize - n + 1;  // +1 gets our \0
       }
       charsWritten = send(socketFD, &plainTextData[n], len, 0); 
       if (charsWritten < 0){
          error("CLIENT: ERROR writing to socket");
       }
       n += len;
   }

   n = 0;
   while(n <= plainTextSize) 
   {
       int len = 1024;
       if(n+len > plainTextSize)
       {
           len = plainTextSize - n + 1;  // +1 gets our \0
       }
       charsWritten = send(socketFD, &keyData[n], len, 0); 
       if (charsWritten < 0){
          error("CLIENT: ERROR writing to socket");
       }
       n += len;
   }


  // Get return message from server
  // Clear out the buffer again for reuse
  // Read data from the socket, leaving \0 at end
  n = 0;

  while((charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0)) > 0)
  {
      if(buffer[0] == '!')
      {
          fprintf(stderr, "Error: enc_client can not use dec_server\n");
          break;
      }
      for(int i=0; i<charsRead; i++)
      {
          printf("%c",buffer[i]);
          n++;
      }
      if(n >= plainTextSize) break;
  }
  printf("\n");
  if (charsRead < 0){
    error("CLIENT: ERROR reading from socket");
  }

  // Close the socket
  close(socketFD); 
  return 0;
}