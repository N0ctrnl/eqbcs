// EQBCS core functions
#include "EQBCS.h"

// ---------------------------------------------------------------------
// Globals
// ---------------------------------------------------------------------

CEqbcs *runInstance = NULL;
const char *Title   = PROG_TITLE;
const char *Version = PROG_VERSION;

int    EQBCS_TraceSockets=0;
int    EQBCS_iDebugMode = 1;

// #define SOCKTRACE

// ---------------------------------------------------------------------
// Constants & statics
// ---------------------------------------------------------------------

const int CCharBufNode::CHUNKSIZE=      512;

const int CClientNode::MAX_CHARNAMELEN= 50;
const int CClientNode::PING_SECONDS=    50;
// CMD_BUFSIZE Must be longer than MAX_CHARNAMELEN - see code.
// Also, must be large enough to handle NetBots msgs.
const int CClientNode::CMD_BUFSIZE=     1024;

const unsigned char CClientNode::MSG_TYPE_NORMAL=   1;
const unsigned char CClientNode::MSG_TYPE_NBMSG=    2;
const unsigned char CClientNode::MSG_TYPE_MSGALL=   3;
const unsigned char CClientNode::MSG_TYPE_TELL=     4;
const unsigned char CClientNode::MSG_TYPE_CHANNELS= 5;
const unsigned char CClientNode::MSG_TYPE_BCI=      6;

unsigned int CClientNode::suiNextIDNum=   0;

const int CSockio::OKAY=       0;
const int CSockio::CLOSEERR=   -1;
const int CSockio::READERR=    -2;
const int CSockio::WRITEERR=   -3;
const int CSockio::BADSOCK=    -4;
const int CSockio::BADPARM=    -5;
const int CSockio::NOSOCK=     -6;
const int CSockio::NOCONN=     -7;

const int CEqbcs::MAX_CLIENTS  = 50;
const int CEqbcs::DEFAULT_PORT = 2112;

// ---------------------------------------------------------------------
// Debug
// ---------------------------------------------------------------------

int CTrace::iTracef(const char *fmt,...)
{
  // Trace to stdout if TRACE defined

  char     temp_str[512];
  va_list  arg_ptr;
  int      str_len;

  if (EQBCS_iDebugMode == 0)
  {
    return (0);
  }

  va_start(arg_ptr,fmt); // get a pointer to the variable arguement
  str_len=vsprintf(temp_str,fmt,arg_ptr); // print the formatted string i
  va_end(arg_ptr);

  if(str_len>0)      // good string to transmit
  {
    fprintf(stderr, "dbg:%s\n", temp_str);
  }

  fflush(stderr);

  return (0);
}

// for pure debugging, no checks

int CTrace::dbg(const char *fmt,...)
{
  // Trace to stdout if TRACE defined
  char     temp_str[512];
  va_list  arg_ptr;
  int      str_len;

  va_start(arg_ptr,fmt); // get a pointer to the variable arguement
  str_len=vsprintf(temp_str,fmt,arg_ptr); // print the formatted string i
  va_end(arg_ptr);

  if(str_len>0) {     // good string to transmit
    fprintf(stdout, "dbg:%s\n", temp_str);
  }

  fflush(stderr);
  return (0);
}

// ---------------------------------------------------------------------
// Network Read/Write functions
// ---------------------------------------------------------------------

// First three are windows only
#ifdef UNIXWIN
void CSockio::vPrintSockErr(void)
{
  int iErrNo;

  iErrNo = WSAGetLastError();

  switch (iErrNo)
  {
  default: CTrace::iTracef("Unknown error %d\n", iErrNo); break;

  case WSANOTINITIALISED  : CTrace::iTracef("A successful WSAStartup must occur before using this function.\n"); break;
  case WSAENETDOWN        : CTrace::iTracef("The Windows Sockets implementation has detected that the network subsystem has failed.\n"); break;
  case WSAEAFNOSUPPORT    : CTrace::iTracef("The specified address family is not supported.\n"); break;
  case WSAEINPROGRESS     : CTrace::iTracef("A blocking Windows Sockets operation is in progress.\n"); break;
  case WSAEMFILE  : CTrace::iTracef("No more file descriptors are available.\n"); break;
  case WSAENOBUFS : CTrace::iTracef("No buffer space is available. The socket cannot be created.\n"); break;
  case WSAEPROTONOSUPPORT : CTrace::iTracef("The specified protocol is not supported.\n"); break;
  case WSAEPROTOTYPE      : CTrace::iTracef("The specified protocol is the wrong type for this socket.\n"); break;
  case WSAESOCKTNOSUPPORT : CTrace::iTracef("The specified socket type is not supported in this address family.\n"); break;
  case WSAEADDRINUSE      : CTrace::iTracef("The specified address is already in use.\n"); break;
  case WSAEINTR   : CTrace::iTracef("The (blocking) call was canceled using WSACancelBlockingCall.\n"); break;
  case WSAEADDRNOTAVAIL   : CTrace::iTracef("The specified address is not available from the local computer.\n"); break;
  case WSAECONNREFUSED    : CTrace::iTracef("The attempt to connect was forcefully rejected.\n"); break;
  case WSAEHOSTUNREACH    : CTrace::iTracef("Host unreachable!\n"); break;
  // case WSAEDESTADDREQ  : CTrace::iTracef("A destination address is required.\n"); break;
  case WSAEFAULT  : CTrace::iTracef("The namelen argument is incorrect.\n"); break;
  case WSAEINVAL  : CTrace::iTracef("The socket is not already bound to an address.\n"); break;
  case WSAEISCONN : CTrace::iTracef("The socket is already connected.\n"); break;
  case WSAENETUNREACH     : CTrace::iTracef("The network can't be reached from this host at this time.\n"); break;
  case WSAENOTSOCK        : CTrace::iTracef("The descriptor is not a socket.\n"); break;
  case WSAETIMEDOUT       : CTrace::iTracef("Attempt to connect timed out without establishing a connection.\n"); break;
  case WSAEWOULDBLOCK     : CTrace::iTracef("The socket is marked as nonblocking and the connection cannot be completed immediately. It is possible to select the socket while it is connecting by selecting it for writing.\n"); break;
  case WSAHOST_NOT_FOUND  : CTrace::iTracef("Authoritative Answer Host not found.\n"); break;
  case WSATRY_AGAIN       : CTrace::iTracef("Non-Authoritative Host not found, or SERVERFAIL.\n"); break;
  case WSANO_RECOVERY     : CTrace::iTracef("Nonrecoverable errors: FORMERR, REFUSED, NOTIMP.\n"); break;
  case WSANO_DATA : CTrace::iTracef("Valid name, no data record of requested type.\n"); break;

  case WSAEACCES  : CTrace::iTracef("The requested address is a broadcast address, but the appropriate flag was not set.\n"); break;
  case WSAENETRESET       : CTrace::iTracef("The connection must be reset because the Windows Sockets implementation dropped it.\n"); break;
  case WSAENOTCONN        : CTrace::iTracef("The socket is not connected.\n"); break;
  case WSAEOPNOTSUPP      : CTrace::iTracef("MSG_OOB was specified, but the socket is not of type SOCK_STREAM.\n"); break;
  case WSAESHUTDOWN       : CTrace::iTracef("The socket has been shutdown; it is not possible to send on a socket after shutdown has been invoked with how set to 1 or 2.\n"); break;
  case WSAEMSGSIZE        : CTrace::iTracef("The socket is of type SOCK_DGRAM, and the datagram is larger than the maximum supported by the Windows Sockets implementation.\n"); break;
  case WSAECONNABORTED    : CTrace::iTracef("The virtual circuit was aborted due to timeout or other failure.\n"); break;
  case WSAECONNRESET      : CTrace::iTracef("The virtual circuit was reset by the remote side.\n"); break;
  }
}

