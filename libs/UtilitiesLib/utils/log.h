#ifndef LOG_H
#define LOG_H

C_DECLARATIONS_BEGIN

typedef enum LogType
{
	// Generic
	LOG_CRASH,				 // Crash reports
	LOG_DEBUG_LOG,			 // Logs for debugging purposes.  Usually temporary, while the bug is being tracked down.
	LOG_TEXT_JOURNAL,		 // Logs for the text journaling system
	LOG_ERROR,				 // Errors we want to know about sent here
	LOG_DEPRECATED,			 // Log messages that are probably worthless are sent here, on the off-chance we still need them
	LOG_TEST_CLIENT,		 // Used to generate output from test clients
	LOG_NET,

	// DBserver
	LOG_DELETE,				 // Proof they deleted their character
	LOG_OFFLINE,			 // Players getting moved out of the db into text
	LOG_SYSTEM_SPECS,		 // Log users system specs when the connect to db
	LOG_OVERLOAD_PROTECTION, // Overload protection messages
	LOG_CHAR_SLOT_APPLY,	 // Proof they commited a character slot to a shard

	// Mapserver
	LOG_ENTITY,				 // General log of things on entitys (no chat, powers, or rewards related)
	LOG_POWERS,				
	LOG_SUPERGROUP,	
	LOG_BUG,				 // Player reported bugs
	LOG_CHAT,				 // Chat
	LOG_CHEATING,			 // Egregious cheats sent here
	LOG_REWARDS,			 // Anything to do with player inventory should go here
	LOG_SZE_REWARDS,		 //	Special for zone event rewards
	LOG_SURVEY,		
	LOG_ACCESS_CMDS,		//	access level command logs
	LOG_INTERNAL_CMDS,		//	debug/internal commands
	LOG_MARTY,
	LOG_ADMIN,				 // Logs admin commands
	LOG_PERFORMANCE,
	LOG_DUPLICATE_MESSAGEIDS,

	// Chatserver

	// Account Server
	LOG_ACCOUNT,			 // Generic account log
	LOG_TRANSACTION,		// Account transaction log

	// Arena Server
	LOG_ARENA_EVENT,		 // In depth arena event logs.

	// Auction Server
	LOG_AUCTION,
	LOG_AUCTION_TRANSACTION,

	// Mission Server
	LOG_DELETED_ARCS,
	LOG_MISSIONSERVER, 

	// RAIDSERVER
	LOG_IOP,

	// STATSERVER
	LOG_STATSERVER,

	// Turnstile Server
	LOG_TURNSTILE_SERVER,

	LOG_COUNT,
}LogType;

typedef enum LogLevel
{
	LOG_LEVEL_DISABLED = -2, // Do not log anything
	LOG_LEVEL_ALERT = -1,	 // This is so important that someone should be notified
	LOG_LEVEL_IMPORTANT,	 // Only the most important information, default value
	LOG_LEVEL_VERBOSE,		 // Information that we would like, but has the possibility of spamming the logs with too much data
	LOG_LEVEL_DEPRECATED,	 // there are old things that are doubtful we should even be logging
	LOG_LEVEL_DEBUG,		 // Lots of information for debugging only

	LOG_LEVEL_COUNT,
}LogLevel;


// default
#define LOG_DEFAULT			0
#define LOG_CONSOLE			1<<0 // Display logged line in the console in addition to writing to file
#define LOG_ONCE			1<<1 // Message is only logged once, then stashed (stash will be periodically cleared)
#define LOG_LOCAL			1<<2 // The message is logged on this machine (in addition to sending to logserver if needed)
#define LOG_CONSOLE_ALWAYS	1<<3 // Display logged line in the console even if level is too low

void logSetLevel( LogType type, LogLevel level);
void logSetDefaultFile(char *fname);
void logSetUseTimestamped(bool enabled);
void logSetDir(const char *dir);
char *getLogDir(void);

int logGetTypeFromName( char * name );
char *logGetTypeName(LogType type);
int logLevel( LogType type );
int logShouldWrite( LogType type, LogLevel level);

void printf_stderr(const char * msg, ...);
void printv_stderr(const char * msg, va_list va);

void log_f( LogType type, LogLevel level, int flags, char * msg, ...  );
void log_va( LogType type, LogLevel level, int flags, char * msg, va_list );
void log_msg( LogType type, LogLevel level, int flags, char * msg  );


// These macros cover all the old log lines that I'm pretty sure are stupid.  If we need them again we can set the log level high
// and they will get logged again
#define LOG_OLD(msg, ...) LOG(LOG_DEPRECATED, LOG_LEVEL_DEPRECATED, LOG_DEFAULT, msg, ##__VA_ARGS__ )
#define LOG_OLD_ERR(msg, ...) LOG(LOG_ERROR, LOG_LEVEL_DEPRECATED, LOG_DEFAULT, msg, ##__VA_ARGS__ )
#define LOG_DEBUG(msg, ...) LOG(LOG_DEBUG_LOG, LOG_LEVEL_DEBUG, LOG_CONSOLE, msg, ##__VA_ARGS__ )

// Logs with default value and no flags (not sure I like this wrapper, I want people to think about the level when they add a log message)
#define LOG_STD(type, msg, ...) LOG(type, LOG_LEVEL_IMPORTANT, LOG_DEFAULT, msg, ##__VA_ARGS__ )


#define LOG(type, level, flags, msg, ...) {if(logShouldWrite(type,level) || (flags&LOG_CONSOLE_ALWAYS)){ log_f( type, level, flags, msg, ##__VA_ARGS__ ); }}
#define LOG_AP(type, level, flags, msg, ap) {if(logShouldWrite(type,level) || (flags&LOG_CONSOLE_ALWAYS)){ log_va( type, level, flags, msg, ap ); }}

typedef void (*SendMsgToLogServerFunc)(LogType type, char * msg);
void setSendMsgToLogServerFunc( SendMsgToLogServerFunc func );

void logBackgroundWriterSetSleep(int sleep_ms);
void logWaitForQueueToEmpty(void);
void logMsgQueueStats(size_t * bytes_written, int * queue_count, int * queue_max, size_t * sorted_memory, size_t * sorted_memory_limit);
int logShutdownAndWait();
void logSetMsgQueueSize(int num_msgs);


void logPutMsg(char *msg_str,char *filename);

// This is the same as logPutMsg, but the file is opened in text mode (slower) and
// the log file extension will be .txt instead of .log
// This should only be used for log files which the players readily open (like the chat log)
void logPutMsgTxt(char *msg_str,char *filename);

extern char log_default_fname[MAX_PATH];
extern char log_default_dir[MAX_PATH];

C_DECLARATIONS_END

#endif
