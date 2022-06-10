/* socket.cc
 *
 * Wrapper classes for the C socket API and related functions.
 */


#include "socket.h"
#include <strings.h>
#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>


/* ------------- Socket class --------------------- */

Socket::Socket( void )
{
  Active = false;
  
    /* Open the socket. */
  SockNum = socket(PF_INET, SOCK_STREAM, 0);
  if( SockNum < 0 )
    perror("Socket::Socket(): can not open socket."); 
}

bool 
Socket::connect(const char *host, int port)
{
    /* Translate the host into a hostent struct. */
  hostent *hostIP = gethostbyname( host );
  
    /* See if the host name was valid. */
  if( !hostIP ) 
    perror("ClientSocket::connect(): unknown host"); 

     /* Build the address data structure. */
  sockaddr_in addr;
  bzero((char *)&addr, sizeof( addr ));

  addr.sin_family = AF_INET;
  bcopy(hostIP->h_addr_list[0], (char *)&addr.sin_addr, 4);
  addr.sin_port = htons(port);

     /* Connect the socket. */
  if( ::connect(SockNum, (sockaddr *)&addr, sizeof( addr )) < 0 )
    return false;
  else 
    return Active = true;
}

void 
Socket::close( void )
{
  if( SockNum < 0 ) return;
  int result = ::close(SockNum);
  if( result < 0 ) 
    perror("Socket::close(): Error closing socket.");
  SockNum = -1;
  Active = false;
}


int 
Socket::send(void *data, int len)
{
  if(!Active)
    perror("Socket::send(): can not send on a closed socket.");
  int result = ::send(SockNum, data, len, 0);
  if( result < 0 )
    perror("Socket::send(): Error sending data on socket.");
  return result;
}


int 
Socket::recv( void *data, int maxLen )
{
  if(!Active) 
    perror("Socket::recv(): can not receive on a closed socket.");  
  int result = ::recv(SockNum, data, maxLen, 0); //MSG_WAITALL);  // 0
  if( result < 0 ) 
    if( errno == ECONNRESET ) 
      return 0;
    else 
      perror( "Socket::recv(): Error reading data from socket." );
  return result;
}

int 
Socket::gets(char *buffer, int maxLen)
{
  if(!Active) 
    perror("Socket::recv(): can not receive on a closed socket.");
  int i = 0;  
  int result = ::read(SockNum, buffer, 1);
  while( result > 0 && i < maxLen && *buffer != '\n' ) {
    i++;
    buffer++;
    result = ::read(SockNum, buffer, 1);
  }
  
  if(result > 0) {
    buffer++;
    *buffer = 0;
    return i + 1;
  }
  else 
    return result;
}


int 
Socket::port() const
{
  if( !Active )
    perror("Socket::remPort(): socket is not active.");
  
  sockaddr_in tempAddr;
  int tempLen = sizeof(tempAddr);
  getpeername(SockNum, (sockaddr *) &tempAddr, (socklen_t *) &tempLen);
                        
  unsigned char *tmp = (unsigned char *) &tempAddr.sin_addr;                        
  return ntohs(tempAddr.sin_port);
}


const char *
Socket::address() const
{
  if( !Active )
    perror("Socket::remAddress(): socket is not active.");

  static sockaddr_in tempAddr;
  int tempLen = sizeof( tempAddr );
  getpeername(SockNum, (sockaddr *) &tempAddr, (socklen_t *) &tempLen);
  
  return inet_ntoa(tempAddr.sin_addr);
}

Socket::Socket( int thesock )
{
  SockNum = thesock;
  Active = true;
}


/* ------------- ServerSocket class --------------------- */

ServerSocket::ServerSocket( void )
{
  Active = false;
  
    /* Open the socket. */
  SockNum = socket(PF_INET, SOCK_STREAM, 0);
  if( SockNum < 0 )
    perror("ServerSocket::ServerSocket(): can not open socket."); 
}


bool 
ServerSocket::bind(int port, int queueSize)
{
     /* Build the address data structure. */
  sockaddr_in addr;
  bzero( (char *)&addr, sizeof( addr ) );

  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);  // use my address.
  addr.sin_port = htons(port); 

  if( ::bind(SockNum, (sockaddr *)&addr, sizeof( addr ) ) < 0) {
    close();
    return false;
  }
    
     /* Listen on the socket for connections from clients. */
  if( ::listen(SockNum, queueSize) < 0 ) {
    close();
    return false;
  }
  
  return Active = true;
}


Socket *
ServerSocket::accept()
{
  sockaddr_in clientAddr;
  int addrlen = sizeof( clientAddr );

     /* Accept a connection. */
  int sock = ::accept( SockNum, 0, 0 );
            
  if( sock < 0 )   // could not accept connection.
    perror("ServerSocket::accept(): could not accept connection.");

     /* Create a new socket object for this socket fd. */
  return new Socket( sock );
}
  

void 
ServerSocket::close( void )
{
  if( SockNum < 0 ) return;
  int result = ::close(SockNum);
  if( result < 0 ) 
    perror("ServerSocket::close(): Error closing socket.");
  SockNum = -1;
  Active = false;
}

int 
ServerSocket::port() const
{
  if( !Active )
    perror("ServerSocket::port(): socket is not active.");
  
  sockaddr_in tempAddr;
  int tempLen = sizeof(tempAddr);
  getsockname(SockNum, (sockaddr *) &tempAddr, (socklen_t *) &tempLen);
                        
  unsigned char *tmp = (unsigned char *) &tempAddr.sin_addr;                        
  return ntohs(tempAddr.sin_port);
}