// ---------------------------------------------------------------------
void CSockio::vShutdownSockets(void)
{
  // Shutdown sockets
  WSACancelBlockingCall();
  WSACleanup();
}

// ---------------------------------------------------------------------
int CSockio::iStartupSockets(int iVerbose)
{
  // Make sure that version 1.1
  WORD wVersionRequested;
  WSADATA wsaData;
  int err;

  wVersionRequested = MAKEWORD(WS_MAJOR, WS_MINOR);

  err = WSAStartup(wVersionRequested, &wsaData);

  if (err != 0) {
    if (iVerbose) {
      CTrace::iTracef("Could not find a useable winsock dll: %d\n", err);

      switch (err)
      {
      default: CTrace::iTracef("Unknown\n"); break;
      case WSASYSNOTREADY: CTrace::iTracef("Indicates that the underlying network subsystem is not ready for network communication.\n"); break;
      case WSAVERNOTSUPPORTED: CTrace::iTracef("The version of Windows Sockets support requested is not provided by this particular Windows Sockets implementation.\n"); break;
      case WSAEINVAL: CTrace::iTracef("The Windows Sockets version specified by the application is not supported by this\n"); break;
      }
    }

    // Tell the user that we couldn't find a useable winsock.dll.
    return -1;
  }
  else {
    if (iVerbose) {
      CTrace::iTracef("Successful WSAStartup!\n");
    }
  }

  // Confirm that the Windows Sockets DLL supports 1.1.  Note that if the
  // DLL supports versions greater than 1.1 in addition to 1.1, it will
  // still return 1.1 in wVersion since that is the version we requested.

  if ( LOBYTE( wsaData.wVersion ) != 1 ||
    HIBYTE( wsaData.wVersion ) != 1 ) {
    // Tell the user that we couldn't find a useable winsock.dll.
    return -1;
  }

  if (iVerbose > 1) {
    CTrace::iTracef("Printf winsock info\n");

    CTrace::iTracef("Version %d.%d\n", (int)LOBYTE(wsaData.wVersion), (int)HIBYTE(wsaData.wVersion));
    CTrace::iTracef("High Version %d.%d\n", (int)LOBYTE(wsaData.wHighVersion), (int)HIBYTE(wsaData.wHighVersion));
    CTrace::iTracef("Description: %s\n", wsaData.szDescription);
    CTrace::iTracef("Status: %s\n", wsaData.szSystemStatus);
    CTrace::iTracef("Max Sockets: %d\n", (unsigned)wsaData.iMaxSockets);
    CTrace::iTracef("Max UPD Datagram size: %d\n", (unsigned)wsaData.iMaxUdpDg);
  }
  return 0;
  // The Windows Sockets DLL is acceptable. Proceed.
}
#endif

// End of windows specific

// Put in empty ones for unix versions
#ifndef UNIXWIN
void CSockio::vPrintSockErr(void){}
void CSockio::vShutdownSockets(void){}
int CSockio::iStartupSockets(int iVerbose){return 0;}
#endif

// ---------------------------------------------------------------------
int CSockio::iReadSock(int iSocketHandle, void *pBuffer, int iSize, int *piBytesRead)
{
  // Reads until all bytes have been read or there is nothing left on the
  // socket Passes back number of bytes read in *piBytesRead Returns
  // CSockio::OKAY, or CSockio::READERR if error On error, socket should be
  // closed by calling functions

  int             iNbrRead = 1;
  int             iSizeLeft;
  int             iTotalRead = 0;
  char            *pBuf;

#ifdef SOCKTRACE
  CTrace::iTracef("SOCK:iReadSock\n");
  fflush(stdout);
#endif

  if (piBytesRead) {
    *piBytesRead = 0;
  }

  if (pBuffer == NULL) {
#ifdef SOCKTRACE
    CTrace::iTracef("SOCK:Bad Parm\n");
    fflush(stdout);
#endif
    return (CSockio::BADPARM);
  }

#ifdef SOCKTRACE
  CTrace::iTracef("SOCK:Read -\n");
  fflush(stdout);
#endif

  pBuf = (char*) pBuffer;

#ifdef SOCKTRACE
  CTrace::iTracef("SOCK:Read -\n");
  fflush(stdout);
#endif

  iSizeLeft = iSize;

#ifdef SOCKTRACE
  CTrace::iTracef("SOCK:Reading %d bytes from %d\n", iSize, iSocketHandle);
  fflush(stdout);
#endif

  while ( iSizeLeft > 0 && iNbrRead > 0) {
    iNbrRead = recv(iSocketHandle, &pBuf[iTotalRead], iSizeLeft, 0);
    iTotalRead += iNbrRead;
    iSizeLeft  -= iNbrRead;
  }

#ifdef SOCKTRACE
  CTrace::iTracef("SOCK:Read %d bytes\n", iTotalRead);
  CTrace::iTracef("%c", pBuf[0]);
  fflush(stdout);
#endif

  if (piBytesRead) {
    *piBytesRead = iTotalRead;
  }

  if ( iTotalRead < iSize ) {
    return (CSockio::READERR);
  }

  return (CSockio::OKAY);   
}
// ---------------------------------------------------------------------
int CSockio::iWriteSock(int iSocketHandle, void *pBuffer, int iSize, int *piBytesWritten)
{
  // Writes to socket until all bytes have been written Passes back number
  // of bytes written in *piBytesWritten Returns CSockio::OKAY, or
  // CSockio::READERR if error On error, socket should be closed by calling
  // functions

  int             iNbrWritten = 1;
  int             iSizeLeft;
  int             iTotalWritten = 0;
  char            *pBuf;
#ifdef PRE_READ_SOCK
  char            cTest;
#endif

#ifdef SOCKTRACE
  CTrace::iTracef("SOCK:iWriteSock\n");
  fflush(stdout);
#endif

#ifdef PRE_READ_SOCK
  while (recv(iSocketHandle, &cTest, 1)) {
#ifdef SOCKTRACE
    CTrace::iTracef("SOCK:found a %d:%ciWriteSock\n", (int)cTest, cTest);
    fflush(stdout);
#endif
  }
#endif // defined PRE_READ_SOCK

  if (piBytesWritten) {
    *piBytesWritten = 0;
  }

  if (pBuffer == NULL) {
#ifdef SOCKTRACE
    CTrace::iTracef("SOCK:Bad Parm\n");
    fflush(stdout);
#endif
    return (CSockio::BADPARM);
  }

#ifdef SOCKTRACE
  CTrace::iTracef("SOCK:Write -\n");
  fflush(stdout);
#endif

  iSizeLeft = iSize;

#ifdef SOCKTRACE
  CTrace::iTracef("SOCK:Write -\n");
  fflush(stdout);
#endif

  pBuf = (char*) pBuffer;

#ifdef SOCKTRACE
  CTrace::iTracef("SOCK:Writing %d bytes to %d\n", iSize, iSocketHandle);
  fflush(stdout);
#endif

  while ( iSizeLeft > 0 && iNbrWritten > 0) {
    iNbrWritten = send(iSocketHandle, &pBuf[iTotalWritten], iSizeLeft, 0);
    iTotalWritten += iNbrWritten;
    iSizeLeft  -= iNbrWritten;
  }

#ifdef SOCKTRACE
  CTrace::iTracef("SOCK:Wrote %d bytes\n", iTotalWritten);
  fflush(stdout);
#endif

  if (piBytesWritten) {
    *piBytesWritten = iTotalWritten;
  }

  if ( iTotalWritten < iSize ) {
    return (CSockio::WRITEERR);
  }

  return (CSockio::OKAY);
}

