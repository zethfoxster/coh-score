// Interface for QtWebKit based web browser used for City Of Heroes Hybrid
#ifndef _HEROBROWSER_H__
#define _HEROBROWSER_H__

#ifdef BUILD_DLL
#	define DLLAPI extern __declspec(dllexport)
#else
#	define DLLAPI extern __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

enum
{
	kWebFlag_None			= 0x0000000,		// used for 'no flags' to be obvious
	kWebFlag_EnablePlugins	= 0x0000001,		// i.e., external plugin like flash, etc
	kWebFlag_EnableJava		= 0x0000002,		// not currently supported
	kWebFlag_ClearDiskCache	= 0x0000004,		// clear the disk cache
};

enum
{
	kWebEvent_MouseMove,
	kWebEvent_MouseRightButtonDown,
	kWebEvent_MouseRightButtonUp,
	kWebEvent_MouseRightButtonDoubleClick,
	kWebEvent_MouseLeftButtonDown,
	kWebEvent_MouseLeftButtonUp,
	kWebEvent_MouseLeftButtonDoubleClick,
	kWebEvent_KeyPress,
};

enum
{
	kWebKeyModifier_SHIFT		= 0x01,
	kWebKeyModifier_CONTROL		= 0x02,
	kWebKeyModifier_ALT			= 0x04,
};

enum
{
	kWebKeyType_Normal = 0,	// normal 'character' key
	kWebKeyType_Special,	// special keys like arrows, function, etc
};

// define some key codes for special keys
// (these are generally the same as GLUT key codes)

enum
{
	kWebKey_F1 = 1,
	kWebKey_F2 = 2,
	kWebKey_F3 = 3,
	kWebKey_F4 = 4,
	kWebKey_F5 = 5,
	kWebKey_F6 = 6,
	kWebKey_F7 = 7,
	kWebKey_F8 = 8,
	kWebKey_F9 = 9,
	kWebKey_F10 = 10,
	kWebKey_F11 = 11,
	kWebKey_F12 = 12,

	kWebKey_Left = 100,
	kWebKey_Up = 101,
	kWebKey_Right = 102,
	kWebKey_Down = 103,
	kWebKey_PageUp = 104,
	kWebKey_PageDown = 105,
	kWebKey_Home = 106,
	kWebKey_End = 107,
	kWebKey_Insert = 108,
	kWebKey_Delete = 109,
	kWebKey_Tab = 110,
	kWebKey_Enter = 111,
	kWebKey_NumPadEnter = 112,
	kWebKey_Escape = 113,
	kWebKey_Backspace = 114,
};

typedef enum
{
	kWebAction_Back,
	kWebAction_Forward,
	kWebAction_Stop,
	kWebAction_Reload,
	kWebAction_Cut,
	kWebAction_Copy,
	kWebAction_Paste,
} EHeroWebAction;

typedef enum
{
	kWebCursor_Arrow,
	kWebCursor_Wait,
	kWebCursor_IBeam,
	kWebCursor_PointingHand,
	kWebCursor_Forbidden,
	kWebCursor_Busy,
} EHeroWebCursor;

// setup communication partner for notifications and queries of the client
// from the web page (e.g., loading status, auth credentials, etc)
typedef enum
{
	kWebNotice_loadStarted,
	kWebNotice_loadProgress,
	kWebNotice_loadFinished,
	kWebNotice_repaintRequested,
	kWebNotice_linkHovered,
	kWebNotice_linkClicked,
	kWebNotice_contentsChanged,
	kWebNotice_cursorChanged,
	kWebNotice_onHeroAuth,

	kWebNotice_onCookie,
	kWebNotice_onNewFeatureClicked,
} EHeroWebNotice;

// Partner communication conduit

typedef int (*HeroWebPartnerCallback)( EHeroWebNotice notice, void* data );

// some generic data fields for passing information back and forth
// which needs a contract between the caller and the HeroBrowser
typedef struct 
{
	char const* url;	// current url associated with event
	int	i1;
	int i2;
	int	i3;
	int i4;
	char* s1;			// non const string pointers for data relay
	char* s2;
	char* s3;
	char const* sc1;			// const string pointers for data relay
	char const* sc2;
	char const* sc3;
} HeroWebNoticeDataGeneric;

// browser system setup and shutdown (called once during life of client)
DLLAPI bool webBrowser_system_init( const char* language, int max_disk_cache_size, int options, const char *registryBase );
DLLAPI void webBrowser_system_shutdown(void);

// page session, called to create/destroy browsing sessions
DLLAPI bool webBrowser_create(int width, int height);
DLLAPI void webBrowser_destroy(void);

DLLAPI void webBrowser_tick(int dt);
DLLAPI void webBrowser_tick_inactive(int dt);
DLLAPI void* webBrowser_render();

DLLAPI bool webBrowser_authorize(char const* username, char const* response, char const* challenge);
DLLAPI bool webBrowser_setSteamAuthSessionTicket(void *steam_ticket, int ticket_len);

DLLAPI bool webBrowser_goto(char const* url);
DLLAPI bool webBrowser_set_html(char const* html);

DLLAPI void webBrowser_mouseEvent( int type, int xloc, int yloc, int modifiers );
DLLAPI void webBrowser_keyEvent( int type, int key_type, int key_val, int modifiers );
DLLAPI void webBrowser_wheelEvent( int delta, int xloc, int yloc, int modifiers );
DLLAPI void webBrowser_triggerAction( EHeroWebAction action );

DLLAPI void webBrowser_activate(void);
DLLAPI void webBrowser_deactivate(void);

DLLAPI HeroWebPartnerCallback webBrowser_setPartner( HeroWebPartnerCallback partnerCallback );

DLLAPI int	webBrowser_version(void);

DLLAPI void webDataRetriever_getWebResetData(int (*partnerCallback)(void *), const char * url);
DLLAPI void webDataRetriever_destroy(void);

#ifdef __cplusplus
}
#endif

#endif // _HEROBROWSER_H__

