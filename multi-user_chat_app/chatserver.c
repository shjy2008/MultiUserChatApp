#include	"unp.h"

#define NAME_SIZE 40
#define MAX_CHATS 40
#define BUFF_SIZE 100000

#define CHAT_FULL  "Max chats achieved, NO SEAT available :-("
#define WELCOME_MSG "Welcome to chatter: Enter your name with a command like: -name=Junyi\n"
#define CHANGE_NAME "-name="

// Array to contain the sockets for all chat users
int fd_chats[MAX_CHATS];
struct name_rec
{
    char name[NAME_SIZE+1];
 };
typedef struct name_rec NAME_REC;

// Array to hold the names for all chat users
NAME_REC name_chats[MAX_CHATS];
int numChats = 0;
int userNameId = 0;

static inline int enable_reuse_address( int sock_fd);
int  tcp_easy_listen(const char *interface, const char *service, int listen_queue);  
void chatLoop(int fd_listen);
void processChat(int instance, char * buffer, int nread);
void addChatter(int newConnect);
void remove_chatter(int fd_toRemove);

int main (int argc, char **argv) 
{ 
    int                 sock_fd;
    const char* ip;
    const char* port;
    
    // If arguments are not provided, use 127.0.0.1 65530 by default
    if (argc != 3) {
      printf("usage:%s [IP] [PORT] \n" , argv[0]);
      ip = "127.0.0.1";
      port = "65530";
      // return -1;
     }
     else {
      ip = argv[1];
      port = argv[2];
     }
    
    sock_fd = tcp_easy_listen(ip, port, 100);

	chatLoop(sock_fd);
    
    return 0; 
} 


/** 
 * @brief Enable address-reuse on a socket prior to bind().
 *
 * @param sock_fd Socket that has yet to be passed to bind().
 * 
 * @return 0 or -1 on error, in which case errno will be set.
 */

static inline int
enable_reuse_address(
    int sock_fd)
{
#ifndef TCP_EASY_NO_REUSE_ADDRESS
  int on = 1;
  if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on) < 0)
    return -1;
#endif
  return 0;
}

/** 
 * @brief called by the sever to listen for connections over a socket address.
 * bind server address and port 
 */

int                               /* listening socket or -1 on errr */
tcp_easy_listen(
    const char *interface,        /* interface to bind to */
    const char *service,          /* string of port or serv name */
    int listen_queue)             /* argument to listen(2) */
{
  int listen_fd;
  struct sockaddr_in listen_on;
  char ip_str[INET_ADDRSTRLEN];

  listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_fd < 0) {
    perror("server: socket()"); 
    return -1;
  }

  memset(&listen_on, 0, sizeof listen_on);
  listen_on.sin_family = AF_INET;
  if (inet_pton(AF_INET, interface, &listen_on.sin_addr) <= 0) {
    perror("server: inet_pton()"); 
    return -1;
  }

  listen_on.sin_port = htons(strtoul (service, NULL, 10));
  if (listen_on.sin_port == 0) {
    perror("server: htons()"); 
    return -1;
  }
  
  if (enable_reuse_address(listen_fd) < 0) {
    perror("server: enable_reuse_address()"); 
    return -1;
  }

  if (bind(listen_fd, (struct sockaddr *) &listen_on, sizeof listen_on) < 0) {
    perror("server: bind()"); 
    return -1;
  }
  
  if (listen(listen_fd, listen_queue) < 0) {
    perror("server: listen()"); 
    return -1;
  }
    
  if (inet_ntop(AF_INET, (const void *) &listen_on.sin_addr, ip_str, INET_ADDRSTRLEN) != NULL) {
       printf("ChatServer %s is ready to accept chatters via port %u.\n", ip_str, ntohs(listen_on.sin_port));
       printf("..................\n");
  }
   
  return listen_fd;
  
}

/*   The main loop for processing the communciation between the server and the chatters
 *   using select() to control reading data from differnt sockets
 *
*/

