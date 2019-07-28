#ifndef _PROTOCOL_H
#define _PROTOCOL_H
#include "GlobalAuth.h"
// when client send to auth
#define PROTOCOL_VERSION 1

// This must match AUTH_BYTES in auth.h
#define MAX_USERDATA 128
// This must match AUTH_BYTES_ORIG in auth.h
#define MAX_USERDATA_ORIG 16
#define MAX_USERDATA_NEW (MAX_USERDATA - MAX_USERDATA_ORIG)

enum {
	AQ_LOGIN,
	AQ_SERVER_LIST,
	AQ_ABOUT_TO_PLAY,
	AQ_LOGOUT,
	AQ_LOGIN_MD5,
	AQ_SERVER_LIST_EXT, // Server List kind
	AQ_MAX,
};

// when auth send to server
enum {
	SQ_ABOUT_TO_PLAY, // 게임서버에  인증서버가 접속하려는 사용자 정보를 알려준다. 
	SQ_KICK_ACCOUNT, // 게임서버에 인증서버가 해당 계정의 Kick을 요청한다. 
	SQ_SERVER_NUM, // 현재 해당 서버에 접속중인 유저의 명수 요청
	SQ_VERSION,	// 응답 프로토콜 Version
	SQ_PING,	// 응답 프로토콜 PING
	SQ_COMPLETE_USERLIST,
    SQ_USER_DATA,
    SQ_GAME_DATA, // ajackson - to send game data about a UID
	SQ_MAX
};

// when auth send to client
enum{
	AC_PROTOCOL_VER,  // 클라이언트에게 프로토콜 버전을 넘겨줌  : int(oneTimekey), int(protocol)
	AC_LOGIN_FAIL,  // 로그인 실패 이유를 알려주          : reason code
	AC_BLOCKED_ACCOUNT,
	AC_LOGIN_OK,    // 로그인 성공, : int(uid), int(oneTimekey),int(updatekey1),int(updatekey2),int(pay_stat),int(remainTime),int(quotaTime)
	AC_SEND_SERVERLIST,
	AC_SEND_SERVER_FAIL,  // 서버 List를 보내주는 것 실패 : char ( error_code )
	AC_PLAY_FAIL,
	AC_PLAY_OK,
	AC_ACCOUNT_KICKED, // account kick : char(error_code)
	AC_BLOCKED_ACCOUNT_WITH_MSG,
	AC_SC_CHECK_REQ,
	AC_QUEUE_SIZE,
	AC_HANDOFF_TO_QUEUE,
	AC_POSITION_IN_QUEUE,
	AC_MAX
};

// return value to client ( error_reason_code ) AC_LOGIN_FAIL
enum {
	S_IP_ALL_OK = 0,
};
enum {
	S_ALL_OK,			 // no error  //0
	S_DATABASE_FAIL,	 // fail to fetch password data or something bad situation takes place at auth database server // 1
	S_INVALID_ACCOUNT,   // no account //2
	S_INCORRECT_PWD,	 // incorrect password //3
	S_ACCOUNT_LOAD_FAIL, // This account exist in user_auth Table , but not user_account Table //4
	S_LOAD_SSN_ERROR,    // fail to load ssn //5
	S_NO_SERVERLIST,     // somthing wrong happens on server table in lin2db database //6
	S_ALREADY_LOGIN,	 // 이미 로그인해 있는 계정 //7
	S_SERVER_DOWN,       // 선택한 서버가 현재 내려가 있음.//8
	S_INCORRECT_MD5Key,  // 다시 로그인 할것  //9 
	S_NO_LOGININFO,      // 역시 다시 로그인 할것 . 시간이 다되었거나 그런것.. // 10
	S_KICKED_BY_WEB,     // 웹에서 Kick함. // 11
	S_UNDER_AGE,		 // 연령 미달. //12
	S_KICKED_DOUBLE_LOGIN,// 외부에서 이중 접속 시도 //13
	S_ALREADY_PLAY_GAME, // 이미 게임 플레이 중 //14
	S_LIMIT_EXCEED,		 // 월드 서버가 접속자수 제한에 걸려 있음. //15
	S_SERVER_CHECK,	     // GM 만 접속 가능. //16
	S_MODIFY_PASSWORD,	 // 패스워드를 수정한 다음에 들어올것. // 17
	S_NOT_PAID,			 // 돈 내야 함.						//18
	S_NO_SPECIFICTIME,	 // 정량제 남은 시간 없음.			// 19
	S_SYSYTEM_ERROR,	 // 시스템 장애						//20
	S_ALREADY_USED_IP,   // 이미 사용중인 IP				//21
	S_BLOCKED_IP,		 // 중국 사업장 IP					//22
	S_VIP_ONLY,
};

