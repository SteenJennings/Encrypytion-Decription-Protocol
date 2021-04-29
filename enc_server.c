// Name: Steinar Jennings
// Class: CS344
// Assignment: One Time Pads
// Encryption Server

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

// Error function used for reporting issues
void error(const char *msg)
{
    perror(msg);
    exit(1);
}

// Set up the address struct for the server socket
void setupAddressStruct(struct sockaddr_in* address,
                        int portNumber)
{

    // Clear out the address struct
    memset((char*) address, '\0', sizeof(*address));

    // The address should be network capable
    address->sin_family = AF_INET;
    // Store the port number
    address->sin_port = htons(portNumber);
    // Allow a client at any address to connect to this server
    address->sin_addr.s_addr = INADDR_ANY;
}

int main(int argc, char *argv[])
{
    int connectionSocket, charsRead;
    char buffer[1024];
    char *plainText, *keyText;
    int msgSize;
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t sizeOfClientInfo = sizeof(clientAddress);
    pid_t pid;
    int p, k;

    // Check usage & args
    if (argc < 2)
    {
        fprintf(stderr,"USAGE: %s port\n", argv[0]);
        exit(1);
    }

    // Create the socket that will listen for connections
    int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket < 0)
    {
        error("ERROR opening socket");
    }

    // Set up the address struct for the server socket
    setupAddressStruct(&serverAddress, atoi(argv[1]));

    // Associate the socket to the port
    if (bind(listenSocket,
             (struct sockaddr *)&serverAddress,
             sizeof(serverAddress)) < 0)
    {
        error("ERROR on binding");
    }

    // Start listening for connetions. Allow up to 5 connections to queue up
    listen(listenSocket, 5);

    // Accept a connection, blocking if one is not available until one connects
    while(1)
    {
        // Accept the connection request which creates a connection socket
        connectionSocket = accept(listenSocket,
                                  (struct sockaddr *)&clientAddress,
                                  &sizeOfClientInfo);
        if (connectionSocket < 0)
        {
            error("ERROR on accept");
        }

        pid = fork();
        switch(pid)
        {
        case -1:
            error("Fork failed\n");
            exit(1);
        case 0:
            // This is the child process! (this is where we process incoming encryption results)
            printf("SERVER: Connected to client running at host %d port %d\n",
                   ntohs(clientAddress.sin_addr.s_addr),
                   ntohs(clientAddress.sin_port));

            // Read the client's message from the socket and store the message on the buffer
            charsRead = recv(connectionSocket, buffer, 1024, 0);
            if (charsRead < 0)
            {
                error("ERROR reading from socket");
            }
            printf("SERVER: I received this from the client: \"%s\"\n", buffer);

            // Check for plaintext keyword
            if(strncmp(buffer,"PLAINTEXT",9) != 0)
            {
                // server mismatch check send message warning client and exit
                sprintf(buffer,"!!!!!!!!!!!!!!!!!!!!");
                if(send(connectionSocket, buffer, 20, 0)<0)
                {
                    fprintf(stderr,"SERVER: Could not return an erro message to the client\n");
                }
                return -1;
            }            
            msgSize = atoi(&buffer[10]); 
            plainText = (char*)malloc(msgSize+1);   // allocates memory equivalent to the size of the chunk
            keyText = (char*)malloc(msgSize+1);     
            p = 0;
            k = 0;
            // We start "i" as the length of the buffer+1 becuase there is a null terminator in the buffer before the plaintext
            for(int i=strlen(buffer)+1; i < charsRead; i++) 
            {   
                // run this condition to fill out plainText variable with the buffer elements until the full message is received
                if(p<=msgSize)
                {
                    plainText[p] = buffer[i];
                    p++;
                }
                // once plainText is finished, we start storing our keytext values. This can be longer, but not shorter than plainText
                else
                {
                    keyText[k] = buffer[i];
                    k++;
                }
            }
            // this section addresses instances where the key didn't fit on the buffer, so we need to keep looking for it.
            if(k < msgSize)
            {
                while((charsRead = recv(connectionSocket, buffer, 1024, 0)) > 0)
                {
                    for(int i=0; i < charsRead; i++)
                    {
                        if(p<=msgSize)
                        {
                            plainText[p] = buffer[i];       // add the rest of the plaintext from the buffer
                            p++;
                        }
                        else
                        {
                            keyText[k] = buffer[i];         // otherwise add remaining key values
                            k++;

                        }
                    }
                    if(k >= msgSize)
                    {
                        break;
                    }
                }
            }
            // this function encrypts our value
            for(int i=0; i<msgSize; i++)
            {   
                char cp = plainText[i];     // plainText character
                char ck = keyText[i];       // keyText char
                if(ck == ' ')           
                {
                    ck = 26;                // if the keyText char is ' ' then we need to add 26 to our plainText char.               
                }
                else
                {
                    ck = ck - 'A';          // otherwise, the keyText char is equal to the distance from the start of the alphabet
                }
                if(cp == ' ')
                {
                    cp = 26;                
                }
                else
                {
                    cp = cp - 'A';
                }

                char cc = (cp+ck)%27;       // plainText char + keyText char % 27 will get us the proper alphabetical rotation
                if(cc == 26)
                {
                    cc = ' ';               // edge case for a ' '
                }
                else
                {
                    cc = 'A' + cc;          // otherwise, we want to return the distance from the start of the alphabet after rotating
                }
                plainText[i] = cc;
            }

            // Send a Success message back to the client
            charsRead = send(connectionSocket,
                             plainText, msgSize, 0);
            if (charsRead < 0)
            {
                error("ERROR writing to socket");
            }
            // Close the connection socket for this client
            close(connectionSocket);
            free(plainText);
            free(keyText);
            return 0;

        default:
            // This is the parent process (we should just go back to listening for more connections)
            break;
        }


    }
    // Close the listening socket
    close(listenSocket);
    return 0;
}

