//
// Filename : EQBCS.cpp
// Purpose  : EQ Box Chat. Private Chat Relay server for EQ bots.
// Author   : ascii38
// Version  : 2.0
// Date     : 20101201
//
// Based on original EQBC by Omnictrl

#include "EQBCS.h"

CEqbcs bcs;

int main(int argc, char* argv[])
{
#ifdef UNIXWIN
	int loadservice=0;
#else
	int pid;
	struct passwd *pw;
	char UserName[LOGIN_NAME_MAX+1]="\0";
#endif
	int giveusage=0;
	int dofork=0;
	int i;

	fflush(stdout);

	for (i=1; i<argc; i++) {
		if (strncmp(argv[i],"-?",2)==0) {
			giveusage=1;
		}
		if (strncmp("-p", argv[i],2)==0) {
      if (strchr("1234567890", *argv[++i])!=NULL)
        bcs.setPort(atoi((const char *)argv[i]));
      else {
        giveusage=1;
        i=argc+1;
      }
    }
		else if (strncmp("-i", argv[i],2)==0) {
      if (bcs.setAddr(argv[++i])==INADDR_NONE) {
        giveusage=1;
        i=argc+1;
      }
    }
    else if (strncmp("-l", argv[i],2)==0) {
      if (bcs.setLogfile(argv[++i])==1) {
        giveusage=1;
        i=argc+1;
      }
    }
#ifdef UNIXWIN
		else if(strncmp(argv[i],"-c",2)==0) {
      loadservice=1;
    }
    else if(strncmp(argv[i],"-d",2)==0)
    {
			if(DoDeleteService(PROG_NAME)) {
				printf("\nService UnInstalled Sucessfully\n");
				exit(0);
			}
			else {
				printf("\nError UnInstalling Service\n");
				exit(1);
			}
    }
		else if(strncmp(argv[i],"-SERVICE",8)==0) {
			dofork=1;
		}
#endif
#ifndef UNIXWIN
		else if(strncmp(argv[i],"-d",2)==0) {
			dofork=1;
		}
		else if (strncmp("-u",argv[i],2)==0) {
			if (argv[++i]) {
				strncpy(UserName, argv[i], LOGIN_NAME_MAX);
			}
			else {
				giveusage=1;
				i=argc+1;
			}
		}
#endif
		else {
			giveusage=1;
		}
	}

	if (giveusage==1) {
		fprintf(stderr, "Usage: "PROG_NAME" [options]\n");
		fprintf(stderr, "  Options are as follows:\n");
		fprintf(stderr, "  -?       \tDisplay this help message.\n");
		fprintf(stderr, "  -p <port>\tPort to listen on.\n");
		fprintf(stderr, "  -i <addr>\tAddress to bind to.\n");
		fprintf(stderr, "  -l <file>\tOutput to logfile rather than STDOUT.\n");
#ifdef UNIXWIN
		fprintf(stderr, "  -c       \tCreate Windows Service.\n");
		fprintf(stderr, "  -d       \tDelete Windows Service.\n");
#endif
#ifndef UNIXWIN
		fprintf(stderr, "  -d       \tRun as background daemon.\n");
		fprintf(stderr, "  -u       \tUsername to execute as.\n");
#endif
		fflush(stderr);
		exit(1);
	}

#ifdef UNIXWIN
	if (loadservice==1) {
		if(DoInstallService(PROG_NAME,PROG_TITLE,argc, argv)) {
			printf("\nService Installed Sucessfully\n");
			DoStartService(PROG_NAME);
			exit(0);
		}
		else {
			printf("\nError Installing Service\n");
			exit(1);
		}
	}
	if(dofork==1) {
		SERVICE_TABLE_ENTRY DispatchTable[]={{PROG_NAME,ServiceMain},{NULL,NULL}};
		StartServiceCtrlDispatcher(DispatchTable);
		exit(0);
	}
#else
	if(UserName[0]!='\0') {
		if ((pw=getpwnam(UserName))==NULL) {
			fprintf(stderr, "ERROR: Cannot find user %s.\n",UserName);
			fflush(stderr);
			exit(1);
		}
		else if ((setgid(pw->pw_gid))==-1 || (setuid(pw->pw_uid))==-1) {
			fprintf(stderr, "ERROR: SetUID to %s failed. Exiting.\n",UserName);
			fflush(stderr);
			exit(1);
		}
	}
	
	if (getuid()==0) {
		fprintf(stderr, "WARNING: Running as root NOT recommended.\n");
	}

  if (dofork==1) {
    pid=fork();
    if (pid < 0) {
      exit(1);
    }
    if (pid !=0) {
      exit(0);
    }
    setsid(); // make the process group & session leader and lose control TTY
    signal(SIGHUP, SIG_IGN);
    umask(0);

    pid=fork();
    if (pid < 0) {
      exit(1);
    }
    if (pid != 0) {
      exit(0);
    }
	}
#endif

  return bcs.processMain(1);
}