// ---------------------------------------------------------------------
int CSockio::iCloseSock(int iSockHandle, int iShut, int iLinger, int iTrace)
{
  // Close the socket Unless otherwise needed, linger should be set to 1
  // Returns 0 on success, CSockio::CLOSEERR on error or CSockio::BADSOCK on
  // invalid socket

  char   bByte=0;
  struct linger  rLinger;
  fd_set fds;
  struct timeval timeOut;
  timeOut.tv_sec = 0;
  timeOut.tv_usec = 50;

  FD_ZERO(&fds);

#ifdef SOCKTRACE
  CTrace::iTracef("SOCK:Close %d\n", iSockHandle);
  fflush(stdout);
#endif

  if (iSockHandle != -1) {
    if (iShut) {
      shutdown(iSockHandle, 1);
    }

    FD_SET(iSockHandle, &fds);

    while (select(iSockHandle, &fds, NULL, NULL, &timeOut) > 0 &&
      recv(iSockHandle, &bByte, 1, 0) == 1)
      {
      if (iTrace) {
        CTrace::iTracef("{%d:%c}", (int)bByte, bByte);
        fflush(stdout);
      }
      FD_SET(iSockHandle, &fds);
    }

    if (iLinger) {
      rLinger.l_onoff = 1;
      rLinger.l_linger = 50;

      setsockopt(iSockHandle, SOL_SOCKET, SO_LINGER,
        (char *)&rLinger, sizeof(rLinger));
    }

#ifdef UNIXWIN
    return closesocket(iSockHandle);
#else
    return close(iSockHandle);
#endif
  }
  else {
    return (CSockio::BADSOCK);
  }
}

// ---------------------------------------------------------------------
int CSockio::iOpenSock(int *piSockHandle, char *pszSocketAddr, int iSocketPort, int iTrace)
{
  // Open the socket. The pszSocketAddr should be a dotted quad address, and
  // not a host name. *piSocket will receive the new socket handle returns
  // CSockio::OK on success
  // CSockio::NOSOCK on socket call fail
  // CSockio::NOCONN on connect call fail

  struct sockaddr_in rAddr;
  int                iRet;
  int                iSockHandle;

  if (piSockHandle == NULL || pszSocketAddr == NULL) {
    return (CSockio::BADPARM);
  }

  *piSockHandle = -1;

  iSockHandle = (int)socket(AF_INET, SOCK_STREAM, 0);

  if (iSockHandle < 0) {
    return (CSockio::NOSOCK);
  }

  rAddr.sin_family = AF_INET;
  rAddr.sin_addr.s_addr = inet_addr(pszSocketAddr);
  rAddr.sin_port = iSocketPort;

  iRet = connect(iSockHandle, (struct sockaddr *)&rAddr, sizeof(rAddr));

  if (iRet == -1) {
    if (iTrace) {
        perror("Connect Failure, error:");
        CTrace::iTracef("Trying to open %s:%d", pszSocketAddr, iSocketPort);
    }

#ifdef UNIXWIN
    closesocket(iSockHandle);
#else
    close(iSockHandle);
#endif
    iSockHandle = -1;
    return (CSockio::NOCONN);
  }

#ifdef SOCKTRACE
  CTrace::iTracef("SOCK:Open (%d) to %s:%d\n", iSockHandle, pszSocketAddr, iSocketPort);
  fflush(stdout);
#endif

  *piSockHandle = iSockHandle;

  return (CSockio::OKAY);
}

// ---------------------------------------------------------------------
// CharBufNode Stuff
// ---------------------------------------------------------------------
CCharBufNode::CCharBufNode()
{
  buffer = new char[CHUNKSIZE];
  next=NULL;
  reset();
}

CCharBufNode::~CCharBufNode()
{
  delete buffer;
}

void CCharBufNode::reset()
{
  nextReadPos = nextWritePos = 0;
}

int CCharBufNode::allRead()
{
  return (nextReadPos == nextWritePos) ? 1 : 0;
}

int CCharBufNode::isFull()
{
  return (nextWritePos < CCharBufNode::CHUNKSIZE) ? 0 : 1;
}

char CCharBufNode::readch()
{
  return allRead() ? 0 : buffer[nextReadPos++];
}

void CCharBufNode::writech(char ch)
{
  if (isFull() == 0) {
    buffer[nextWritePos++] = ch;
  }
}

CCharBufNode *CCharBufNode::getNext()
{
  return next;
}

void CCharBufNode::setNext(CCharBufNode *newNext)
{
  next = newNext;
}

// ---------------------------------------------------------------------
// CharBuf Stuff
// ---------------------------------------------------------------------
CCharBuf::CCharBuf(void)
{
  // Create empty charbuf
  head = NULL;
  nextReadPos=0;
}

CCharBuf::~CCharBuf()
{
  // returns NULL
  while (head) head = DequeueHead();
}

// Privates
void CCharBuf::IncreaseBuf()
{
  CCharBufNode *cbn;
  CCharBufNode *cbn_temp;

  cbn = new CCharBufNode;

  if (head == NULL) {
    head = cbn;
  }
  else {
    cbn_temp = head;
    while (cbn_temp->getNext() != NULL) {
      cbn_temp = cbn_temp->getNext();
    }
    cbn_temp->setNext(cbn);
  }
}

CCharBufNode *CCharBuf::DequeueHead()
{
  // returns next node if any
  CCharBufNode *cbn = NULL;

  cbn = head->getNext();
  delete head;
  head = cbn;

  return cbn;
}

// Publics
int CCharBuf::hasWaiting()
{
  return (head && head->allRead() == 0) ? 1 : 0;
}

void CCharBuf::writeChar(char ch)
{
  CCharBufNode *cbn;

  if (head == NULL) {
    IncreaseBuf();
  }

  cbn = head;
  while (cbn->getNext()) {
    cbn = cbn->getNext();
  }

  if (cbn->isFull()) {
    IncreaseBuf();
    cbn = cbn->getNext();
  }

  cbn->writech(ch);
}

void CCharBuf::writesz(const char *szStr)
{
  if (szStr) {
    while (*szStr) {
      writeChar(*szStr);
      szStr++;
    }
  }
}

char CCharBuf::readChar()
{
  char ch = 0;

  if (head && head->allRead() == 0) {
    ch = head->readch();
    if (head->allRead()) {
      if (head->getNext()) {
        head = DequeueHead();
      }
      else {
        head->reset();
      }
    }
  }

  return ch;
}

