#include "HeroBrowser.h"

#include "WebKitAdapter.h"
#include <qwebpage.h>
#include <qapplication.h>
#include <qevent.h>
#include <qaction.h>
#include <float.h>		// for _controlfp_s
#include <windows.h>	// for GetQueueStatus() and QS_ defines

WebKitAdapter*	g_browser;
QApplication*	gApplication;
char			g_language[32];	// locale language code, ISO 639-1 alpha-2
QString			g_registryBase;
WebDataRetriever* g_dataRetriever;

static QByteArray	s_SavedSteamSessionAuthTicket;

// N.B. Qt requires FPU set to full precision to operate correctly
// otheriwse it can't adequately resolve timers and events.
// CoH client defaults to 24 bit precision which needs to be changed
// to full precision while HeroBrowser is servicing a call

// More difficult is that this also needs to be the case when Qt
// code is running outside of one of our guarded entry points.
// For example, QApplications event dispatcher internal window
// can have its windowProc called by the system as part of the event
// pump. For example, qt_internal_proc() and QtWndProc()

// @todo For the time being we have placed guards in the low level
// window proc messaging code of Qt (qt_internal_proc() and QtWndProc())
// until such time as we can come up with a suitable custom event dispatcher
// or change CoH client and mapserver to use full precision

//**
// sets FPU to full precision and restores original precision when object
// goes out of scope
class FPU_Precision_Guard
{
public:
	FPU_Precision_Guard()
	{
		_controlfp_s(&saved_fpu_control, 0, 0);		// retrieve current control word
		_controlfp_s(NULL, _PC_64, _MCW_PC);		// set the FPU to full precision
	}
	~FPU_Precision_Guard()
	{
		// restore FPU precision originally set on construction
		_controlfp_s(NULL, saved_fpu_control, _MCW_PC);
	}
	unsigned int saved_fpu_control;	// saved floating point control word
};

//**
// HeroBrowser API calls
DLLAPI int	webBrowser_version(void)
{
	FPU_Precision_Guard fpu_guard;	// set full precision and restores when leave scope
	return 0x00010000;				// current version of HeroBrowser
}

// Qt message output can be useful for debugging and evaluation
// qtDebug(), etc. calls will end up going through this function for
// handling.
#ifdef ENABLE_QT_MESSAGE_OUTPUT
#include <qfile.h>

void hero_qt_message_handler(QtMsgType type, const char *msg)
{
	static QFile logFile;
	QString log_string;
	if (type != QtDebugMsg)
	{
		static const char * msg_type_names[] = {
			"DEBUG",
			"WARNING",
			"ERROR",
			"FATAL"
		};
		log_string = QString().sprintf("%s: %s\n", msg_type_names[type], msg);
	}
	else
	{
		// until we have our own 'log' category we are using the DEBUG channel
		// and we are relying on explicit new lines and wrapping in the messages
		log_string = msg;
	}

	// currently output to debug log
	OutputDebugStringA(log_string.toAscii());

	// and text file colocated with application
	if (!logFile.isOpen())
	{
		logFile.setFileName("herobrowser.log");
		logFile.open(QFile::WriteOnly);
		logFile.resize(0);	// start log over on every run
	}
	logFile.write(log_string.toAscii());

	logFile.flush();
}
#endif	// ENABLE_QT_MESSAGE_OUTPUT

