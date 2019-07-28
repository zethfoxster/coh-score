/***************************************************************************
 *     Copyright (c) 2008-2008, NCSoft
 *     All Rights Reserved
 *     Confidential Property of NCSoft
 *
 * Module Description:
 *  No stupid ftp client seemed to be able to grab the zip files from crash
 *  reports and delete them. so I made this.
 *
 ***************************************************************************/
#include "file.h"
#include "rsa.h"
#include "bignum.h"
#include "netio.h"
#include "estring.h"
#include "ftpclient.h"
#include "getopt.h"
#include "net_packet.h"
#include "net_masterlist.h"
#include "sock.h"
#include "memcheck.h"
#include "error.h"
#include <conio.h>
#include <process.h>
#include "timing.h"
#include "mathutil.h"
#include "netio_stats.h"
#include "utils.h"
#include "net_version.h"
#include "wininclude.h"
#include "osdependent.h"
#include "utils.h"
#include "assert.h"
#include "error.h"
#include "mathutil.h"
#include "earray.h"
#include "MemoryPool.h"
#include "StashTable.h"
#include "structNet.h"
#include "netio_core.h"
#include "net_linklist.h"
#include "net_link.h"

static BOOL g_verbose = FALSE;

typedef enum GrabFlags
{
    GrabFlags_None,
    GrabFlags_OverwriteExisting = 1<<0,    
    // resume, append, etc.
} GrabFlags;



BOOL FtpGrab(char *ipfrom, char *username, char *userpass, char *from_ftp_directory, char *extension_to_match, char *dirto, BOOL overwrite_existing)
{
    char *cursor;
    char *fn;
    char *ls_res = NULL;
    char pathbuf[MAX_PATH];
    FtpClient *c = ftpLogin(ipfrom, username, userpass);
    if( !c )
    {
        printf("login to %s for %s failed\n", ipfrom, username);
        return FALSE;
    }

#define CR(P) if(!P){ printf( #P " call failed. %s\n", c->reply); \
        FtpClient_Destroy(c);                                     \
        return 1;                                                 \
    }
    
    if(from_ftp_directory)
        CR(FtpClient_CD(c,from_ftp_directory));
    
    // get directory info
    sprintf(pathbuf,"*.%s", extension_to_match);
    CR(FtpClient_LS(c,pathbuf));
    pprintf(g_verbose,"ls returned \n%s\n", c->data);    
    
    estrCopy2(&ls_res,(char*)c->data);  
    cursor = ls_res;
    while(fn = strsep(&cursor,"\r\n"))
    {
        char dest_filename[MAX_PATH];
        FILE *fp;
        int n;
        
        if(*cursor == '\n')
            cursor++;
        
        sprintf(dest_filename,"%s/%s",dirto,fn);
        if(fileExists(dest_filename))
        {
            if(!overwrite_existing)
            {
                pprintf(g_verbose,"%s already exists on dest, skipping\n",dest_filename);
                continue;
            }
            else
            {
                pprintf(g_verbose,"%s exists, overwriting\n",dest_filename);
            }
        }
        
        pprintf(g_verbose,"grabbing %s\n",fn);
        
        if(!FtpClient_FileGet(c, fn))
        {
            printf("couldn't get file %s. skipping\n", fn);
            continue;
        }
        pprintf(g_verbose,"file %s has %i bytes\n", fn, c->datalen);
        
        fp = fopen(dest_filename,"wb");
        if(!fp)
        {
            printf("couldn't create %s for writing. skipping\n",dest_filename);
            continue;
        }
        
        n = fwrite(c->data, c->datalen, 1, fp);
        if(n != 1)
            printf("warning: wrote %i bytes, expected to write %i bytes\n",n,c->datalen);
        fclose(fp);
        
         if(!FtpClient_RM(c,fn))
         {
             printf("unable to delete %s. was written locally successfully\n",fn);
             continue;
         }
    }
    
    FtpClient_Destroy(c);

    return TRUE;
}