// ---------------------------------------------------------------------
// ClientNode Stuff
// ---------------------------------------------------------------------
CClientNode::CClientNode(const char *szCharName, int iSocketHandle, CClientNode *newNext)
{
  bAuthorized = 0;
  bCmdMode = 0;
  bLocalEcho = 1;
  lastWriteError = 0;
  lastReadError = 0;
  closeMe = 0;
  readyToSend = 0;
  this->szCharName = new char[MAX_CHARNAMELEN];
  cmdBuf = new char[CMD_BUFSIZE];
  memset(cmdBuf, 0, CMD_BUFSIZE);
  cmdBufUsed=0;
  this->chanList=NULL;
  lastPingReponseTimeSecs = 0;
  lastPingSecs = time(NULL); // pretend we have already pinged.

  bTempWriteBlock = false;

  if (suiNextIDNum == 0) {
    suiNextIDNum = rand(); // rand sucks.
  }
  suiNextIDNum++;
  this->uiIDNum = suiNextIDNum;

  strncpy(this->szCharName, szCharName, MAX_CHARNAMELEN-1);

  next = newNext;
  this->iSocketHandle = iSocketHandle;
  inBuf = new CCharBuf();
  outBuf = new CCharBuf();
  lastChar = '\n'; // force name on next
}

CClientNode::~CClientNode()
{
  if (this->chanList) delete this->chanList;
  if (inBuf) delete inBuf;
  inBuf = NULL;
  if (outBuf) delete outBuf;
  outBuf = NULL;
}

// ---------------------------------------------------------------------
// Eqbcs Stuff
// ---------------------------------------------------------------------
CEqbcs::CEqbcs()
{
  amRunning = 0;
  clientList = NULL;
  listenBuf = NULL;
  iServerHandle = -1;
  iExitNow = 0;
  iSigHupCaught = 0;
  iPort = DEFAULT_PORT;
  iAddr=INADDR_ANY;
  bNetBotChanges = false;
  listenBufOn = true;
  LogFile=stdout;
}

CEqbcs::~CEqbcs()
{
  CClientNode *cn;

  for (cn = clientList; cn != NULL; cn = cn->next) {
    cn->closeMe = 1;
  }
  CloseAllSockets();
  CSockio::vShutdownSockets();
  CClientNode *cn_next=NULL;
  for (cn = clientList; cn != NULL; cn = cn_next) {
    cn_next = cn->next;
    delete cn;
  }
}

// ---------------------------------------------------------------------
// Initiliaze Networking and Bind To Port
// ---------------------------------------------------------------------
int CEqbcs::NET_initServer(int iPort, struct sockaddr_in *sockAddress)
{
  // return handle to server, or -1 on error
  int socketOpt = 1;
  int iHandle = 1;

  if (CSockio::iStartupSockets(EQBCS_TraceSockets) != 0) {
    perror("Failed to create winsock");
  }

  if ((iHandle = (int)socket(AF_INET,SOCK_STREAM,0))==0) {
    // if socket failed then display error and exit
    perror("Create master_socket");
    return -1;
  }

  // multi connections
  if (setsockopt(iHandle, SOL_SOCKET, SO_REUSEADDR,
    (char *)&socketOpt, sizeof(socketOpt))<0)
    {
    CSockio::iCloseSock(iServerHandle, 1, 1, EQBCS_TraceSockets);
    perror("setsockopt");
    return -1;
  }

  sockAddress->sin_family = AF_INET;
  sockAddress->sin_addr.s_addr = iAddr;
  sockAddress->sin_port = htons((unsigned short)iPort);

  if (bind(iHandle, (struct sockaddr *)sockAddress,
    sizeof(struct sockaddr_in))<0)
    {
    // if bind failed then display error message and exit
     perror("bind");
  }

  // backlog of 1 - Keep it light
  if (listen(iHandle, 1)<0) {
    perror("listen");
  }

  return iHandle;
}

// ---------------------------------------------------------------------
// Count the clients (Active and Inactive)
// ---------------------------------------------------------------------
int CEqbcs::countClients(void)
{
  int count = 0;

  for (CClientNode *cn=clientList; cn != NULL; cn = cn->next) {
    count++;
  }

  return count;
}

// ---------------------------------------------------------------------
// Get max file descriptor (for select in win32)
// ---------------------------------------------------------------------
int CEqbcs::getMaxFD()
{
  int max = iServerHandle;

  for (CClientNode *cn=clientList; cn != NULL; cn = cn->next) {
    if (cn->closeMe == 0) {
      max = (cn->iSocketHandle > max) ? cn->iSocketHandle : max;
    }
  }

  return (max < 0) ? 1 : max+1;
}


// ---------------------------------------------------------------------
// Write Local Char - one char to local buffer
// ---------------------------------------------------------------------

void CEqbcs::WriteLocalChar(char ch)
{
  if (listenBuf && listenBufOn) {
    listenBuf->writeChar(ch);
  }
}

// ---------------------------------------------------------------------
// Send String to local only
// ---------------------------------------------------------------------

void CEqbcs::WriteLocalString(const char *szStr)
{
  while (*szStr) {
    WriteLocalChar(*szStr);
    szStr++;
  }
}

// ---------------------------------------------------------------------
// Send char to all clients
// ---------------------------------------------------------------------

void CEqbcs::AppendCharToAll(char ch)
{
  if (listenBuf && listenBufOn) listenBuf->writeChar(ch);

  for (CClientNode *cn=clientList; cn != NULL; cn = cn->next) {
    if (cn->bAuthorized && cn->closeMe==0 && cn->iSocketHandle>=0
      && cn->bTempWriteBlock==false)
      {
      cn->outBuf->writeChar(ch);
    }
  }
}

// ---------------------------------------------------------------------
// Write To All
// ---------------------------------------------------------------------
void CEqbcs::SendToAll(const char *szStr)
{
  if (szStr) {
    while (*szStr) {
      AppendCharToAll(*szStr);
      szStr++;
    }
  }
}

// ---------------------------------------------------------------------
// Write sender name to each
// ---------------------------------------------------------------------
void CEqbcs::SendMyNameToAll(CClientNode *cn, int iMsgType)
{
  if (iMsgType == CClientNode::MSG_TYPE_NBMSG) {
    SendToAll("\tNBPKT:");
    SendToAll(cn->szCharName);
    SendToAll(":");
  }
  else {
    AppendCharToAll('<');
    SendToAll(cn->szCharName);
    AppendCharToAll('>');
    AppendCharToAll(' ');
  }
}

// ---------------------------------------------------------------------
// Write sender name to specific client
// ---------------------------------------------------------------------
void CEqbcs::SendMyNameToOne(CClientNode *cn, CClientNode *cn_to, int iMsgType)
{
  // iMsgType not used currently.  Included in definition in case it's
  // needed later
  if (cn_to->bAuthorized && cn_to->closeMe == 0 &&
    cn_to->iSocketHandle >= 0 && cn->bTempWriteBlock == false)
  {
     if(iMsgType == CClientNode::MSG_TYPE_BCI)
     {
        cn_to->outBuf->writeChar('{');
        cn_to->outBuf->writesz(cn->szCharName);
        cn_to->outBuf->writeChar('}');
        cn_to->outBuf->writeChar(' ');
        return;
     }
     cn_to->outBuf->writeChar('[');
     cn_to->outBuf->writesz(cn->szCharName);
     cn_to->outBuf->writeChar(']');
     cn_to->outBuf->writeChar(' ');

     WriteLocalChar('[');
     WriteLocalString(cn->szCharName);
     WriteLocalString("] to [");
     WriteLocalString(cn_to->szCharName);
     WriteLocalString("]: ");
  }
}

