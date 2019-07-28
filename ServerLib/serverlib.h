#pragma once

typedef void (*ServerHandler)(void);
typedef void (*HttpHandler)(char**); // takes a Str with html header in it, page contents should be appended
typedef struct ConsoleDebugMenu ConsoleDebugMenu;

typedef struct StashTableImp* StashTable;
typedef struct HTTPServer HTTPServer;

typedef enum ServerLibFlags
{
	kServerLibDefault				= (1 << 0),
	kServerLibLikePiggs				= (1 << 1),
} ServerLibFlags;

typedef struct ServerLibConfig
{
	const char *name;

	char icon_letter;
	int icon_color;		// 0xrrggbb

	ServerHandler init_handler;	// configuration and other fast operations.
								// register persist layer types here for
								// automatic handling
	ServerHandler load_handler;	// data fixup and other slow operations.
								// registered persist layer types will have
								// already been loaded once this handler is
								// called.
	ServerHandler tick_handler;
	ServerHandler stop_handler;	// clean up after someone closes the app

	HttpHandler http_handler;	// just for summary right now

	ConsoleDebugMenu *consoledebugmenu;	// a header will automatically be appended

	ServerLibFlags flags;

} ServerLibConfig;
extern ServerLibConfig g_serverlibconfig;

typedef struct ServerLibState
{
	char exe_name[MAX_PATH];
	char cmd_line[1024];
	int argc;
	char **argv;

	U32 started;
	char started_readable[32];

	int quitting;
	int verbose;

	int http_port;
	HTTPServer *http_server;

} ServerLibState;
extern ServerLibState g_serverlibstate;
