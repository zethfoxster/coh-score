#ifndef _LOGCOMM_H
#define _LOGCOMM_H

#include "netio.h"
#include "log.h"

typedef struct Entity Entity;
typedef enum LogType LogType;
typedef enum LogLevel LogLevel;

int logNetInit(void);
void dbLog(char const *cmdname, Entity *e, FORMAT fmt, ...);
void dbLogBug(FORMAT fmt, ...);
void logFlush(int force);
void logSendDisconnect(F32 timeout);
void logDisconnectFlush(void);


extern NetLink	log_comm_link;

// don't call these directly, the macros below will keep arguments from being gathered if we are not actually logging the line
void log_ent( Entity * e, LogType type, LogLevel level, int flags, int sg, char * fmt, ...  );
void log_dbid(int dbid, LogType type, LogLevel level, int flags, char * fmt, ...  );
void log_sg( char * name, int id, LogLevel level, int flags, char * fmt, ...  );



#define LOG_ENT(ent, type, level, flags, msg, ...) {if(logShouldWrite(type,level)){ log_ent( ent, type, level, flags, 0, msg, ##__VA_ARGS__ ); }else if(flags&LOG_CONSOLE_ALWAYS){ printf(msg, ##__VA_ARGS__); }}
#define LOG_DBID(dbid, type, level, flags, msg, ...) {if(logShouldWrite(type,level)){ log_dbid( dbid, type, level, flags, msg, ##__VA_ARGS__ ); }else if(flags&LOG_CONSOLE_ALWAYS){ printf(msg, ##__VA_ARGS__); }}
#define LOG_ENT_OLD( ent, msg, ...) LOG_ENT( ent, LOG_ENTITY, LOG_LEVEL_DEPRECATED, 0, msg, ##__VA_ARGS__ )

#define LOG_SG(name, id, level, flags, msg, ...) {if(logShouldWrite(LOG_SUPERGROUP,level)){ log_sg( name, id, level, flags, msg, ##__VA_ARGS__ ); }else if(flags&LOG_CONSOLE_ALWAYS){ printf(msg, ##__VA_ARGS__); }}
#define LOG_SG_ENT(ent, level, flags, msg, ... ) {if(logShouldWrite(LOG_SUPERGROUP,level)){ log_ent( ent, LOG_SUPERGROUP, level, flags, 1, msg, ##__VA_ARGS__ ); }else if(flags&LOG_CONSOLE_ALWAYS){ printf(msg, ##__VA_ARGS__); }}

#endif