// ---------------------------------------------------------------------
// Write Own Name to Each
// ---------------------------------------------------------------------
void CEqbcs::WriteOwnNames(void)
{
  // Called only when msgall mode is on.
  WriteLocalString(" [*ALL*] ");
  for (CClientNode *cn=clientList; cn != NULL; cn = cn->next) {
    if (cn->bAuthorized && cn->closeMe == 0 &&
      cn->iSocketHandle >= 0 && cn->bTempWriteBlock == false)
      {
      cn->outBuf->writeChar(' ');
      cn->outBuf->writesz(cn->szCharName);
      cn->outBuf->writeChar(' ');
    }
  }
}

// ---------------------------------------------------------------------
// Send Net Bot Send List to this client
// ---------------------------------------------------------------------
void CEqbcs::SendNetBotSendList(CClientNode *cnSend)
{
  cnSend->outBuf->writesz("\tNBCLIENTLIST=");
  int iCount = 0;
  for (CClientNode *cn=clientList; cn != NULL; cn = cn->next) {
    if (cn->bAuthorized && cn->closeMe == 0 &&
      cn->iSocketHandle >= 0 && cn->bTempWriteBlock == false)
      {
      if (iCount++) cnSend->outBuf->writeChar(' ');
      cnSend->outBuf->writesz(cn->szCharName);
    }
  }
  cnSend->outBuf->writesz("\n");
}

// ---------------------------------------------------------------------
// Notify Net Bot Changes, if any
// ---------------------------------------------------------------------
void CEqbcs::NotifyNetBotChanges(void)
{
  if (bNetBotChanges) {
    for (CClientNode *cn=clientList; cn != NULL; cn = cn->next) {
      if (cn->bAuthorized && cn->closeMe == 0 &&
        cn->iSocketHandle >= 0 && cn->bTempWriteBlock == false)
        {
        SendNetBotSendList(cn);
      }
    }
    bNetBotChanges = false;
  }
}

// ---------------------------------------------------------------------
// Notify Net Bot Client Join
// ---------------------------------------------------------------------
void CEqbcs::NotifyClientJoin(char *szName)
{
  if (szName != NULL && *szName !=0)
  {
    for (CClientNode *cn=clientList; cn != NULL; cn = cn->next)
    {
      if (cn->bAuthorized && cn->closeMe == 0 && cn->iSocketHandle >= 0 && cn->bTempWriteBlock == false)
      {
        cn->outBuf->writesz("\tNBJOIN=");
        cn->outBuf->writesz(szName);
        cn->outBuf->writesz("\n");
      }
    }
  }
}

// ---------------------------------------------------------------------
// Notify Net Bot Client Quit
// ---------------------------------------------------------------------
void CEqbcs::NotifyClientQuit(char *szName)
{
  if (szName != NULL && *szName !=0)
  {
    for (CClientNode *cn=clientList; cn != NULL; cn = cn->next)
    {
      if (cn->bAuthorized && cn->closeMe == 0 && cn->iSocketHandle >= 0 && cn->bTempWriteBlock == false)
      {
        cn->outBuf->writesz("\tNBQUIT=");
        cn->outBuf->writesz(szName);
        cn->outBuf->writesz("\n");
      }
    }
  }
}

// ---------------------------------------------------------------------
// Add new incoming client
// ---------------------------------------------------------------------
void CEqbcs::HandleNewClient(struct sockaddr_in *sockAddress)
{
  // Open the new socket as 'new_socket'
  char buf[256];
  int iSocketHandle;
  int iBytesWrote;
  int addrlen=sizeof(*sockAddress);
  const char *loginName = "--LOGIN--";

  iSocketHandle = (int)accept((unsigned)iServerHandle, (struct sockaddr *)sockAddress, (socklen_t *)&addrlen);

  if (iSocketHandle < 0) {
    perror("Failed to connect new client - accept");
    return;
  }

  if (countClients() < MAX_CLIENTS) {
    sprintf((char *)buf, "-- Client connection: fd %d\n", iSocketHandle);
    WriteLocalString(buf);

    clientList = new CClientNode(loginName, iSocketHandle, clientList);
  }
  else {
    WriteLocalString("-- Incoming client rejected -- too many connections\n");
    sprintf(buf, (char *)"Denied - too many connections");
    CSockio::iWriteSock(iSocketHandle, buf, (int)strlen(buf), &iBytesWrote);
    CSockio::iCloseSock(iSocketHandle, 1, 1, EQBCS_TraceSockets);
  }
}

// ---------------------------------------------------------------------
// Update channel list
// ---------------------------------------------------------------------
void CEqbcs::HandleUpdateChannels(CClientNode *cn)
{
  char szTemp[2048]={0};
  int i=0;

  if (cn->chanList!=NULL) delete cn->chanList;
  while (cn->inBuf->hasWaiting()) szTemp[i++]=cn->inBuf->readChar();
  szTemp[i]=0;
  cn->chanList=new char[strlen(szTemp)+1];
  strcpy(cn->chanList,szTemp);
  sprintf(szTemp, "%s joined channels %s.\n", cn->szCharName, cn->chanList);
  cn->outBuf->writesz(szTemp);
  WriteLocalString(szTemp);
}

// ---------------------------------------------------------------------
// Process Tells
// ---------------------------------------------------------------------
void CEqbcs::HandleTell(CClientNode *cn)
{
  char szName[CClientNode::MAX_CHARNAMELEN];
  char szMsg[2048]={0};
  char szTemp[2048];
  char *token;
  char ch;
  int i=0;
  CClientNode *cn_to=clientList;

  ch=cn->inBuf->readChar();
  while (ch!=' ' && ch!='\n' && ch!='\0' && i<CClientNode::MAX_CHARNAMELEN-1 && cn->inBuf->hasWaiting()) {
    szName[i++]=ch;
    ch=cn->inBuf->readChar();
  }
  szName[i]='\0';

  i=0;
  while (cn->inBuf->hasWaiting()) {
    ch=cn->inBuf->readChar();
    if (ch=='\\' && cn->inBuf->hasWaiting()) ch=cn->inBuf->readChar();
    szMsg[i++]=ch;
    }
  szMsg[i++]='\n';
  szMsg[i]='\0';

  while (cn_to!=NULL && strcasecmp(cn_to->szCharName, szName)!=0)
    cn_to=cn_to->next;

  if (cn_to!=NULL) {
    SendMyNameToOne(cn, cn_to, CClientNode::MSG_TYPE_TELL);
    cn_to->outBuf->writesz(szMsg);
    WriteLocalString(szMsg);
    return;
  } else {
    i=0;
    for (cn_to=clientList; cn_to!=NULL; cn_to=cn_to->next) {
      if((cn->bLocalEcho || cn_to!=cn) && cn_to->chanList!=NULL) {
        strncpy(szTemp,cn_to->chanList,2048);
        token=strtok(szTemp," \n");
        while (token!=NULL) {
          if (strcmp(token,szName)==0) {
            WriteLocalString(szName);
            WriteLocalString(": ");
            SendMyNameToOne(cn, cn_to, CClientNode::MSG_TYPE_TELL);
            cn_to->outBuf->writesz(szMsg);
            WriteLocalString(szMsg);
            i=1;
            break;
          } else
            token=strtok(NULL," \n");
        }
      }
    }
  }
  if (i==0) {
      cn->outBuf->writesz("-- ");
      cn->outBuf->writesz(szName);
      cn->outBuf->writesz(": No such name.\n");
      while (cn->inBuf->hasWaiting()) ch=cn->inBuf->readChar();
  }
}

