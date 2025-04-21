#include	"unp.h"

#define BUFF_SIZE 10000 

void chatUser(const char *hostname, const char *aPort);
 
int main (int argc, char *argv[]) 
{ 
    const char* ip;
    const char* port;
  
    // If arguments are not provided, use 127.0.0.1 65530 by default
    if (argc != 3) {
      printf("usage:%s [IP] [PORT] \n" , argv[0]);
        ip = "127.0.0.1";
        port = "65530";
      //return -1;
     }
     else {
		ip = argv[1];
		port = argv[2];
     }

    printf("\nStarting chat session: host(%s) port(%s)\n", 
        ip, port);    
    
    chatUser(ip, port);
    
    return 0; 
} 

void chatUser(const char *hostname, const char *aPort)
{  
    unsigned char buffer[BUFF_SIZE]; 
    struct sockaddr_in sin; 
    //int nread; 
    int fd_chat  = -1; 
    int maxfd;//, iNumInputs; 
    fd_set fdCheck; 
    
    // Now we need to create a socket connection with the chat server

    if ((fd_chat  = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
         perror("client: socket()"); 
         exit(-1); 
    } 
    else 
    {    
        sin.sin_family = AF_INET; 
        sin.sin_port = htons(strtoul (aPort, NULL, 10)); 
        Inet_pton(AF_INET, hostname, &sin.sin_addr);
          
        if (connect(fd_chat , (struct sockaddr *)&sin, sizeof(sin)) < 0) 
        { 
            perror("client: connect()"); 
            exit(-1); 
        } 
    } 
    
    for (; ;) {
        FD_ZERO(&fdCheck); // Clear all bits in fdCheck
        FD_SET(fd_chat, &fdCheck); // Turn on the bit for fd_chat in fdCheck
        maxfd = fd_chat; // Initialize maxfd with fd_chat

        FD_SET(STDIN_FILENO, &fdCheck); // Turn on the bit for STDIN_FILENO in fdCheck
        maxfd = max(maxfd, STDIN_FILENO); // Update maxfd if needed

        // Check the readibility of each socket using select(), and check failure of select()
        int ret = select(maxfd + 1, &fdCheck, NULL, NULL, NULL);
        if (ret < 0) {
            perror("client: select()"); 
            exit(-1);
        }

        // Check the readibility of the input fd using FD_ISSET();
        if (FD_ISSET(STDIN_FILENO, &fdCheck)) {
            memset(buffer, 0, sizeof(buffer)); // Clear the buffer before reading data into it
            if (Fgets((char*)buffer, MAXLINE, stdin) != NULL) {
                Writen(fd_chat, buffer, strlen((const char*)buffer));
            }
        }

        // Check the readibility of the client socket fd using FD_ISSET();
        if (FD_ISSET(fd_chat, &fdCheck)) {
            memset(buffer, 0, sizeof(buffer)); // Clear the buffer before reading data into it
            if (Read(fd_chat, buffer, MAXLINE) > 0) {
                Fputs((const char*)buffer, stdout);
            }
        }
    }
   
    if (fd_chat  >= 0) 
        close(fd_chat ); 
    exit(0); 

}