void chatLoop(int fd_listen)
{ 
    char buffer[BUFF_SIZE];    
    struct sockaddr_in fsin; 
    socklen_t fromlen = sizeof fsin;
    int i; 
    int maxfd;//, iNumInputs; 
    int newConnect, nread;
    fd_set fdCheck; 
    char ip_str[INET_ADDRSTRLEN];
    
    // The select statement is the key call in this routine. 
    
    for(; ;)
    {
        FD_ZERO(&fdCheck); // Clear all bits in fdCheck
        FD_SET(fd_listen, &fdCheck); // Turn on the bit for fd_accept in fdCheck
        maxfd = fd_listen; // Initialize maxfd with fd_accept

	      // Turn on the bit for each chat user's socket in fdCheck, and update maxfd (fd_chats[MAX_CHATS])
        for (i = 0; i < MAX_CHATS; ++i) {
            FD_SET(fd_chats[i], &fdCheck);
            if (fd_chats[i] > maxfd) {
                maxfd = fd_chats[i];
            }
        }

        // Check the readibility of each socket using select(), and check failure of select();
        int ret = select(maxfd + 1, &fdCheck, NULL, NULL, NULL);
        if (ret < 0) {
            perror("server: select()"); 
            return;
        }

        // Check if need to accept a new client
        // Check the readibility of the listen socket fd_listen using FD_ISSET();
        if (FD_ISSET(fd_listen, &fdCheck)) {
            // If the listen socket is ready for reading, it means that a new chatter is requesting to join.
            // (1) Accept this new chatter using accept().
            newConnect = accept(fd_listen, (SA*)NULL, NULL);
            if (newConnect < 0) {
                perror("server: accept()"); 
                return;
            }

            // (2) Get the new chatter info using getpeername(), and print the followng message on the server:
            //   "Chatter ** connected to the server via port **".
            if (getpeername(newConnect, (SA*)&fsin, &fromlen) < 0) {
                perror("server: getpeername()"); 
                return;
            }
            // Convert address from network to host, used for printing
            if (inet_ntop(AF_INET, &fsin.sin_addr, ip_str, INET_ADDRSTRLEN) == NULL)  {
                perror("server: inet_ntop()");
                return;
            }

            uint16_t port = ntohs(fsin.sin_port); // Convert port from network to host

            printf("Chatter %s connected to the server via port %d\n", ip_str, port);

            // (3) check if the number of existing chatters is no smaller than MAX_CHATS. If yes, send the
            // message in CHAT_FULL the new chatter using write(), and close the new connection; otherwise call
            // the addChatter function to add this new chatter.
            if (numChats >= MAX_CHATS) {
                if (write(newConnect, CHAT_FULL, strlen(CHAT_FULL)) < 0) {
                  perror("server: write(CHAT_FULL)");
                  close(newConnect);
                  return;
                }
                close(newConnect);
            }
            else {
                addChatter(newConnect);
            }
        }

        // for each chatter recorded in the fd_chats[], check the readbility of its socket
        for (i = 0; i < MAX_CHATS; ++i) {
            if (FD_ISSET(fd_chats[i], &fdCheck)) {
                // If it is ready to read, receive the data from the socket using read()
                memset(buffer, 0, sizeof(buffer)); // Clear the buffer before reading data into it
                nread = read(fd_chats[i], buffer, sizeof(buffer) - 1);

                // if the number of bytes read is larger than 0, 
                if (nread > 0) {
                    // (1)  add the NULL terminator '\n'
                    // (2)  call the processChat() function to process the received data
                    // buffer[nread] = '\n';
                    processChat(i, buffer, nread);
                }
                else {
                    // (1) printf("Time to close out chat file descriptor: %d\n", fd_chats[i]);
                    // (2) call the remove_chatter() function to remove this chatter.
                    printf("Time to close out chat file descriptor: %d\n", fd_chats[i]);
                    remove_chatter(i);
                }
            }
        }
    }
}

//************************************************
// processChat called to deal with each chat message
 
void processChat(int instance, char * buffer, int nread)
{
   char outbuff[BUFF_SIZE]; 
   int i, len;
  
   len = strlen(CHANGE_NAME);
   if (strncmp(CHANGE_NAME, buffer, len)== 0)
    {        
    // Special message of  chatter requesting a name something like:
    // -name=Junyi
    // In this case we copy Junyi into the name_chats table

        char oldName[NAME_SIZE + 1];
        memset(oldName, 0, sizeof(oldName));
        memcpy(oldName, &name_chats[instance], NAME_SIZE);

        int nameLength = strlen(buffer) - len - 1; // - 1 because there is a '\n' at the end of the buffer
        if (nameLength > NAME_SIZE) {
          nameLength = NAME_SIZE;
        }
        memset(&name_chats[instance], 0, NAME_SIZE + 1);
        memcpy(&name_chats[instance], &buffer[len], nameLength);

        // Notify the one who changes his name
        char changeNameNoticeBuff[100];
        memset(changeNameNoticeBuff, 0, sizeof(changeNameNoticeBuff));
        int changeNameNoticeLen = snprintf(changeNameNoticeBuff, sizeof(changeNameNoticeBuff), "Your name is changed to: %s\n\n", (char*)&name_chats[instance]);
        if (write(fd_chats[instance], changeNameNoticeBuff, changeNameNoticeLen) < 0) {
          perror("server: write(changeNameNotice)");
          return;
        }

        // Notify the other chatters that someone has changed his name
        memset(changeNameNoticeBuff, 0, sizeof(changeNameNoticeBuff));
        changeNameNoticeLen = snprintf(changeNameNoticeBuff, sizeof(changeNameNoticeBuff), 
            "%s's name is changed to: %s\n\n", oldName, (char*)&name_chats[instance]);
        for (i = 0; i < numChats; ++i) {
            if (i != instance)
                if (write(fd_chats[i], changeNameNoticeBuff, changeNameNoticeLen) < 0) {
                  perror("server: write(changeNameNotice)");
                  return;
                }
        }

        return;
    }
    
   // Prefix buffer with name of user who sent it
   len = sprintf(outbuff, "\n%s: %s\n", 
       (char *)&name_chats[instance], buffer);
   
    // Send the data in outbuff to all the other chatters except the sender using write();
    for (i = 0; i < numChats; ++i) {
        if (i != instance)
            if (write(fd_chats[i], outbuff, len) < 0) {
              perror("server: write(outbuff)");
              return;
            }
    }
}