// copy/paste of HandleTell. works for the time being -.-
void CEqbcs::HandleBciMessage(CClientNode *cn)
{
   char szName[CClientNode::MAX_CHARNAMELEN];
  char szMsg[2048]={0};
  char szTemp[2048];
  char *token;
  char ch;
  int i=0;
  CClientNode *cn_to=clientList;

  ch=cn->inBuf->readChar();
  while (ch!=' ' && ch!='\n' && ch!='\0' && i<CClientNode::MAX_CHARNAMELEN-1 && cn->inBuf->hasWaiting()) {
    szName[i++]=ch;
    ch=cn->inBuf->readChar();
  }
  szName[i]='\0';

  i=0;
  while (cn->inBuf->hasWaiting()) {
    ch=cn->inBuf->readChar();
    if (ch=='\\' && cn->inBuf->hasWaiting()) ch=cn->inBuf->readChar();
    szMsg[i++]=ch;
    }
  szMsg[i++]='\n';
  szMsg[i]='\0';

  while (cn_to!=NULL && strcasecmp(cn_to->szCharName, szName)!=0)
    cn_to=cn_to->next;

  if (cn_to!=NULL) {
    SendMyNameToOne(cn, cn_to, CClientNode::MSG_TYPE_BCI);
    cn_to->outBuf->writesz(szMsg);
    return;
  } else {
    i=0;
    for (cn_to=clientList; cn_to!=NULL; cn_to=cn_to->next) {
      if((cn->bLocalEcho || cn_to!=cn) && cn_to->chanList!=NULL) {
        strncpy(szTemp,cn_to->chanList,2048);
        token=strtok(szTemp," \n");
        while (token!=NULL) {
          if (strcmp(token,szName)==0) {
            WriteLocalString(szName);
            WriteLocalString(": ");
            SendMyNameToOne(cn, cn_to, CClientNode::MSG_TYPE_BCI);
            cn_to->outBuf->writesz(szMsg);
            WriteLocalString(szMsg);
            i=1;
            break;
          } else
            token=strtok(NULL," \n");
        }
      }
    }
  }
  if (i==0) {
      cn->outBuf->writesz("-- ");
      cn->outBuf->writesz(szName);
      cn->outBuf->writesz(": No such name.\n");
      while (cn->inBuf->hasWaiting()) ch=cn->inBuf->readChar();
  }
}

// ---------------------------------------------------------------------
// Disconnect command
// ---------------------------------------------------------------------
void CEqbcs::CmdDisconnect(CClientNode *cn)
{
  if (cn) {
    WriteLocalString("-- ");
    WriteLocalString(cn->szCharName);
    WriteLocalString(" CmdDisconnect.\n");
    cn->closeMe = 1;
  }
}

// ---------------------------------------------------------------------
// Send Names command
// ---------------------------------------------------------------------
void CEqbcs::CmdSendNames(CClientNode *cn_to)
{
  int count = 0;

  cn_to->outBuf->writesz("-- Names:");
  WriteLocalString("-- ");
  WriteLocalString(cn_to->szCharName);
  WriteLocalString(" Requested Names:");

  for (CClientNode *cn=clientList; cn != NULL; cn = cn->next) {
    if (cn->bAuthorized && cn->closeMe == 0 && cn->iSocketHandle >= 0) {
      count++;
      cn_to->outBuf->writeChar(' ');
      cn_to->outBuf->writesz(cn->szCharName);
      WriteLocalString(" ");
      WriteLocalString(cn->szCharName);
    }
  }
  cn_to->outBuf->writesz(".\n");
  WriteLocalString(".\n");
}

// ---------------------------------------------------------------------
// Do a command
// ---------------------------------------------------------------------
void CEqbcs::DoCommand(CClientNode *cn)
{
  // Note: All found commands should process and return immediately.
  if (cn) {
    cn->bCmdMode = false;
    cn->cmdBufUsed = 0;
    if (cn->cmdBuf) {
      if (strcmp("NBMSG", cn->cmdBuf)==0) {
        cn->inBuf->writeChar('\t');
        cn->inBuf->writeChar((char)CClientNode::MSG_TYPE_NBMSG);
        return;
      }
      if (!strcmp("BCI", cn->cmdBuf))
      {
         cn->inBuf->writeChar('\t');
         cn->inBuf->writeChar((char)CClientNode::MSG_TYPE_BCI);
         return;
      }
      if (strcmp("NBNAMES", cn->cmdBuf)==0) {
        SendNetBotSendList(cn);
        return;
      }
      if (strcmp("NAMES", cn->cmdBuf) == 0) {
        CmdSendNames(cn);
        return;
      }
      if (strcmp("DISCONNECT", cn->cmdBuf) == 0) {
        CmdDisconnect(cn);
        return;
      }
      if (strcmp("MSGALL", cn->cmdBuf) == 0) {
        cn->inBuf->writeChar('\t');
        cn->inBuf->writeChar((char)CClientNode::MSG_TYPE_MSGALL);
        return;
      }
      if (strcmp("TELL", cn->cmdBuf) == 0) {
        cn->inBuf->writeChar('\t');
        cn->inBuf->writeChar((char)CClientNode::MSG_TYPE_TELL);
        return;
      }
      if (strcmp("CHANNELS", cn->cmdBuf) == 0) {
        cn->inBuf->writeChar('\t');
        cn->inBuf->writeChar((char)CClientNode::MSG_TYPE_CHANNELS);
        return;
      }
      if (strncmp("LOCALECHO", cn->cmdBuf,9) == 0) {
        (cn->cmdBuf[10]=='1') ? cn->bLocalEcho=1 : cn->bLocalEcho=0;
        cn->outBuf->writesz("-- Local Echo: ");
        (cn->bLocalEcho) ? cn->outBuf->writesz("ON\n") : cn->outBuf->writesz("OFF\n");
        return;
      }
      if ( strcmp( "PONG", cn->cmdBuf ) == 0)
      {
         cn->lastPingReponseTimeSecs = time( NULL );
         return;
      }
    }
  }

  cn->outBuf->writesz("-- Unknown Command: ");
  if (cn->cmdBuf) cn->outBuf->writesz(cn->cmdBuf);
  cn->outBuf->writesz(".\n");
}

void CEqbcs::PingAllClients( time_t curTime )
{
   for ( CClientNode *cn = clientList; cn != NULL; cn = cn->next )
   {
      if ( cn->lastPingSecs + cn->PING_SECONDS < curTime )
      {
         cn->outBuf->writesz( "\tPING\n" );
         cn->lastPingSecs = curTime;
      }
   }

}

