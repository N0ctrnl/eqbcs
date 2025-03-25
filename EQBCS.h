// Module Name: EQBCS.h
// Purpose    : EQ Box Chat. Private Chat Relay server for EQ bots.
// Author     : ascii38
// Date       : 20101218
//
// Based on original EQBC by Omnictrl//

#ifndef __EQBCS2_H__
#define __EQBCS2_H__

#define PROG_NAME "EQBCS2"
#define PROG_TITLE "EQ Box Chat Server"
#define PROG_VERSION "10.12.18"

// Change the following line if you want to have
// a tad more security (for example, change to "LOGIN:PASSWORD=")
// Just be sure to change on the client side as well.

#define LOGIN_START_TOKEN "LOGIN="

// Make VC++ 2005 STFU about insecure string functions
#if defined(_MSC_VER) && _MSC_VER >=1400
#define _CRT_SECURE_NO_DEPRECATE 1
#endif

// Windows specific includes and defines
#ifdef _WIN32
#define UNIXWIN
#define WIN32_LEAN_AND_MEAN

#ifdef _UNICODE
#error Set Character Set to "Not Set" in Project Properties.
#endif

#define socklen_t int
#define strcasecmp _stricmp
#pragma comment(lib,"wsock32.lib")
#include <windows.h>
#include <winsvc.h>
#include <tchar.h>
#include <winsock.h>
#define WS_MAJOR  1
#define WS_MINOR  1
typedef unsigned long in_addr_t;
#endif


// Unix specific includes
#ifndef UNIXWIN
#include <stdarg.h>
#include <unistd.h>
#ifndef __USE_XOPEN
  #define __USE_XOPEN // Very important in RH 5.2 - gets proper signal support
#endif
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#endif

#if defined (_SC_LOGIN_NAME_MAX) && !defined (LOGIN_NAME_MAX) && !defined (UNIXWIN)
  #define LOGIN_NAME_MAX _SC_LOGIN_NAME_MAX
#endif

// General (non-OS specific) includes
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

// Windows service crap
#ifdef UNIXWIN
void WINAPI ServiceMain(DWORD argc, LPTSTR *argv);
void WINAPI ServiceCtrlHandler(DWORD Opcode);
BOOL DoInstallService(char* lpServiceName, char* lpDisplayName, int c, char* v[]);
BOOL DoDeleteService(char* lpServiceName);
BOOL DoStartService(char* lpServiceName);
BOOL DoStopService(char* lpServiceName);
#endif

// ---------------------------------------------------------------------
// Classdecs */
// ---------------------------------------------------------------------

class CTrace
{
public:
  static int iTracef(const char *fmt,...);
  static int dbg(const char *fmt,...);
};

class CCharBufNode
{
private:
  CCharBufNode *next;
  char *buffer;
  int nextWritePos;
  int nextReadPos;
public:
  static const int CHUNKSIZE;
public:
  CCharBufNode();
  ~CCharBufNode();
  void reset();
  int isFull();
  int allRead();
  char readch();
  void writech(char ch);
  CCharBufNode *getNext();
  void setNext(CCharBufNode *newNext);
};

class CCharBuf
{
private:
  CCharBufNode *head;
  int nextReadPos;
private: // Internal
  void IncreaseBuf();
  CCharBufNode *DequeueHead();
public:
  CCharBuf();
  ~CCharBuf();
  int hasWaiting();
  void writeChar(char ch);
  void writesz(const char *szStr);
//    char peekChar();
  char readChar();
};