// one time system initialization for embedded web browser system
DLLAPI bool webBrowser_system_init( const char* language,		// locale language for browser (ISO 639-1 alpha-2 codes)
									int max_disk_cache_size,	// disk cache size in bytes, use zero for no cache
									int options,				// web setting flags
									const char *registryBase	// Base path for registry cookie entries
								 )
{
	FPU_Precision_Guard fpu_guard;	// set full precision and restores when leave scope

	max_disk_cache_size;	// not currently referenced since we disabled the cache completely for the moment

	strncpy_s( g_language, sizeof(g_language), language, _TRUNCATE );	// locale language for browser (ISO 639-1 alpha-2 codes)
	g_registryBase = QString(registryBase);

	if (!qApp)	// lazy initialize once for life of client
	{
		static int argc = 0;
		static const char* argv[] = {""};
		QApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);

#ifdef ENABLE_QT_MESSAGE_OUTPUT
		qInstallMsgHandler(hero_qt_message_handler);
#endif

		// Add a search path so that we can find Qt plugins relative to the application
		// as opposed to the Qt build location
		// These are the image converter and codec plugins we need for page content
		gApplication = new QApplication(argc, (char **)argv);
		gApplication->addLibraryPath("./qt_plugins");
		gApplication->addLibraryPath(qApp->applicationDirPath());

		// @todo should allow these to be set from client
		QWebSettings *defaultSettings = QWebSettings::globalSettings();
		defaultSettings->setAttribute(QWebSettings::JavascriptEnabled, true);
		defaultSettings->setAttribute(QWebSettings::JavascriptCanOpenWindows, false);
		defaultSettings->setAttribute(QWebSettings::PluginsEnabled, (options & kWebFlag_EnablePlugins) != 0 );
		defaultSettings->setAttribute(QWebSettings::JavaEnabled, (options & kWebFlag_EnableJava) != 0 );
	}

	return true;
}

// one time system shutdown for embedded web browser system
DLLAPI void webBrowser_system_shutdown(void)
{
	FPU_Precision_Guard fpu_guard;	// set full precision and restores when leave scope
	delete gApplication; gApplication = NULL;
}


DLLAPI bool webBrowser_create(int width, int height)
{
	FPU_Precision_Guard fpu_guard;	// set full precision and restores when leave scope
	if (gApplication)
	{
		g_browser = new WebKitAdapter( width, height, g_language, g_registryBase );

		if (!s_SavedSteamSessionAuthTicket.isEmpty())
		{
			g_browser->setSteamAuthSessionTicket(s_SavedSteamSessionAuthTicket);
			s_SavedSteamSessionAuthTicket.clear();
		}
	}

	return true;
}

DLLAPI void webBrowser_destroy(void)
{
	FPU_Precision_Guard fpu_guard;	// set full precision and restores when leave scope
	delete g_browser; g_browser = NULL;
// @todo destroy on app exit		delete gApplication; gApplication = NULL;
}

// give qt a time slice in milliseconds to process
DLLAPI void webBrowser_tick(int dt)
{
	FPU_Precision_Guard fpu_guard;	// set full precision and restores when leave scope
	if (gApplication && g_browser)
	{
		qApp->processEvents(QEventLoop::AllEvents, dt);
	}
}

// Use this tick if the browser is 'inactive' or does not have focus
// and you want it to receive timer events for animations etc, but you don't want it to eat
// events on you such as keyboard input.
DLLAPI void webBrowser_tick_inactive(int dt)
{
	FPU_Precision_Guard fpu_guard;		// set full precision and restores when leave scope
	if (gApplication && g_browser)
	{
		// if we don't have keyboard focus we don't want Qt occasionally eating keys on us
		// so we only tick the browser after the key has been consumed
		// @todo mouse clicks as well?
		if (!GetQueueStatus(QS_KEY))	
			qApp->processEvents(QEventLoop::AllEvents, dt);
	}
}

DLLAPI void* webBrowser_render()
{
	FPU_Precision_Guard fpu_guard;	// set full precision and restores when leave scope
	if (gApplication && g_browser)
		return g_browser->render();
	else
		return NULL;
}

// call when results of HeroAuth authentication challenge come back from account server
// original challenge is passed back so that it can be matched up with what HeroBrowser
// believes is the current challenge.
// returns false if that challenge is no longer active
DLLAPI bool webBrowser_authorize(char const* username, char const* response, char const* challenge)
{
	FPU_Precision_Guard fpu_guard;	// set full precision and restores when leave scope
	if (gApplication && g_browser)
	{
		return g_browser->authorize(username, response, challenge);
	}
	return false;
}