// ---------------------------------------------------------------------
// Read All Clients that (might) have pending data
// ---------------------------------------------------------------------
void CEqbcs::ReadAllClients(fd_set *fds)
{
  char ch;
  int iBytesRead;
  int lastRet = CSockio::OKAY;

#ifdef UNIXWIN
  WSASetLastError(0);
#endif

  for (CClientNode *cn=clientList; cn != NULL; cn = cn->next)    {
    if (FD_ISSET(cn->iSocketHandle, fds)) {
      if ((lastRet = CSockio::iReadSock(cn->iSocketHandle,
        &ch, 1, &iBytesRead)) == CSockio::OKAY && iBytesRead)
        {
        if (iBytesRead < 0) {
          cn->lastReadError = iBytesRead;
          cn->closeMe = 1;
        }
        else if (cn->bAuthorized && cn->bCmdMode == false) {
          if (ch == '\t' && cn->inBuf->hasWaiting() == 0) {
            cn->bCmdMode = true;
          }
          else if (ch == '\n') {
            cn->readyToSend = 1;
            cn->lastChar = ' '; // force to no spaces at start of next line
          }
          else if (cn->lastChar != ' ' || ch != ' ') {
            cn->inBuf->writeChar(ch);
            cn->lastChar = ch;
          }
        }
        else if (cn->cmdBufUsed < (CClientNode::CMD_BUFSIZE-1)) {
          if (ch == '\n' && cn->bCmdMode) {
            cn->cmdBuf[cn->cmdBufUsed] = 0;
            DoCommand(cn);
            cn->lastChar = ' ';
          }
          else if (ch != '\r') {
            cn->cmdBuf[cn->cmdBufUsed] = ch;
            cn->cmdBufUsed++;
          }
        }
      }
      else {
#ifdef UNIXWIN
        if (lastRet != CSockio::OKAY || WSAGetLastError()) {
          if (lastRet == -2) {
            cn->lastReadError = -1;
          }
          else {
            CSockio::vPrintSockErr();
            cn->lastReadError = WSAGetLastError();
          }
          cn->closeMe = 1;
          WSASetLastError(0);
        }
#else
        if (lastRet == CSockio::READERR) {
          cn->lastReadError = 1;
          cn->closeMe = 1;
        }
#endif
      }
    }
  }
}

// ---------------------------------------------------------------------
// Clean dead clients
// ---------------------------------------------------------------------
void CEqbcs::CleanDeadClients(void)
{
  CClientNode *cn = clientList;
  CClientNode *cn_last = NULL;
  CClientNode *cn_temp = NULL;

  while (cn != NULL) {
    if (cn->iSocketHandle == -1 && cn->closeMe == 1) {
      if (cn_last == NULL) // It's the head.
        {
        clientList = clientList->next;
        delete cn;
        cn = clientList;
      }
      else {
        cn_temp = cn;
        cn_last->next = cn->next;
        cn = cn->next;
        delete cn_temp;
      }
    }
    else {
      cn_last = cn;
      cn = cn->next;
    }
  }
}

// ---------------------------------------------------------------------
// Close socket handles for dead clients
// ---------------------------------------------------------------------
void CEqbcs::CloseDeadClients(void)
{
  for (CClientNode *cn=clientList; cn != NULL; cn = cn->next) {
    if (cn->iSocketHandle != -1 && cn->closeMe == 1) {
      CSockio::iCloseSock(cn->iSocketHandle, 1, 1, EQBCS_TraceSockets);
      NotifyClientQuit(cn->szCharName);
      WriteLocalString("-- ");
      WriteLocalString(cn->szCharName);
      WriteLocalString(" has left the server.\n");
      cn->iSocketHandle = -1;
      bNetBotChanges = true;
    }
  }
}

// ---------------------------------------------------------------------
// Close all sockets - call before exit
// ---------------------------------------------------------------------
void CEqbcs::CloseAllSockets()
{
  if (iServerHandle != -1) {
    CSockio::iCloseSock(iServerHandle, 1, 1, EQBCS_TraceSockets);
    iServerHandle = -1;
  }

  for (CClientNode *cn=clientList; cn != NULL; cn = cn->next) {
    if (cn->iSocketHandle != -1) {
      CSockio::iCloseSock(cn->iSocketHandle, 1, 1, EQBCS_TraceSockets);
      cn->iSocketHandle = -1;
    }
  }
}

// ---------------------------------------------------------------------
// Grab the data for the people we are ready to send from, and queue it up
// ---------------------------------------------------------------------
void CEqbcs::HandleReadyToSend(void)
{
  // MsgTypes are handled by inserting \t<msgtype> into the output buffer
  // before the string read from the socket.  These MsgTypes are only
  // inserted when there is a message type other than MSG_TYPE_NORMAL.

  int iMsgType=0;
  int ch;

  for (CClientNode *cn=clientList; cn != NULL; cn = cn->next) {
    if (cn->readyToSend && cn->iSocketHandle != -1 && cn->closeMe == 0) {
      if (cn->inBuf->hasWaiting()) {
        ch = cn->inBuf->readChar();
        if (ch == '\t') // check for msgtype
          {
            iMsgType = cn->inBuf->readChar();
            if (iMsgType!=CClientNode::MSG_TYPE_TELL && iMsgType!=CClientNode::MSG_TYPE_BCI &&
              iMsgType!=CClientNode::MSG_TYPE_CHANNELS) ch=cn->inBuf->readChar();
          }
          else {
            iMsgType = CClientNode::MSG_TYPE_NORMAL;
          }
          if (iMsgType == CClientNode::MSG_TYPE_MSGALL) {
            cn->bTempWriteBlock = true;
          }
          if (iMsgType == CClientNode::MSG_TYPE_BCI) {
            HandleBciMessage(cn);
            cn->readyToSend = 0;
            cn->bTempWriteBlock = false;
            listenBufOn = true;
            return;
          }
          if (iMsgType == CClientNode::MSG_TYPE_TELL) {
            HandleTell(cn);
            cn->readyToSend = 0;
            cn->bTempWriteBlock = false;
            listenBufOn = true;
            return;
          }
          if (iMsgType == CClientNode::MSG_TYPE_CHANNELS) {
            HandleUpdateChannels(cn);
            cn->readyToSend = 0;
            cn->bTempWriteBlock = false;
            listenBufOn = true;
            return;
          }
          // Turn off local display if it is NBMSG.
          if (iMsgType == CClientNode::MSG_TYPE_NBMSG) {
            listenBufOn = false;
          }
          SendMyNameToAll(cn, iMsgType);
          if (iMsgType == CClientNode::MSG_TYPE_MSGALL) {
            WriteOwnNames();
          }
          AppendCharToAll(ch);
          while (cn->inBuf->hasWaiting()) {
            AppendCharToAll(cn->inBuf->readChar());
          }
          AppendCharToAll('\n');
      }
      cn->readyToSend = 0;
      cn->bTempWriteBlock = false;
      listenBufOn = true;
    }
  }
}

// ---------------------------------------------------------------------
// Kick off same name - when authorized comes, remove any other with
// same name */
// ---------------------------------------------------------------------
void CEqbcs::KickOffSameName(CClientNode *cnCheck)
{
  for (CClientNode *cn=clientList; cn != NULL; cn = cn->next)  {
    if (cn != cnCheck && strcmp(cn->szCharName, cnCheck->szCharName) == 0) {
      cn->closeMe = true;
      WriteLocalString("-- Kicking off connection the same as: ");
      WriteLocalString(cn->szCharName);
      WriteLocalString(".\n");
      if (strlen(cn->szCharName) < CClientNode::MAX_CHARNAMELEN-5) {
        strcat(cn->szCharName, "-old");
      }
    }
  }
}

// ---------------------------------------------------------------------
// Authorize Clients
// ---------------------------------------------------------------------
void CEqbcs::AuthorizeClients(void)
{
  static const char *loginTest = LOGIN_START_TOKEN;
  char *p;
  int copied=0;

  for (CClientNode *cn=clientList; cn != NULL; cn = cn->next) {
    if (cn->bAuthorized==0 && (unsigned)cn->cmdBufUsed>strlen(loginTest) &&
      strrchr(&cn->cmdBuf[strlen(loginTest)+1], ';'))
      {
      for (p = &cn->cmdBuf[strlen(loginTest)];
        *p != ';' && copied < CClientNode::MAX_CHARNAMELEN-1; p++)
        {
        cn->szCharName[copied] = *p;
        copied++;
      }
      cn->szCharName[copied] = 0;
      cn->bAuthorized = 1;
      cn->cmdBufUsed=0;
      NotifyClientJoin(cn->szCharName);
      WriteLocalString("-- ");
      WriteLocalString(cn->szCharName);
      WriteLocalString(" has joined the server.\n");
      bNetBotChanges = true;
      KickOffSameName(cn);
    }
  }
}

