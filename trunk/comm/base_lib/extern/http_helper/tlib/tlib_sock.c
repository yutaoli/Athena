#ifndef _TLIB_SOCK_C_
#define _TLIB_SOCK_C_

#include "tlib_sock.h"

/* Take a service name, and a service type, and return a port number.  If the
   service name is not found, it tries it as a decimal number.  The number
   returned is byte ordered for the network. */
int TLib_Sock_Atoport(service, proto)
char *service;
char *proto;
{
  int port;
  long int lport;
  struct servent *serv;
  char *errpos;

  /* First try to read it from /etc/services */
  serv = getservbyname(service, proto);
  if (serv != NULL)
    port = serv->s_port;
  else { /* Not in services, maybe a number? */
    lport = strtol(service,&errpos,0);
    if ( (errpos[0] != 0) || (lport < 1) || (lport > 65535) )
      return -1; /* Invalid port address */
    port = htons(lport);
  }
  return port;
}

/* Converts ascii text to in_addr struct.  NULL is returned if the address
   can not be found. */
struct in_addr *TLib_Sock_Atoaddr(address)
char *address;
{
  struct hostent *host;
  static struct in_addr saddr;

  /* First try it as aaa.bbb.ccc.ddd. */
  saddr.s_addr = inet_addr(address);
  if (saddr.s_addr != -1) {
    return &saddr;
  }
  host = gethostbyname(address);
  if (host != NULL) {
    return (struct in_addr *) *host->h_addr_list;
  }
  return NULL;
}

/* This function listens on a port, and returns connections.  It forks
   returns off internally, so your main function doesn't have to worry
   about that.  This can be confusing if you don't know what is going on.
   The function will create a new process for every incoming connection,
   so in the listening process, it will never return.  Only when a connection
   comes in, and we create a new process for it will the function return.
   This means that your code that calls it should _not_ loop.

   The parameters are as follows:
     socket_type: SOCK_STREAM or SOCK_DGRAM (TCP or UDP sockets)
     port: The port to listen on.  Remember that ports < 1024 are
       reserved for the root user.  Must be passed in network byte
       order (see "man htons").
     listener: This is a pointer to a variable for holding the file
       descriptor of the socket which is being used to listen.  It
       is provided so that you can write a signal handler to close
       it in the event of program termination.  If you aren't interested,
       just pass NULL.  Note that all modern unixes will close file
       descriptors for you on exit, so this is not required. */