int main(int argc, char *argv[])
{
    BOOL overwrite_existing = FALSE;
    char *cursor;
    U32 time_last_run_started;
    BOOL continuous_checking = FALSE;
    BOOL unique_instance = FALSE;
    HANDLE instance_mutex = NULL;
    char console_status[1024];
    char *dirto = NULL;
    char *ipfrom = "errors.coh.com";
    char *userpass = "kicks" ;
    char *username = "fullerrors";
    char *from_ftp_directory = NULL; // "CityOfHeroes";
    char *extension_to_match = "*";

	int broadcastSize = 1024;
	extern int g_assert_on_netlink_overflow;
    int i = 0;
    
	memCheckInit();

	sprintf(console_status, "%d: %s", GetCurrentProcessId(), argv[0]);
	SetConsoleTitle(console_status);

	printf("\n\n");

	pktSetDebugInfo();
	bsAssertOnErrors(true);
	disableLogging(true);

    // test area  
//    printf("bignum had %i errors\n", BigNum_Test());
//    printf("rsaTest had %i errors\n",rsaTest()); 

    while(optind < argc)
    {
        int opt = getopt(argc,argv,"h?x:d:u:p:scov");
        if(opt == -1)
            break;
        switch ( opt )
        {
        case '?':
        case 'h':
            printf("usage: %s [opts] <from ip addr> <to local folder> "
                   "\n opts are:"
                   "\n[-x extensions to grab] "
                   "\n[-d ftp directory to grab from] -u user -p password "
                   "\n[-s only a single instance allowed to talk to this ip address "
                   "\n[-c run continuously: check every hour " 
                   "\n[-v verbose]\n", argv[0]);
            exit(0);
        break;
        case 'x':
            extension_to_match = strdup(optarg);
            break;
        case 'd':
            from_ftp_directory = strdup(optarg);
            break;
        case 'u':
            username = strdup(optarg);
            break;
        case 'p':
            userpass = strdup(optarg);
            break;
        case 's':
            unique_instance = TRUE;
            break;
        case 'c':
            continuous_checking = TRUE;
            break;
        case 'o':
            overwrite_existing = TRUE;
            break;
        case 'v':
            g_verbose = TRUE;
            break;
        default:
            Errorf("invalid switch value.");
            break;
        };
    }
    
    if(optind < argc)
        ipfrom = argv[optind++];
    if(optind < argc)
        dirto = argv[optind++];
#define TESTP(P) if(!P){ printf("must specify param " #P " \n");    \
        return 0;                                                   \
    }
    
    TESTP(username);
    TESTP(userpass);
    TESTP(ipfrom);
    TESTP(dirto);
    mkdirtree(dirto);

    cursor = dirto + strlen(dirto) - 1;
    if(*cursor == '/' || *cursor == '\\')
        *cursor = 0;
    
#undef TESTP

    if(unique_instance)
    {
        static char mutex_name[MAX_PATH];
        sprintf(mutex_name,"FTPGRABBER_UNIQUE_INST_%s",ipfrom);
        instance_mutex = CreateMutex(NULL,0,mutex_name);
        if(WAIT_OBJECT_0 != WaitForSingleObject(instance_mutex, 1))
        {
            pprintf(g_verbose,"exclusive access could not be established. Another instance is already accessing %s. exiting.\n", ipfrom);
            return 0;
        }
    }

    do{
        time_last_run_started = timerSecondsSince2000();

        FtpGrab(ipfrom,username,userpass,from_ftp_directory,extension_to_match,dirto, overwrite_existing);

        if(continuous_checking)
        {
            U32 dt = timerSecondsSince2000Diff(time_last_run_started);
            U32 period = 60*60;
            Sleep((MAX(period - dt,period))*1000);
        }
    } while(continuous_checking);
    
}