DLLAPI bool webBrowser_setSteamAuthSessionTicket(void *steam_ticket, int ticket_len)
{
	FPU_Precision_Guard fpu_guard;	// set full precision and restores when leave scope
	QByteArray tempByteArray;

	for (int i = 0; i < ticket_len; i++)
	{
		// Convert steam_ticket from data to string
		char string[3];
		sprintf_s(string, sizeof(string), "%02x", ((unsigned char *)steam_ticket)[i]);
		tempByteArray.append(string);
	}

	if (g_browser)
	{
		return g_browser->setSteamAuthSessionTicket(tempByteArray);
	}
	else
	{
		// Save it for when the browser is created
		s_SavedSteamSessionAuthTicket = tempByteArray;
	}

	return true;
}

DLLAPI bool webBrowser_goto(char const* url)
{
	FPU_Precision_Guard fpu_guard;	// set full precision and restores when leave scope
	if (gApplication && g_browser)
	{
		g_browser->load(url);
		return true;
	}
	else
		return false;
}

DLLAPI bool webBrowser_set_html(char const* html)
{
	FPU_Precision_Guard fpu_guard;	// set full precision and restores when leave scope
	if (gApplication && g_browser)
	{
		g_browser->setHtml(html);
		return true;
	}
	else
		return false;
}

DLLAPI void webBrowser_mouseEvent( int type, int xloc, int yloc, int modifiers )
{
	FPU_Precision_Guard fpu_guard;	// set full precision and restores when leave scope
	if (gApplication && g_browser)
	{
		g_browser->mouseEvent(type, xloc, yloc, modifiers);
	}
}

DLLAPI void webBrowser_keyEvent( int type, int key_type, int key_val, int modifiers )
{
	FPU_Precision_Guard fpu_guard;	// set full precision and restores when leave scope
	if (gApplication && g_browser)
	{
		g_browser->keyEvent(type, key_type, key_val, modifiers);
	}
}

DLLAPI void webBrowser_wheelEvent( int delta, int xloc, int yloc, int modifiers )
{
	FPU_Precision_Guard fpu_guard;	// set full precision and restores when leave scope
	if (gApplication && g_browser)
	{
		g_browser->wheelEvent(delta, xloc, yloc, modifiers);
	}
}

DLLAPI void webBrowser_triggerAction( EHeroWebAction action )
{
	FPU_Precision_Guard fpu_guard;	// set full precision and restores when leave scope
	if (gApplication && g_browser)
	{
		g_browser->triggerAction(action);
	}
}

DLLAPI void webBrowser_activate( void )
{
	FPU_Precision_Guard fpu_guard;	// set full precision and restores when leave scope
	if (gApplication && g_browser)
	{
		g_browser->activate();
	}
}

DLLAPI void webBrowser_deactivate( void )
{
	FPU_Precision_Guard fpu_guard;	// set full precision and restores when leave scope
	if (gApplication && g_browser)
	{
		g_browser->deactivate();
	}
}

// returns the current partner callback
DLLAPI HeroWebPartnerCallback webBrowser_setPartner( HeroWebPartnerCallback partnerCallback )
{
	FPU_Precision_Guard fpu_guard;	// set full precision and restores when leave scope
	HeroWebPartnerCallback previous = NULL;

	if (gApplication && g_browser)
	{
		previous = g_browser->setPartner(partnerCallback);
	}

	return previous;
}

DLLAPI void webDataRetriever_getWebResetData(int (*partnerCallback)(void *), const char * url)
{
	FPU_Precision_Guard fpu_guard;	// set full precision and restores when leave scope

	if (!g_dataRetriever)
	{
		g_dataRetriever = new WebDataRetriever(partnerCallback);
	}
	g_dataRetriever->getWebResetData(url);
}

DLLAPI void webDataRetriever_destroy(void)
{
	FPU_Precision_Guard fpu_guard;	// set full precision and restores when leave scope

	if (g_dataRetriever)
	{
		delete g_dataRetriever;
		g_dataRetriever = NULL;
	}
}