int TLib_Sock_Get_Connection(socket_type, port, listener)
int socket_type;
u_short port;
int *listener;
{
	struct sockaddr_in address;
	int listening_socket;
	int connected_socket = -1;
	int new_process;
	int reuse_addr = 1;

	/* Setup internet address information.  
	This is used with the bind() call */
	memset((char *) &address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = port;
	address.sin_addr.s_addr = htonl(INADDR_ANY);

	listening_socket = socket(AF_INET, socket_type, 0);
	if (listening_socket < 0) {
	perror("socket");
	exit(EXIT_FAILURE);
	}

	if (listener != NULL)
	*listener = listening_socket;

	setsockopt(listening_socket, SOL_SOCKET, SO_REUSEADDR, (void*)(&(reuse_addr)), 
	sizeof(reuse_addr));

	if (bind(listening_socket, (struct sockaddr *) &address, 
	sizeof(address)) < 0) {
	perror("bind");
	close(listening_socket);
	exit(EXIT_FAILURE);
	}

	if (socket_type == SOCK_STREAM) {
	listen(listening_socket, 5); /* Queue up to five connections before
	having them automatically rejected. */

	while(connected_socket < 0) {
	connected_socket = accept(listening_socket, NULL, NULL);
	if (connected_socket < 0) {
	/* Either a real error occured, or blocking was interrupted for
	some reason.  Only abort execution if a real error occured. */
	if (errno != EINTR) {
	perror("accept");
	close(listening_socket);
	exit(EXIT_FAILURE);
	} else {
	continue;    /* don't fork - do the accept again */
	}
	}

	new_process = fork();
	if (new_process < 0) {
	perror("fork");
	close(connected_socket);
	connected_socket = -1;
	}
	else { /* We have a new process... */
	if (new_process == 0) {
	/* This is the new process. */
	close(listening_socket); /* Close our copy of this socket */
	if (listener != NULL)
	*listener = -1; /* Closed in this process.  We are not 
	responsible for it. */
	}
	else {
	/* This is the main loop.  Close copy of connected socket, and
	continue loop. */
	close(connected_socket);
	connected_socket = -1;
	}
	}
	}
	return connected_socket;
	}
	else
	return listening_socket;
}

static int SetNBlock(int iSock)
{
int iFlags;

	iFlags = fcntl(iSock, F_GETFL, 0);
	iFlags |= O_NONBLOCK;
	iFlags |= O_NDELAY;
	fcntl(iSock, F_SETFL, iFlags);
	return 0;
}

static int Create_sock(socket_type, port, listener)
int socket_type;
u_short port;
int *listener;
{
	struct sockaddr_in address;
	int listening_socket;
	int reuse_addr = 1;

	/* Setup internet address information.  
	This is used with the bind() call */
	memset((char *) &address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = port;
	address.sin_addr.s_addr = htonl(INADDR_ANY);

	listening_socket = socket(AF_INET, socket_type, 0);
	if (listening_socket < 0) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	if (listener != NULL)
		*listener = listening_socket;

	setsockopt(listening_socket, SOL_SOCKET, SO_REUSEADDR, (void*)(&(reuse_addr)), 
		sizeof(reuse_addr));

	if (bind(listening_socket, (struct sockaddr *) &address, 
		sizeof(address)) < 0) {
		perror("bind");
		close(listening_socket);
		exit(EXIT_FAILURE);
	}

	if (socket_type == SOCK_STREAM) {
		listen(listening_socket, 5); /* Queue up to five connections before
		having them automatically rejected. */
	}
	return listening_socket;
}

static int CreateUnixSock(int socket_type, char* sSockPath, int* listener)
{
struct sockaddr_un name;
int listening_socket;
int reuse_addr = 1;

	/* Remove previous socket */
	unlink(sSockPath);

	/* Setup Unix Domain name information.  
	This is used with the bind() call */
	memset((char *) &name, 0, sizeof(name));
	name.sun_family = AF_UNIX;
	strncpy(name.sun_path, sSockPath, sizeof(name.sun_path));

	listening_socket = socket(AF_UNIX, socket_type, 0);
	if (listening_socket < 0) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	if (listener != NULL)
	*listener = listening_socket;

	setsockopt(listening_socket, SOL_SOCKET, SO_REUSEADDR, (void*)(&(reuse_addr)), 
	sizeof(reuse_addr));

	if (bind(listening_socket, (struct sockaddr *) &name, 
	sizeof(name)) < 0) {
		perror("bind");
		close(listening_socket);
		exit(EXIT_FAILURE);
	}

	if (socket_type == SOCK_STREAM) {
		listen(listening_socket, 5); /* Queue up to five connections before
		having them automatically rejected. */
	}
	return listening_socket;
}

static int ConnectUnixSock(int socket_type, char* sSockPath)
{
struct sockaddr_un name;
int sock;

	sock = socket(AF_UNIX, socket_type, 0);
	if (sock < 0) {
		perror("socket");
		return -1;
	}
	memset((char *) &name, 0, sizeof(name));
	name.sun_family = AF_UNIX;
	strncpy(name.sun_path, sSockPath,strlen(sSockPath)+1);

	if (connect(sock, (struct sockaddr*)&name, sizeof(name)) < 0) {
		perror("connect");
		close(sock);
		return -1;
	}
	return sock;
}

/* This is a generic function to make a connection to a given server/port.
   service is the port name/number,
   type is either SOCK_STREAM or SOCK_DGRAM, and
   netaddress is the host name to connect to.
   The function returns the socket, ready for action.*/
int TLib_Sock_Make_Connection(service, type, netaddress)
char *service;
int type;
char *netaddress;
{
  /* First convert service from a string, to a number... */
  int port = -1;
  struct in_addr *addr;
  int sock, connected;
  struct sockaddr_in address;

	if (type == SOCK_STREAM) 
		port = TLib_Sock_Atoport(service, "tcp");
	if (type == SOCK_DGRAM)
		port = TLib_Sock_Atoport(service, "udp");
	if (port == -1) {
		fprintf(stderr,"TLib_Sock_Make_Connection:  Invalid socket type.\n");
		return -1;
	}
	addr = TLib_Sock_Atoaddr(netaddress);
	if (addr == NULL) {
		fprintf(stderr,"TLib_Sock_Make_Connection:	Invalid network address.\n");
		return -1;
	}
 
	memset((char *) &address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = (port);
	address.sin_addr.s_addr = addr->s_addr;

	sock = socket(AF_INET, type, 0);

	// printf("Connecting to %s on port %d.\n",inet_ntoa(*addr),htons(port));

	if (type == SOCK_STREAM) {
		connected = connect(sock, (struct sockaddr *) &address, 
			sizeof(address));
		if (connected < 0) {
			perror("connect");
			close(sock);
			return -1;
		}
		return sock;
	}
	/* Otherwise, must be for udp, so bind to address. */
	if (bind(sock, (struct sockaddr *) &address, sizeof(address)) < 0) {
		perror("bind");
		close(sock);
		return -1;
	}
	return sock;
}

/* This is just like the read() system call, accept that it will make
   sure that all your data goes through the socket. */
int TLib_Sock_Read(sockfd, buf, count)
int sockfd;
char *buf;
size_t count;
{
  size_t bytes_read = 0;
  int this_read;

  while (bytes_read < count) {
    do
      this_read = read(sockfd, buf, count - bytes_read);
    while ( (this_read < 0) && (errno == EINTR) );
    if (this_read < 0)
      return this_read;
    else if (this_read == 0)
      return bytes_read;
    bytes_read += this_read;
    buf += this_read;
  }
  return count;
}

/* This is just like the write() system call, accept that it will
   make sure that all data is transmitted. */
int TLib_Sock_Write(sockfd, buf, count)
int sockfd;
char *buf;
size_t count;
{
  size_t bytes_sent = 0;
  int this_write;

  while (bytes_sent < count) {
    do
      this_write = write(sockfd, buf, count - bytes_sent);
    while ( (this_write < 0) && (errno == EINTR) );
    if (this_write <= 0)
      return this_write;
    bytes_sent += this_write;
    buf += this_write;
  }
  return count;
}

/* This function reads from a socket, until it recieves a linefeed
   character.  It fills the buffer "str" up to the maximum size "count".

   This function will return -1 if the socket is closed during the read
   operation.

   Note that if a single line exceeds the length of count, the extra data
   will be read and discarded!  You have been warned. */
int TLib_Sock_Gets(sockfd, str, count)
int sockfd;
char *str;
size_t count;
{
  int bytes_read;
  int total_count = 0;
  char *current_position;
  char last_read = 0;

  current_position = str;
  while (last_read != 10) {
    bytes_read = read(sockfd, &last_read, 1);
    if (bytes_read <= 0) {
      /* The other side may have closed unexpectedly */
      return -1; /* Is this effective on other platforms than linux? */
    }
    if ( (total_count < count) && (last_read != 10) && (last_read !=13) ) {
      current_position[0] = last_read;
      current_position++;
      total_count++;
    }
  }
  if (count > 0)
    current_position[0] = 0;
  return total_count;
}

/* This function writes a character string out to a socket.  It will 
   return -1 if the connection is closed while it is trying to write. */
int TLib_Sock_Puts(sockfd, str)
int sockfd;
char *str;
{
  return TLib_Sock_Write(sockfd, str, strlen(str));
}

/* This ignores the SIGPIPE signal.  This is usually a good idea, since
   the default behaviour is to terminate the application.  SIGPIPE is
   sent when you try to write to an unconnected socket.  You should
   check your return codes to make sure you catch this error! */
void TLib_Sock_Ignore_pipe(void)
{
  struct sigaction sig;

  sig.sa_handler = SIG_IGN;
  sig.sa_flags = 0;
  sigemptyset(&sig.sa_mask);
  sigaction(SIGPIPE,&sig,NULL);
}

static int SingleSend(int iSock, char* sBuf, int iLen)
{
fd_set WriteFds;
int iNfds;
struct timeval stZerotv;

	if (iSock < 0) return -1;
	FD_ZERO(&WriteFds);
	FD_SET(iSock, &WriteFds);
	stZerotv.tv_sec = stZerotv.tv_usec = 0;
	iNfds = iSock + 1;

	if (select(iNfds, NULL, &WriteFds, NULL, &stZerotv) > 0) {
		if (FD_ISSET(iSock, &WriteFds)) 
			return send(iSock, sBuf, iLen, 0);
	}
	return 0;
}

static void MultiSend(fd_set* WriteFds, int iNfds, char* sBuf, int iLen)
{
register int iFd;
struct timeval stZerotv;

	stZerotv.tv_sec = stZerotv.tv_usec = 0;
	iLen = strlen(sBuf);

	if (select(iNfds, NULL, WriteFds, NULL, &stZerotv) > 0) {
	for (iFd = 3; iFd < iNfds; iFd++)
	if (FD_ISSET(iFd, WriteFds)) 
	send(iFd, sBuf, iLen, 0);
	}
}

// Not block read line, return 1 when match '\n' or read iBytes.
static int SockReadln(int iSock, char* sStr, int iBytes)
{
int iLen;
char* sLn;

	if ((iLen = recv(iSock, sStr, iBytes, MSG_PEEK)) <= 0) return -1;
	sStr[iLen - 1] = 0;
	if (sLn = strchr(sStr, '\n')) {
		iLen = sLn - sStr + 1;
	} else if (iLen != iBytes) return 0; 
	if ((iLen = recv(iSock, sStr, iLen, MSG_PEEK)) <= 0) return -1;
	sStr[iLen - 1] = 0;
	return 1;
}

#endif