class CClientNode
{
public: // Constants
  static const int MAX_CHARNAMELEN;
  static const int CMD_BUFSIZE;
  static const int PING_SECONDS;
  static const unsigned char MSG_TYPE_NORMAL;
  static const unsigned char MSG_TYPE_NBMSG;
  static const unsigned char MSG_TYPE_MSGALL;
  static const unsigned char MSG_TYPE_TELL;
  static const unsigned char MSG_TYPE_CHANNELS;
  static const unsigned char MSG_TYPE_BCI;
  static unsigned int suiNextIDNum;
public: // Vars
  int iSocketHandle;
  bool bAuthorized;
  int lastWriteError;
  int lastReadError;
  bool closeMe;
  int readyToSend;
  char lastChar;
  char *szCharName;
  char *cmdBuf;
  char *chanList;
  bool bLocalEcho;
  int cmdBufUsed;
  bool bCmdMode;
  unsigned uiIDNum;
  bool bTempWriteBlock;
  CCharBuf *outBuf;
  CCharBuf *inBuf;
  CClientNode *next;
  time_t lastPingSecs;
  int lastPingReponseTimeSecs;
public:
  CClientNode(const char *szCharName, int iSocketHandle, CClientNode *newNext);
  ~CClientNode();
};

class CSockio
{
public:
  static const int OKAY;
  static const int CLOSEERR;
  static const int READERR;
  static const int WRITEERR;
  static const int BADSOCK;
  static const int BADPARM;
  static const int NOSOCK;
  static const int NOCONN;

public:
  static void vPrintSockErr(void);
  static void vShutdownSockets(void);
  static int iStartupSockets(int iVerbose);
  static int iReadSock(int iSocketHandle, void *pBuffer, int iSize, int *piBytesRead);
  static int iWriteSock(int iSocketHandle, void *pBuffer, int iSize, int *piBytesWritten);
  static int iCloseSock(int iSockHandle, int iShut, int iLinger, int iTrace);
  static int iOpenSock(int *piSockHandle, char *pszSocketAddr, int iSocketPort, int iTrace);
};

class CEqbcs
{
private:
  static const int MAX_CLIENTS;
  static const int DEFAULT_PORT;

  bool listenBufOn;
  CCharBuf *listenBuf;
  CClientNode *clientList;
  int amRunning;
  int iServerHandle;
  int iExitNow;
  int iSigHupCaught;
  int iPort;
  in_addr_t iAddr;
  FILE *LogFile;
  bool bNetBotChanges;

private:
  int NET_initServer(int iPort, struct sockaddr_in *sockAddress);
  int countClients(void);
  int getMaxFD();
  void SendToLocal(char ch);
  void WriteLocalChar(char ch);
  void WriteLocalString(const char *szStr);
  void AppendCharToAll(char ch);
  void SendToAll(const char *szStr);
  void SendMyNameToAll(CClientNode *cn, int iMsgType);
  void SendMyNameToOne(CClientNode *cn, CClientNode *cn_to, int iMsgType);
  void WriteOwnNames(void);
  void HandleNewClient(struct sockaddr_in *sockAddress);
  void HandleUpdateChannels(CClientNode *cn);
  void HandleTell(CClientNode *cn);
  void CmdDisconnect(CClientNode *cn);
  void CmdSendNames(CClientNode *cn_to);
  void SendNetBotSendList(CClientNode *cnSend);
  void NotifyNetBotChanges();
  void DoCommand(CClientNode *cn);
  void ReadAllClients(fd_set *fds);
  void PingAllClients(time_t curTime);
  void CleanDeadClients(void);
  void CloseDeadClients(void);
  void CloseAllSockets();
  void HandleReadyToSend();
  void KickOffSameName(CClientNode *cnCheck);
  void AuthorizeClients();
  void HandleLocal();
  int CheckClients();
  void SetupSelect(fd_set *fds);
  void PrintWelcome();
  void ProcessLoop(struct sockaddr_in *sockAddress);
  void NotifyClientJoin(char *szName);
  void NotifyClientQuit(char *szName);
  void HandleBciMessage(CClientNode *cn);
public:
  CEqbcs();
  ~CEqbcs();
  int processMain(int exitOnFail);
  void setExitFlag();
  void setPort(int newPort);
  in_addr_t setAddr(const char* newAddr);
  int setLogfile(const char* szLogfile);
  static void vCtrlCHandler(int iValue);
  static void vBrokenHandler(int iValue);
};

extern CEqbcs bcs;

#endif // __EQBCS2_H__