// ---------------------------------------------------------------------
// Handle flushing local output
// ---------------------------------------------------------------------
void CEqbcs::HandleLocal()
{
  if (listenBuf) {
    while (listenBuf->hasWaiting()) {
			fprintf(LogFile, "%c", listenBuf->readChar());
    }
		fflush(LogFile);
	}
	// Here, add remote handlers, callbacks, etc.
}

// ---------------------------------------------------------------------
// Check Clients: login or write pending or  remove dead connections
// ---------------------------------------------------------------------
int CEqbcs::CheckClients(void)
{
  char writeBuf[512];
  int bufUsed;
  int maxBuf = sizeof(writeBuf);
  int iRetCode = 0;
  int iBytesWrote = 0;

  AuthorizeClients();
  CloseDeadClients();
  CleanDeadClients();
  HandleReadyToSend();
  HandleLocal();
  NotifyNetBotChanges();

#ifdef UNIXWIN
  WSASetLastError(0);
#endif

  if (listenBuf) {
    while (listenBuf->hasWaiting()) {
      iRetCode = 1;
      WriteLocalChar(listenBuf->readChar());
    }
  }
  for (CClientNode *cn=clientList; cn != NULL; cn = cn->next) {
    while (cn->outBuf->hasWaiting() && cn->lastWriteError == 0) {
      iRetCode = 1; // Any written to will be 1;
      for (bufUsed=0; bufUsed<maxBuf && cn->outBuf->hasWaiting(); bufUsed++) {
        writeBuf[bufUsed] = cn->outBuf->readChar();
      }
      if (bufUsed > 0) {
        CSockio::iWriteSock(cn->iSocketHandle, writeBuf, bufUsed, &iBytesWrote);
      }
#ifdef UNIXWIN
      if (iBytesWrote < 1 && WSAGetLastError()) {
        cn->closeMe = 1;
        cn->lastWriteError = WSAGetLastError();
      }
      WSASetLastError(0);
#else
      if (iBytesWrote < 1) {
        cn->closeMe = 1;
        cn->lastWriteError = iBytesWrote;
      }
#endif
    }
  }
  return iRetCode;
}

// ---------------------------------------------------------------------
// Setup which sockets to listen on
// ---------------------------------------------------------------------
void CEqbcs::SetupSelect(fd_set *fds)
{
  FD_ZERO(fds);

  // setup which sockets to listen on
  FD_SET((unsigned)iServerHandle, fds);

  for (CClientNode *cn=clientList; cn != NULL; cn = cn->next) {
    if (cn->iSocketHandle != -1 && cn->closeMe != 1) {
      FD_SET((unsigned)cn->iSocketHandle, fds);
    }
  }
}

// ---------------------------------------------------------------------
// Print Welcome
// ---------------------------------------------------------------------
void CEqbcs::PrintWelcome(void)
{
  char szPort[10];
  sprintf(szPort, "%d", iPort);

  WriteLocalString(Title);
  WriteLocalString(" ");
  WriteLocalString(Version);
  WriteLocalString("\nWaiting for connections on port: ");
  WriteLocalString(szPort);
  WriteLocalString("...\n");
}

// ---------------------------------------------------------------------
// Never ending looping
// ---------------------------------------------------------------------
void CEqbcs::ProcessLoop(struct sockaddr_in *sockAddress)
{
  int iPending;
  //int iExtraHandles=5;
  // Extra handles for select (STDIN, OUT, ERR..)
  // Not supposed to matter, but it has before
  fd_set fds;
  fd_set empty_fds1;
  fd_set empty_fds2;
  // Not worrying about FD_SETSIZE - if too small, then fix/recompile
  struct timeval timeOut;
  int selectMax;

  FD_ZERO(&empty_fds1);
  FD_ZERO(&empty_fds2);

  PrintWelcome();

  while (iExitNow == 0) {
    CheckClients();
    SetupSelect(&fds);

#ifdef UNIXWIN
    selectMax = getMaxFD();
#else
    selectMax = getdtablesize();
#endif

    try {
      timeOut.tv_sec = 5;
      timeOut.tv_usec = 50;
      iPending=select(selectMax, &fds, &empty_fds1, &empty_fds2, &timeOut);
    }
    catch(char * str) {
      CTrace::dbg("Exception: %s", str);
    }

    if ((iPending<0) && (errno!=EINTR)) { // there was an error with select()
#ifdef UNIXWIN
      CSockio::vPrintSockErr();
      WSASetLastError(0);
#else
      perror("select() error");
#endif
    }
    if (iPending > 0 && iExitNow == 0) {
      if (FD_ISSET(iServerHandle, &fds)) {
        HandleNewClient(sockAddress);
      }
      ReadAllClients(&fds);
    }
   PingAllClients( time( NULL ) );
  }
  CloseAllSockets();
  CSockio::vShutdownSockets();
}

// ---------------------------------------------------------------------
// Signal Support
// ---------------------------------------------------------------------
void CEqbcs::vCtrlCHandler(int iValue)
{
  // CtrlCHandler
  CTrace::iTracef("Got Ctrl-C (%d): Exiting", iValue);
  if (EQBCS_iDebugMode) fflush(stdout);
  if (runInstance) runInstance->setExitFlag();
}

void CEqbcs::vBrokenHandler(int iValue)
{
#ifndef UNIXWIN
  signal(SIGPIPE, vBrokenHandler);
#endif
}

// ---------------------------------------------------------------------
void CEqbcs::setExitFlag()
{
  iExitNow = 1;
}

// ---------------------------------------------------------------------
// Port setup (call before processMain)
// ---------------------------------------------------------------------
void CEqbcs::setPort(int newPort)
{
  this->iPort = newPort;
}

// ---------------------------------------------------------------------
// Bind to address setup
// ---------------------------------------------------------------------
in_addr_t CEqbcs::setAddr(const char* newAddr)
{
  this->iAddr = inet_addr(newAddr);
  return(this->iAddr);
}

// ---------------------------------------------------------------------
// Logfile setup
// ---------------------------------------------------------------------
int CEqbcs::setLogfile(const char* szLogfile)
{
  if((this->LogFile=fopen(szLogfile,"a"))==NULL) {
    fprintf(stderr, "ERROR: Could not open file %s for write.\n\n", szLogfile);
    return(1);
  }
  return(0);
}
// ---------------------------------------------------------------------
// Process Main - For UI Threading - publicly accessible
// ---------------------------------------------------------------------
int CEqbcs::processMain(int exitOnFail)
{
  struct sockaddr_in sockAddress;

  runInstance = this;
  signal(SIGINT, vCtrlCHandler);
#ifndef UNIXWIN
  signal(SIGPIPE, vBrokenHandler);
  srandom(time(NULL));
#endif

  listenBuf = new CCharBuf();
  clientList = NULL;

  amRunning = 1;

  if ((iServerHandle = NET_initServer(iPort, &sockAddress)) == -1) {
    if (exitOnFail) {
      exit(EXIT_FAILURE);
    }
  }
  else {
    ProcessLoop(&sockAddress);
  }

  if (LogFile!=stdout) fclose(LogFile);
  amRunning = 0;
  return 0;
}