// *******************************************************
 // addChatter called to deal with a new Chatter connect

void addChatter(int newConnect)
{
  char buffer[BUFF_SIZE];  
  int i,total = 0;  

  // sprintf(buffer,"unknown:%d", numChats);
  sprintf(buffer, "User%d", userNameId);
  ++userNameId;

  strcpy((char *)&name_chats[numChats], buffer);

// Create welcome message along with a list of who is currently
// connected to the chat server
 
  total = sprintf(buffer,"%s\n", WELCOME_MSG);
  if (numChats <= 0)
    total +=sprintf(&buffer[total], "You are the first chatter!\n");
  else
    {
        total += sprintf(&buffer[total], " Current Chatters: ");
        for ( i=0; i < numChats; i++)
        {
          total += sprintf(&buffer[total], " %s ", (char *) &name_chats[i]);  
        }
    }
    
     total += sprintf(&buffer[total], "\n"); 
     total += sprintf(&buffer[total], "Your name: %s\n\n", (char *)&name_chats[numChats]); 

  // Send message back to new connecting chatter
  if (write(newConnect, buffer, total) < 0) {
    perror("server: write(welcome message)");
    return;
  }

  // Add Chatter to fd_chat table
  fd_chats[numChats++] = newConnect;

  // Notice all the other chatters that a new chatter comes
  char newChatterNotice[100];
  int noticeLen = sprintf(newChatterNotice,"A new chatter %s comes into the chat room. Current chatter count: %d\n\n", (char *)&name_chats[numChats], numChats);
  for (i = 0; i < numChats - 1; ++i) {
      if (write(fd_chats[i], newChatterNotice, noticeLen) < 0) {
        perror("server: write(newChatterNotice)");
        return;
      }
  }
}


void remove_chatter(int fd_toRemove)
{
  // Notice all the other chatters that a chatter leaves
  char chatterLeaveNotice[100];
  int noticeLen = sprintf(chatterLeaveNotice,"The chatter %s leaves the chat room. Current chatter count: %d\n\n", (char *)&name_chats[fd_toRemove], numChats - 1);
  for (int i = 0; i < numChats; ++i) {
      if (write(fd_chats[i], chatterLeaveNotice, noticeLen) < 0) {
        perror("server: write(chatterLeaveNotice)");
        return;
      }
  }

  printf("The chatter %s leaves the chat room. Current chatter count: %d\n", (char *)&name_chats[fd_toRemove], numChats - 1);

   int k, fd_save; 
   // close out terminated chat user and compress the
   // chat tables
   fd_save = fd_chats[fd_toRemove];

  // Note: A bug with the code below: if fd_toRemove == 0, fd_chats[0] will not be cleared
  //  for (k=fd_toRemove; k < numChats -1; k++)
  //    {
  //       fd_chats[k] = fd_chats[k+1];
  //       strcpy((char *)&name_chats[k], (char *)&name_chats[k+1]);
  //    }

    for (k = fd_toRemove; k < numChats; ++k) {
      // If the chat users are full and now we need to remove the last one, directly set it to 0
      if (fd_toRemove == MAX_CHATS - 1) {
        fd_chats[fd_toRemove] = 0;
        break;
      }
      else { // Otherwise, set k + 1 to k
        fd_chats[k] = fd_chats[k + 1];
        strcpy((char *)&name_chats[k], (char *)&name_chats[k+1]);
      }
    }
  
   numChats -= 1;
   printf("Closing %d\n", fd_save);
   close(fd_save);
}