// AS_QUIT_GAME Reason
enum {
	S_QUIT_NORMAL=0,
};

// when world server to auth server
enum{
	AS_PLAY_OK,
	AS_PLAY_FAIL,
	AS_PLAY_GAME,
	AS_QUIT_GAME,
	AS_KICK_ACCOUNT,
	AS_SERVER_USERNUM,
	AS_BAN_USER, // uid, account, account는 uid가 0일 경우 account를 검사함. 
	AS_VERSION,
	AS_PING,
	AS_WRITE_USERDATA, // 9 DB의 User_data 테이블에 사용자 정의 데이타를 넣도록 한다. 
	AS_SET_CONNECT,    // Server가 로그인을 받도록 하는 시점.
	AS_PLAY_USER_LIST, // Server가 Reconnect되는 시점에 보내주는 플레이어 List
	AS_SET_SERVER_ID,
	AS_SERVER_USER_NUM_BY_QUEUE_LEVEL,
	AS_FINISHED_QUEUE,
	AS_SET_LOGIN_FREQUENCY,
	AS_QUEUE_SIZES,
	AS_READ_USERDATA,  // ajackson Sept. 30, 2008 - a read user data -- This is AS_READ_USER_DATA in the dbserver
    AS_WRITE_GAMEDATA, // ajackson Sept. 30, 2008 - a write game data -- This is AS_WRITE_GAME_DATA in the dbserver
    AS_READ_GAMEDATA,  // ajackson Sept. 30, 2008 - a read game data -- This is AS_READ_GAME_DATA in the dbserver
	AS_SHARD_TRANSFER, // DGNOTE - request from "departure" dbserver for a shard transfer
	AS_MAX
};

enum COUNTRY {
	CC_KOREA,
	CC_JAPAN,
};

// IPServer에서 Auth서버로 보낼때 프로토콜
enum {
	IA_SERVER_VERSION, //0
	IA_IP_KIND,
	IA_IP_USE,	// return_code, uid, ip, 
	IA_IP_START_OK,
	IA_IP_START_FAIL,
	IA_IP_USE_FAIL,
	IA_IP_SESSIONKEY, //6
	IA_IP_INSTANTLOGIN_OK,
	IA_IP_INSTANTLOGIN_FAIL,
	IA_IP_KICK,
	IA_IP_READY_FAIL,
	IA_IP_READY_OK,
	IA_IP_SET_STARTTIME_OK,
	IA_IP_SET_STARTTIME_FAIL,
	IA_MAX,
};

enum {
	AI_SERVER_VERSION,
	AI_IP_KIND,
	AI_IP_ACQUIRE,
	AI_IP_RELEASE,
	AI_IP_START_CHARGE,
	AI_IP_STOP_CHARGE,
	AI_IP_INSTANT_START_GAME,
	AI_IP_INSTANT_STOP_GAME,
	AI_IP_KICKED,
	AI_IP_READY_GAME,
	AI_IP_SET_START_TIME,
	AI_MAX,
};

// IPServer와의 에러
// IA_IP_USE_FAIL, AI_IP_START_GAME
enum {
	IP_ALL_OK,			 //0 no error 
	IP_DB_ERROR,			 //1
	IP_ALREADY_USE,		 //2
	IP_LIMIT_OVER,		 //3
	IP_TIME_OUT,
	IP_NOT_EXIST,
	IP_NOT_SUBSCRIBED,
	IP_SESSION_NOT_EXIST,
	IP_UNKNOWN_KIND,
	IP_SESSION_CREATE_FAIL,
	IP_SERVER_SOCKET_FAIL,
	IP_ALREADY_WAIT,
};

enum {
	AW_START,
	AW_QUIT,
	AW_MAX,
};

enum {
	WA_VERSION,
	WA_SEND_OK,
	WA_SEND_FAIL,
	WA_MAX,
};

#endif
