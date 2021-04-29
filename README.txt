This program contains 5 programs an encryption and a decryption server/client, as well as a decryption/encryption key generator.
The programs can be run from the command line and compiled using the compileall script in the directory. 
The programs used socket based IPC supporting 5 concurrent sockets.


compile the programs
$ ./compileall
To run the program first, select generate a key:
$ keygen 1000 > mykey
The key is now stored in the mykey file. 
If you have a text file it can be encrypted using the following command, after starting up the servers.
$ enc_server [ENC_PORT #] &
$ dec_server [DEC_PORT #] &
$ enc_client [message.txt] mykey [ENC_PORT #] > ciphertext1
This will store the encrpted plaintext file in the directory as ciphertext1
To decrypt the file run the following command.
$ dec_client ciphertext1 mykey [DEC_PORT #] > plaintext1
This will store the decrypted file as plaintext1 in the same directory.