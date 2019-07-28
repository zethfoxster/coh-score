#include "WebKitAdapter.h"
#include "HeroBrowser.h"

#include <qwebpage.h>
#include <qpainter.h>
#include <qfile.h>
#include <qnetworkrequest.h>
#include <qwebframe.h>
#include <qevent.h>
#include <qdebug.h>
#include <qwebelement.h>
#include <qapplication.h>
#include <qcoreapplication.h>
#include <qevent.h>
#include <qaction.h>
#include <qnetworkaccessmanager.h>
#include <qnetworkdiskcache.h>
#include <qsslerror.h>
#include <qsslconfiguration.h>
#include <qauthenticator.h>
#include <qnetworkreply.h>
#include <qlist.h>
#include <shlobj.h>					// for SHGetFolderPath et al and local user app data path
#include <sstream>


static QUrl guessUrlFromString(const QString &string)
{
	QString urlStr = string.trimmed();
	QRegExp test(QLatin1String("^[a-zA-Z]+\\:.*"));

	// Check if it looks like a qualified URL. Try parsing it and see.
	bool hasSchema = test.exactMatch(urlStr);
	if (hasSchema) {
		QUrl url = QUrl::fromEncoded(string.toLatin1(), QUrl::StrictMode);
		if (url.isValid())
			return url;
	}

	// Might be a file.
	if (QFile::exists(urlStr))
		return QUrl::fromLocalFile(urlStr);

	// Might be a shorturl - try to detect the schema.
	if (!hasSchema) {
		int dotIndex = urlStr.indexOf(QLatin1Char('.'));
		if (dotIndex != -1) {
			QString prefix = urlStr.left(dotIndex).toLower();
			QString schema = (prefix == QLatin1String("ftp")) ? prefix : QLatin1String("http");
			QUrl url(schema + QLatin1String("://") + urlStr, QUrl::TolerantMode);
			if (url.isValid())
				return url;
		}
	}

	// Fall back to QUrl's own tolerant parser.
	return QUrl(string, QUrl::TolerantMode);
}

static QEvent::Type convert_to_qt_event_type(int wm_type)
{
	QEvent::Type result;

	switch(wm_type)
	{
	default:
		result = QEvent::None;
		break;

	case kWebEvent_MouseMove:
		result = QEvent::MouseMove;
		break;

	case kWebEvent_MouseRightButtonDown:
	case kWebEvent_MouseLeftButtonDown:
		result = QEvent::MouseButtonPress;
		break;

	case kWebEvent_MouseRightButtonUp:
	case kWebEvent_MouseLeftButtonUp:
		result = QEvent::MouseButtonRelease;
		break;

	case kWebEvent_MouseRightButtonDoubleClick:
	case kWebEvent_MouseLeftButtonDoubleClick:
		result = QEvent::MouseButtonDblClick;
		break;
	}

	return result;
}

static Qt::MouseButton convert_to_qt_button(int wm_type)
{
	Qt::MouseButton result;

	switch(wm_type)
	{
	default:
		result = Qt::NoButton;
		break;
	case kWebEvent_MouseMove:
		result = Qt::NoButton;
		break;
	case kWebEvent_MouseLeftButtonDown:
	case kWebEvent_MouseLeftButtonUp:
		result = Qt::LeftButton;
		break;
	case kWebEvent_MouseRightButtonDown:
	case kWebEvent_MouseRightButtonUp:
		result = Qt::RightButton;
		break;
	}

	return result;
}


static Qt::KeyboardModifiers convert_to_qt_modifiers(int modifiers)
{
	Qt::KeyboardModifiers result = Qt::NoModifier;

	if(modifiers & kWebKeyModifier_SHIFT)
		result |= Qt::ShiftModifier;

	if(modifiers & kWebKeyModifier_CONTROL)
		result |= Qt::ControlModifier;

	if(modifiers & kWebKeyModifier_ALT)
		result |= Qt::AltModifier;
	return result;
}

static Qt::Key convert_to_qt_key(int key_val)
{
	Qt::Key key_qt = Qt::Key_unknown;

	switch (key_val)
	{
		case kWebKey_F1:		key_qt = Qt::Key_F1;		break;
		case kWebKey_F2:		key_qt = Qt::Key_F2;		break;
		case kWebKey_F3:		key_qt = Qt::Key_F3;		break;
		case kWebKey_F4:		key_qt = Qt::Key_F4;		break;
		case kWebKey_F5:		key_qt = Qt::Key_F5;		break;
		case kWebKey_F6:		key_qt = Qt::Key_F6;		break;
		case kWebKey_F7:		key_qt = Qt::Key_F7;		break;
		case kWebKey_F8:		key_qt = Qt::Key_F8;		break;
		case kWebKey_F9:		key_qt = Qt::Key_F9;		break;
		case kWebKey_F10:		key_qt = Qt::Key_F10;		break;
		case kWebKey_F11:		key_qt = Qt::Key_F11;		break;
		case kWebKey_F12:		key_qt = Qt::Key_F12;		break;

		case kWebKey_Left:		key_qt = Qt::Key_Left;		break;
		case kWebKey_Up:		key_qt = Qt::Key_Up;		break;
		case kWebKey_Right:		key_qt = Qt::Key_Right;	    break;
		case kWebKey_Down:		key_qt = Qt::Key_Down;		break;
		case kWebKey_PageUp:	key_qt = Qt::Key_PageUp;	break;
		case kWebKey_PageDown:	key_qt = Qt::Key_PageDown;	break;
		case kWebKey_Home:		key_qt = Qt::Key_Home;		break;
		case kWebKey_End:		key_qt = Qt::Key_End;		break;
		case kWebKey_Insert:	key_qt = Qt::Key_Insert;	break;
		case kWebKey_Delete:	key_qt = Qt::Key_Delete;	break;
		case kWebKey_Tab:		key_qt = Qt::Key_Tab;		break;
		case kWebKey_Enter:		key_qt = Qt::Key_Return;	break;
		case kWebKey_NumPadEnter:key_qt = Qt::Key_Enter;	break;
		case kWebKey_Escape:	key_qt = Qt::Key_Escape;	break;
		case kWebKey_Backspace:	key_qt = Qt::Key_Backspace; break;
	}
	return key_qt;
}

static QWebPage::WebAction convert_to_qt_action(EHeroWebAction action)
{
	QWebPage::WebAction qt_action = QWebPage::NoWebAction;
	switch (action)
	{
	case kWebAction_Back:		qt_action =  QWebPage::Back;		break;
	case kWebAction_Forward:	qt_action =  QWebPage::Forward;		break;
	case kWebAction_Stop:		qt_action =  QWebPage::Stop;		break;
	case kWebAction_Reload:		qt_action =  QWebPage::Reload;		break;
	case kWebAction_Cut:		qt_action =  QWebPage::Cut;			break;
	case kWebAction_Copy:		qt_action =  QWebPage::Copy;		break;
	case kWebAction_Paste:		qt_action =  QWebPage::Paste;		break;
	}
	return qt_action;
}

static EHeroWebCursor convert_from_qt_cursorshape(Qt::CursorShape qt_shape)
{
	EHeroWebCursor hb_shape = kWebCursor_Arrow;
	switch (qt_shape)
	{
	case Qt::ArrowCursor:		hb_shape =  kWebCursor_Arrow;		break;
	case Qt::WaitCursor:		hb_shape =  kWebCursor_Wait;		break;
	case Qt::IBeamCursor:		hb_shape =  kWebCursor_IBeam;		break;
	case Qt::PointingHandCursor:hb_shape =  kWebCursor_PointingHand;break;
	case Qt::ForbiddenCursor:	hb_shape =  kWebCursor_Forbidden;	break;
	case Qt::BusyCursor:		hb_shape =  kWebCursor_Busy;		break;
	default:
		// lots of others, do we care?
		// just return 'arrow' for those until we have a use case
		hb_shape = kWebCursor_Arrow;
		break;
	}
	return hb_shape;
}

static const char* network_operation_string( QNetworkAccessManager::Operation op )
{
	switch (op)
	{
	case QNetworkAccessManager::HeadOperation: return "HEAD"; break;
	case QNetworkAccessManager::GetOperation: return "GET"; break;
	case QNetworkAccessManager::PutOperation: return  "PUT"; break;
	case QNetworkAccessManager::PostOperation: return "POST"; break;
	case QNetworkAccessManager::DeleteOperation: return "DELETE"; break;
	case QNetworkAccessManager::UnknownOperation: return "UNKNOWN"; break;
	}
	return "invalid";
}

//**
// HeroWebView
HeroWebView::HeroWebView() : m_partnerCallback(NULL)
{
}

// returns the current partner callback
HeroWebPartnerCallback HeroWebView::setPartner( HeroWebPartnerCallback partnerCallback )
{
	HeroWebPartnerCallback current = m_partnerCallback;
	m_partnerCallback = partnerCallback;
	return current;
}

// method on our view subclass to get events from web core
// e.g. cursor change events
bool HeroWebView::event(QEvent* event)
{
	if ( event->type() == QEvent::CursorChange )
	{
		Qt::CursorShape shape = cursor().shape();
		EHeroWebCursor hb_shape = convert_from_qt_cursorshape(shape);

		if (m_partnerCallback)		// notify partner of event
		{
			HeroWebNoticeDataGeneric data;
			memset(&data, 0, sizeof(data));
			data.i1 = hb_shape;

			(*m_partnerCallback)( kWebNotice_cursorChanged,  &data );
		}
		return true;
	}
	return false;	// do we want QWebView to get it?
}

//**
// HeroAuth
// details of custom PlayNC http digest authorization

// test if challenge is a HeroAuth challenge
static bool reply_needs_hero_authentication(QNetworkReply* reply)
{
	if (reply->error() == QNetworkReply::AuthenticationRequiredError)
	{
		// Handle PlayNC "HeroAuth" authorization requests
		if ( reply->hasRawHeader(QByteArray("WWW-Authenticate")) )
		{
			QByteArray const& header_value = reply->rawHeader( QByteArray("WWW-Authenticate") );
			
			QString val_string = header_value;
			val_string = val_string.trimmed();
			QStringList parts = val_string.split(' ', QString::SkipEmptyParts);
			if (parts.size()>0 && parts[0].compare( "HeroAuth", Qt::CaseInsensitive ) == 0)
			{
				return true;	// a HeroAuth authentication
			}
		}
	}
	return false;
}

HeroAuth::HeroAuth(void) : m_is_credential_request_pending(false)
{
}

void HeroAuth::clear()
{
	m_username.clear();
	m_response.clear();
	m_options.clear();
	m_url.clear();
	m_post_payload.clear();
	m_op = QNetworkAccessManager::UnknownOperation;

	m_is_credential_request_pending = false;
}

bool HeroAuth::is_challenge_complete()
{
	return !m_is_credential_request_pending && m_username.size() && m_response.size() && m_options.size();
}

void HeroAuth::set_credentials( QByteArray const& username, QByteArray const& response )
{
	m_username = username;
	m_response = response;
	m_is_credential_request_pending = false;
}

// parse the contents of challenge header, return true if it is a valid HeroAuth challenge
bool HeroAuth::parse_authenticate_request( QNetworkReply* reply )
{
	m_url = reply->url();

	m_options.clear();

	m_op = reply->operation();
	m_options["op"] = network_operation_string(m_op);
	m_options["uri"] = reply->url().encodedPath();

	// parse authenticate header challenge
	// @todo we assume m_options follow a following layout
	// Since PlayNC can assure that this is the case this isn't a terrible
	// assumption. If time permits make a more general parser
	QByteArray const& header_value = reply->rawHeader( QByteArray("WWW-Authenticate") );
	QByteArray challenge = header_value;

	// @#@!@#@!@#!@#!@
	// testingl @todo remove
//	challenge = "HeroAuth realm=\"secure.ncsoft.com\", nonce=\"5i7x9xooSniZD6Lo+eThOg==\", opaque=\"t5OXeY4lQQmP/wMq5rPsRA==\", timestamp=\"1311707316.773600\"";
//	m_options["uri"] = QString("/cgi-bin/accountManagement.pl");
	// @#@!@#@!@#!@#!@

	QRegExp rex("realm=\"(.+)\"", Qt::CaseInsensitive, QRegExp::RegExp2);
	rex.setMinimal(true);	//don't be greedy on the end quote

	if (rex.indexIn(challenge)==-1)
		return false;
	m_options["realm"] = rex.cap(1);

	rex.setPattern("nonce=\"(.+)\"");
	if (rex.indexIn(challenge)==-1)
		return false;
	m_options["nonce"] = rex.cap(1);

	rex.setPattern("opaque=\"(.+)\"");
	if (rex.indexIn(challenge)==-1)
		return false;
	m_options["opaque"] = rex.cap(1);

	rex.setPattern("timestamp=\"(.+)\"");
	if (rex.indexIn(challenge)==-1)
		return false;
	m_options["timestamp"] = rex.cap(1);

	return true;
}

QByteArray HeroAuth::get_challenge_to_digest()
{
	QString string_to_digest;

	string_to_digest = m_options["nonce"] + m_options["op"] + m_options["uri"] + m_options["timestamp"];

	return string_to_digest.toLatin1();
}

// return the string to put in the http "Authorization" header for this challenge
QByteArray HeroAuth::get_authorization_header()
{
	QString auth = "HeroAuth ";
	auth += QString("username=\"%1\", ").arg(QString(m_username));
	auth += QString("realm=\"%1\", ").arg( m_options["realm"] );
	auth += QString("nonce=\"%1\", ").arg( m_options["nonce"] );
	auth += QString("uri=\"%1\", ").arg( m_options["uri"] );
	auth += QString("response=\"%1\", ").arg(QString(m_response));
	auth += QString("opaque=\"%1\", ").arg( m_options["opaque"] );
	auth += QString("timestamp=\"%1\", ").arg( m_options["timestamp"] );

	return auth.toLatin1();
}


//**
// HeroNetworkAccessManager

static void debug_log_network_reply(QNetworkReply* reply)
{
	reply;

	#ifdef ENABLE_QT_MESSAGE_OUTPUT
		// report load completion, errors, headers and indicate if it came from the cache
		qDebug() << "net loaded [" << reply->error() << "] " << reply->url().toEncoded() << "\n";
		if (reply->error())
		{
			qDebug() << "   !ERROR [" << reply->error() << "] " << reply->errorString() << "\n";
		}
		if (reply->attribute( QNetworkRequest::SourceIsFromCacheAttribute ).toBool())
		{
			qDebug() << "    CACHE HIT" << "\n";
		}

		QUrl redirection = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
		if (redirection.isValid())
		{
			QUrl newUrl = reply->url().resolved(redirection);
			qDebug() << "    REDIRECTION " << newUrl << " \n";
		}

		QList<QByteArray> httpHeaders = reply->rawHeaderList();	// returns 'key' list, i.e., the header names
		for (int i = 0; i < httpHeaders.size(); ++i)
		{
			const QByteArray &headerName = httpHeaders.at(i);
			qDebug() << "           |- " << QString::fromLatin1(headerName) << "  :  " << QString::fromLatin1(reply->rawHeader(headerName)) << "\n";
		}
	#endif	// ENABLE_QT_MESSAGE_OUTPUT
}

static void debug_log_network_request(HeroNetworkAccessManager::Operation op, QNetworkRequest const& request, QIODevice *outgoingData)
{
	op;
	request;
	outgoingData;

	#ifdef ENABLE_QT_MESSAGE_OUTPUT
		qDebug() << "net request: " << network_operation_string( op ) << " url: " << request.url().toString() << "\n";
		QList<QByteArray> httpHeaders = request.rawHeaderList();	// returns 'key' list, i.e., the header names
		for (int i = 0; i < httpHeaders.size(); ++i)
		{
			const QByteArray &headerName = httpHeaders.at(i);
			qDebug() << "              |- " << QString::fromLatin1(headerName) << "  :  " << QString::fromLatin1(request.rawHeader(headerName)) << "\n";
		}
		if (op == QNetworkAccessManager::PostOperation || op == QNetworkAccessManager::PutOperation)
		{
			if (outgoingData)
			{
				char content[1024];
				qint64 bytesPeeked = outgoingData->peek(content, sizeof(content));
				if (bytesPeeked>0)
				{
					// save form data for retry of post
					QByteArray s( content, bytesPeeked );
					qDebug() << "              |--- displaying "  << bytesPeeked << " of " << outgoingData->size() << " bytes total in request body\n";
					qDebug() << "              |--- " << s << "\n";
				}
			}
			else
			{
				qDebug() << "    WARN      |--- NO REQUEST CONTENT PROVIDED?!\n";
			}
		}

	#endif
}

HeroNetworkAccessManager::HeroNetworkAccessManager(const char* language) : QNetworkAccessManager(NULL), m_language(language), m_SteamSessionAuthTicket()
{
}

bool HeroNetworkAccessManager::setSteamAuthSessionTicket(const QByteArray &steamSessionAuthTicket)
{
	m_SteamSessionAuthTicket = steamSessionAuthTicket;
	return true;
}

void HeroNetworkAccessManager::handleRedirect()
{
	// Reuse saved Steam session authentication ticket
	m_SteamSessionAuthTicket = m_PrevSteamSessionAuthTicket;
	m_PrevSteamSessionAuthTicket.clear();
}

QNetworkReply *HeroNetworkAccessManager::createRequest(Operation op, const QNetworkRequest &request, QIODevice *outgoingData)
{
	QNetworkRequest customized_request(request);

	// @todo
	// provide a client interface for setting arbitrary conditional header customizations

	// Conditionally provide PlayNC required headers
	// For Live, all of the PlayNC requests will be denoted by being located at https://secure.<optioal.>ncsoft.com/ (the entire “secure” subdomain of ncsoft.com belongs to PlayNC.
	// For QA and Dev, the distinction is not as clear-cut and we may just have to settle for http(s)://*.ncsoft.corp.
	QRegExp rx("^(https://secure(\\.[^/]+)*\\.ncsoft\\.com|https?://[^/]+\\.ncsoft\\.corp)", Qt::CaseInsensitive);
	if (rx.indexIn(customized_request.url().toString())>= 0)
	{
		// PlayNC requires custom header fields
		const QByteArray playNC_client_id("314e6d46-712d-4f53-95f8-850b377d0709");
		customized_request.setRawHeader( QByteArray("X-NCsoft-Client-Id"), playNC_client_id );
		customized_request.setRawHeader( QByteArray("X-NCsoft-Language"), m_language );

		// Save the Steam session authentication ticket in case we need it for redirection
		m_PrevSteamSessionAuthTicket = m_SteamSessionAuthTicket;

		if (!m_SteamSessionAuthTicket.isEmpty())
		{
			customized_request.setRawHeader( QByteArray("X-NCsoft-SteamAuthTicket"), m_SteamSessionAuthTicket );
			m_SteamSessionAuthTicket.clear();
		}

		// begin hack
		// if we are POSTing to PlayNC then hold onto the contents of the post in case we need to
		// play it back for an auth challenge
		if (op == QNetworkAccessManager::PostOperation && outgoingData && !m_current_hero_auth.is_challenge_complete())
		{
			// N.B. outgoingData->size() is often going to be zero unless we do a first access on the data, bug?
			// so we do it explicitly first with the overload that works and provides more information
			// about exactly what is happening.
			char content[32];		// just an initial placeholder to get size set
			qint64 bytesPeeked = outgoingData->peek(content, sizeof(content));
			if (bytesPeeked>0)
			{
				// now that we've 'primed the pump' ask for the whole enchilada
				m_plaync_post_contents = outgoingData->peek(outgoingData->size());
			}
			#ifdef ENABLE_QT_MESSAGE_OUTPUT
				if (bytesPeeked>0)
					qDebug() << "hero auth:  storing post data for potential auth: " << m_plaync_post_contents << "\n";
				else
					qDebug() << "hero auth:  ERROR could not save post data for potential auth: " << "\n";
			#endif
		}
	}
	else
	{
		// @todo
		// always provide a standard "Accept-Language" header?
		// Some websites expect this but QtWebKit usually fills one in using the system
		// locale I believe.
//		customized_request.setRawHeader( QByteArray("Accept-Language"), m_language );
	}

	// Is this a completed HeroAuth authentication request replay?
	if (m_current_hero_auth.is_challenge_complete())
	{
		#ifdef ENABLE_QT_MESSAGE_OUTPUT
				qDebug() << "hero auth:  challenge complete. embedding authorization into request: " << m_current_hero_auth.get_authorization_header() << "\n";
		#endif
		// add the authorization headers
		customized_request.setRawHeader( QByteArray("Authorization"), m_current_hero_auth.get_authorization_header() );

		// done with the auth
		m_current_hero_auth.clear();
		m_plaync_post_contents.clear();
	}

	debug_log_network_request(op, customized_request, outgoingData);

	return QNetworkAccessManager::createRequest(op, customized_request, outgoingData);
}

#if 0 // currently handling signal on the page, i.e. see net_finishedSlot()
void HeroNetworkAccessManager::finishedSlot(QNetworkReply* reply)
{
	// (good place if we want to determine which requests end up as QNetworkReply::ContentNotFoundError)
}
#endif

void NewFeature::Open(int featureID)
{
	emit newFeatureClicked(featureID);
}

//**
// WebKitAdapter
WebKitAdapter::WebKitAdapter( int width, int height, const char* language, const QString &registryBase )
	: QObject(),  m_cookieJar(registryBase), m_netAccessManager(language), m_page(NULL), m_language(language), m_partnerCallback(NULL) 
{
	m_pageSize = QSize(width, height);
	m_page.setViewportSize(m_pageSize);

	// We use a scene instead of rendering the QWebPage directly so that we can proxy and render any control windows that may draw on
	// top of the web content (like form select combo boxes)
	m_view_web.setPage(&m_page);

	m_scene.addItem(&m_view_web);	// scene owns the webview now and will delete at destruction
	m_view_scene.setScene(&m_scene);
	m_view_scene.viewport()->setParent(NULL);
	m_scene.setStickyFocus(true);
	m_view_web.resize(width, height);
	m_view_scene.resize(width, height);

	// create the target surface
	m_image = QImage(m_pageSize, QImage::Format_RGB32); // Format_ARGB32_Premultiplied, or Format_RGB32
	m_image.fill(Qt::transparent);


	// In addition to setting the active cookie jar for the netAccessManger, this call also
	// makes netAccessManger the parent of the cookieJar, which means netAccessManger will
	// try to delete the cookie jar when netAccessManger is destroyed.
	m_netAccessManager.setCookieJar(&m_cookieJar);

	// Calling setParent(0) here to stop m_netAccessManager deleting the cookie jar on destruction.
	// m_cookieJar will be destroyed normally when WebKitAdapter is destroyed.
	m_cookieJar.setParent(0);

	// setup to cache network accesses
#if 0
	// 07-18-2011
	// Disk cache for network requests is being DISABLED for a few reasons.

	// The QNetworkDiskCache is very basic and its implementation leads to some problems.
	// Particularly with lots of small cached items.
	// The cached items are saved as individual files in the cache folder. There isn't a
	// index or catalog that is maintained. On the very first request that does a cache verification
	// the contents of the entire cache have to be scanned on disk. This can be time consuming for
	// a cache with a large number of files, especially if they cause a lot of drive seeks given that
	// cache files can easily be placed all over the disk. This will be more of a problem initially before
	// the files are warm in the system file cache. But there is still a problem incrementally adding
	// newly cached items when the there are already a lot of items in the cache folder.

	// I did an experiment to change the implementation so that the disk cache is created up front
	// once as part of the 'init_once' and then supplied to each browser instance that gets created instead
	// of creating it fresh here and guaranteeing that we will pay the price of the initial cache expire call.
	// However, the network access manager currently takes ownership of the cache and destroys it upon
	// destruction. That means network access manager would need to be created up front as well but
	// we need the network access manager to delete all it's request before the page is destroyed and there
	// isn't time for that change.

	// There is also some evidence that cache behavior is not operating correctly in all cases, i.e. caching
	// files that should not be cached and returning stale data. There may be bugs in
	// QNetworkAccessHttpBackend::validateCache() which is supposed to default to "PreferNetwork" in absence of
	// an explicit attribute in the http request.
	{
		char szUserFolder[1024];
		// This method gets the "Local Settings\Application Data" folder of the current user
		HRESULT hRes = SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA|CSIDL_FLAG_CREATE, NULL, 0, szUserFolder);
		if (SUCCEEDED(hRes))
		{
			#define LOCAL_USER_CACHE_SUBDIR			"\\NCSoft\\CoX\\webcache"
			strcat_s(szUserFolder, sizeof(szUserFolder), LOCAL_USER_CACHE_SUBDIR );
			SHCreateDirectoryEx(NULL, szUserFolder, NULL);
			m_netDiskCache.setCacheDirectory( szUserFolder );
//			m_netDiskCache.setCacheDirectory(qApp->applicationDirPath() + "/webcache");
			// By default the max cache size is 50M, this can be set to a different value however
			m_netAccessManager.setCache(&m_netDiskCache);
		}
	}
#endif
	m_page.setNetworkAccessManager(&m_netAccessManager);
	mp_newFeature = new NewFeature();

	// link JS bridge
	connect(m_page.mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(addJSObject()));

	// connect signals for partner notifications and queries
	connect(&m_page, SIGNAL(loadStarted()), this, SLOT(loadStartedSlot()));
	connect(&m_page, SIGNAL(loadProgress(int)), this, SLOT(loadProgressSlot(int)));
	connect(&m_page, SIGNAL(loadFinished(bool)),  this, SLOT(loadFinishedSlot(bool)));
	connect(&m_page, SIGNAL(linkHovered(const QString &, const QString &, const QString &)), this, SLOT(linkHoveredSlot(const QString &, const QString &, const QString &)));
	connect(mp_newFeature, SIGNAL(newFeatureClicked(int)),  this, SLOT(newFeatureClickedSlot(int)));

	// scene is now in charger of telling us if one of its widgets or the web page needs a repaint
//	connect(&m_page, SIGNAL(repaintRequested(const QRect&)), this, SLOT(repaintRequestedSlot(const QRect&)));
	connect(&m_scene, SIGNAL(changed(const QList<QRectF> &)), this, SLOT(sceneChangedSlot(const QList<QRectF> &)));

	// connect signals from the network manager, particularly for secure connections
	connect(&m_netAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(net_finishedSlot(QNetworkReply*)));
	connect(&m_netAccessManager, SIGNAL(authenticationRequired(QNetworkReply*, QAuthenticator*)), this, SLOT(authenticationRequiredSlot(QNetworkReply*, QAuthenticator*)));
	connect(&m_netAccessManager, SIGNAL(sslErrors( QNetworkReply *, const QList<QSslError> &)), this, SLOT(sslErrorsSlot( QNetworkReply *, const QList<QSslError> &  )));

	// give the page focus so that the edit box carets will render
	QFocusEvent event( QEvent::FocusIn, Qt::ActiveWindowFocusReason);
	qApp->sendEvent(&m_page, &event);

}

WebKitAdapter::~WebKitAdapter()
{
	delete mp_newFeature;

	// remove webview from scene so that
	// we owns it and the webview doesn't try to delete it
	m_scene.removeItem(&m_view_web);	
}

// called to resolve a previous HeroAuth authentication challenge from PlayNC
// return false if the authorization is no longer current
bool WebKitAdapter::authorize(char const* username, char const* response, char const* challenge)
{
	if (!m_netAccessManager.m_current_hero_auth.m_is_credential_request_pending)
		return false;
	// is it a response to the current challenge?
	if (m_netAccessManager.m_current_hero_auth.get_challenge_to_digest() != challenge)
		return false;

	// ok set credentials and reissue the request such that the authorization http headers
	// will be setup
	m_netAccessManager.m_current_hero_auth.set_credentials(username, response);
	// a completed challenge?
	if (!m_netAccessManager.m_current_hero_auth.is_challenge_complete())
		return false;

	// partner may not have been able to complete the challenge for various reasons
	// (e.g., server is not available to sign the challenge) and it tells us that
	// through special strings sent in the response
	if (_stricmp(response, "timeout")==0)
	{
		#ifdef ENABLE_QT_MESSAGE_OUTPUT
				qDebug() << "hero auth:  partner challenge timeout " << "\n";
		#endif
		m_netAccessManager.m_current_hero_auth.clear();	// clear the pending auth request

		QString html("<html><body bgcolor=black text=white><p/><p/><h3>&nbsp;&nbsp;&nbsp;%1</h3></body></html>");
		QString wait("Server is not responding. Please try again later.");
		if (m_language == "de")
			wait = "Der Server antwortet nicht. Bitte versuchen Sie es später erneut.";
		else if (m_language == "fr")
			wait = "Le serveur ne répond pas. Veuillez réessayer plus tard.";
		QString page_html = html.arg(wait);

		m_page.triggerAction(QWebPage::Stop);	// stop any loads in progress
		m_view_web.setHtml( page_html, QUrl() );
		return false;
	}

	//**
	// should be a successful challenge
	// retry the original request
	m_page.triggerAction(QWebPage::Stop);	// stop any loads in progress

	// assuming we only have to support a POST and GET
	// @todo will we ever need to auth a PUT or other HTTP command?
	#ifdef ENABLE_QT_MESSAGE_OUTPUT
		qDebug() << "hero auth:  authorized. retrying request. " << "\n";
	#endif
	if (m_netAccessManager.m_current_hero_auth.get_operation() == QNetworkAccessManager::PostOperation)
	{
		QNetworkRequest	request(m_netAccessManager.m_current_hero_auth.m_url);
		m_page.mainFrame()->load( request, QNetworkAccessManager::PostOperation, m_netAccessManager.m_current_hero_auth.get_post_data() );
	}
	else
	{
		// @todo this could be done the same way as the post but only changing as little as possible atm
		m_page.mainFrame()->load(m_netAccessManager.m_current_hero_auth.m_url);
	}

	return true;
}

bool WebKitAdapter::setSteamAuthSessionTicket(const QByteArray &steamSessionAuthTicket)
{
	return m_netAccessManager.setSteamAuthSessionTicket(steamSessionAuthTicket);
}

void WebKitAdapter::setHtml(const char* html)
{
	m_page.triggerAction(QWebPage::Stop);	// stop any loads in progress
	m_view_web.setHtml( html, QUrl() );
}

void WebKitAdapter::load(const char* uri)
{
	m_page.triggerAction(QWebPage::Stop);	// stop any loads in progress

	QUrl url = guessUrlFromString(QString::fromLatin1(uri));
	m_page.mainFrame()->load(url);
}

void WebKitAdapter::loadStartedSlot()
{
	// if for some reason we are starting another page load then cancel any pending auth
	// unless this is the auth retry
	if (m_netAccessManager.m_current_hero_auth.is_challenge_in_progress()
			&& !m_netAccessManager.m_current_hero_auth.is_challenge_complete())
	{
		m_netAccessManager.m_current_hero_auth.clear();
	}

	if (m_partnerCallback)		// notify partner of signal
	{
		HeroWebNoticeDataGeneric data;
		memset(&data, 0, sizeof(data));
		QByteArray aURL = m_page.mainFrame()->url().toString().toAscii();
		data.url = aURL.constData();
		(*m_partnerCallback)( kWebNotice_loadStarted,  &data );
	}
}

void WebKitAdapter::loadProgressSlot(int progress)
{
	if (m_partnerCallback)		// notify partner of signal
	{
		HeroWebNoticeDataGeneric data;
		memset(&data, 0, sizeof(data));
		QByteArray aURL = m_page.mainFrame()->url().toString().toAscii();
		data.url = aURL.constData();
		data.i1 = progress;
		(*m_partnerCallback)( kWebNotice_loadProgress, &data );
	}

}

//void WebKitAdapter::repaintRequestedSlot(const QRect& dirtyRect)
void WebKitAdapter::sceneChangedSlot(const QList<QRectF> & region)
{
	region; // not currently used

	if (m_partnerCallback)		// notify partner of signal
	{
		HeroWebNoticeDataGeneric data;
		memset(&data, 0, sizeof(data));
		QByteArray aURL = m_page.mainFrame()->url().toString().toAscii();
		data.url = aURL.constData();
//		data.i1 = dirtyRect.top();
//		data.i2 = dirtyRect.left();
//		data.i3 = dirtyRect.bottom();
//		data.i4 = dirtyRect.right();
		(*m_partnerCallback)( kWebNotice_repaintRequested, &data );
	}

}

void WebKitAdapter::loadFinishedSlot(bool ok)
{
	if (m_partnerCallback)		// notify partner of signal
	{
		HeroWebNoticeDataGeneric data;
		memset(&data, 0, sizeof(data));
		QByteArray aURL = m_page.mainFrame()->url().toString().toAscii();
		data.url = aURL.constData();
		data.i1 = ok;
		(*m_partnerCallback)( kWebNotice_loadFinished, &data );
	}
}

void WebKitAdapter::linkHoveredSlot(const QString &link, const QString &title, const QString &textContent)
{
	if (m_partnerCallback)		// notify partner of signal
	{
		HeroWebNoticeDataGeneric data;
		memset(&data, 0, sizeof(data));
		QByteArray aURL = m_page.mainFrame()->url().toString().toAscii();
		data.url = aURL.constData();

		// could probably use qPrintable to accomplish same
		// but this guarantees buffer existence across the callback
		QByteArray a1 = link.toAscii();
		data.sc1 =  a1.constData();	
		QByteArray a2 = title.toAscii();
		data.sc2 = a2.constData();
		QByteArray a3 = textContent.toAscii();
		data.sc3 = a3.constData();
		// N.B. the string data pointers won't be valid past this call
		(*m_partnerCallback)( kWebNotice_linkHovered, &data );
	}
}

void WebKitAdapter::newFeatureClickedSlot(int id)
{
	if (m_partnerCallback)		// notify partner of signal
	{
		HeroWebNoticeDataGeneric data;
		memset(&data, 0, sizeof(data));
		data.i1 = id;
		(*m_partnerCallback)( kWebNotice_onNewFeatureClicked, &data );
	}
}

void WebKitAdapter::net_finishedSlot(QNetworkReply* reply)
{
	debug_log_network_reply(reply);

	QUrl redirection = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
	if (redirection.isValid())
	{
		m_netAccessManager.handleRedirect();
	}

	if (reply->error() == QNetworkReply::AuthenticationRequiredError)
	{
		// if this is a reply to retry that we already authorized then
		// the auth failed and we just let it fall through and say that
		// instead of getting into an unending loop of auths
		bool request_was_autherization_attempt = reply->request().hasRawHeader( QByteArray("Authorization") );

		if (!request_was_autherization_attempt && reply_needs_hero_authentication(reply) && m_partnerCallback)
		{
			#ifdef ENABLE_QT_MESSAGE_OUTPUT
				qDebug() << "hero auth:  reply has authorization challenge." << "\n";
			#endif
			// supersede any existing challenge
			m_netAccessManager.m_current_hero_auth.clear();
			// parse the challenge and request credentials if it is valid
			if (m_netAccessManager.m_current_hero_auth.parse_authenticate_request(reply))
			{
				// If authorizing a post request copy the post data from the request
				// into the auth so we have it to play back on the retry
				if (reply->operation() == QNetworkAccessManager::PostOperation)
				{
					m_netAccessManager.m_current_hero_auth.set_post_playload(m_netAccessManager.m_plaync_post_contents);
				}

				// stop this replies 'unathorized' content from being displayed
				// we will reissue the request with the challenge response
				m_page.triggerAction(QWebPage::Stop);

				// we don't want to paint this responses unauthorized content
				// so we reset the view to have some more benign html
				QString html("<html><body bgcolor=black text=white><p/><p/><h3>&nbsp;&nbsp;&nbsp;%1</h3></body></html>");
				QString wait("Please wait...");
				if (m_language == "de")
					wait = "Bitte warten ...";
				else if (m_language == "fr")
					wait = "Veuillez patienter...";
				QString page_html = html.arg(wait);

				m_view_web.setHtml( page_html, QUrl() );

				// ask partner to respond to the challenge
				HeroWebNoticeDataGeneric data;
				memset(&data, 0, sizeof(data));
				QByteArray url_dst = reply->url().toString().toAscii();
				data.url = url_dst.constData();

				QByteArray challenge_string = m_netAccessManager.m_current_hero_auth.get_challenge_to_digest();
				data.sc1 = challenge_string.constData();

				m_netAccessManager.m_current_hero_auth.m_is_credential_request_pending = true;
				(*m_partnerCallback)( kWebNotice_onHeroAuth, &data );

				// when credentials come back we use m_current_hero_auth to retry the request
				// with the correct challenge response
			}
			else
			{
				// malformed HeroAuth, just skip it and show the unauthorized content
			}
		}
	}
	else if (reply->error() == QNetworkReply::OperationCanceledError)
	{
#if 0	
		// pending requests are also cancelled when browser is shutting down
		// need to have protection for that if we want to use this in general

		// Show an error when operation gets cancelled
		QString html("<html><body bgcolor=black text=white><p/><p/><h3>&nbsp;&nbsp;&nbsp;%1</h3></body></html>");
		QString content("Operation cancelled.");
		if (m_language == "de")
			content = "Storniert.";
		else if (m_language == "fr")
			content = "Annulée.";
		QString page_html = html.arg(content);

		m_view_web.setHtml( page_html, QUrl() );
#endif
	}
	else if (reply->error() == QNetworkReply::ContentNotFoundError)
	{
		// @todo
		// page not found screen?
	}

}

void WebKitAdapter::sslErrorsSlot(QNetworkReply* reply, const QList<QSslError>& errors)
{
#ifdef ENABLE_QT_MESSAGE_OUTPUT
	qDebug() << "HeroBrowser SSL errors: " << errors << "\n";
	qDebug() << "  Peer Certificate Chain:\n";

	QSslCertificate cert;
	foreach(cert, reply->sslConfiguration().peerCertificateChain())
	{
		qDebug() << "  |--Certificate: " << cert << "\n";
		qDebug() << "  |  Issue      : " << cert.issuerInfo(QSslCertificate::CommonName) << "\n";
		qDebug() << "  |  Subject    : " << cert.subjectInfo(QSslCertificate::CommonName) << "\n";
	}
#else
	Q_UNUSED(errors);
#endif
	// currently we just ignore errors
	reply->ignoreSslErrors();
}

void WebKitAdapter::authenticationRequiredSlot(QNetworkReply *reply, QAuthenticator *authenticator)
{
	Q_UNUSED(reply);
	Q_UNUSED(authenticator);
	// @todo
	// The built in private authenticator of Qt handles a few different flavors of 
	// standard http auths (e.g. basic, digest md5,etc)
	// However, we would have to modify the Qt source code to support the PlayNC
	// "HeroAuth" digest variant since it is currently not implemented to allow
	// you to add custom auth handling.
	// Also, note that internally the auth will try to do a resend but we can't
	// try to two step on something like basic with a flag auth as createRequest
	// is not called again with the resend.
}

// Our original strategy just asked the page to render directly.
// While this was simple and straightforward it did not allow us to
// capture the rendering of any widgets that Qt rendered on top of the web
// content such as the window controls for forms (e.g. select combo boxes)
// Thus, we've changed to using a QGraphicsView so that those controls will
// get proxied and rendered along with the page content into our surface.
void* WebKitAdapter::render(void)
{
	QPainter p(&m_image);
	QRectF	dst_rect(0, 0, m_image.width(), m_image.height());
	QRect	src_rect(0, 0, m_image.width(), m_image.height());
	m_view_scene.render(&p, dst_rect, src_rect);
	p.end();
	// @todo, setup client usage so that this isn't necessary
	// or at least optional
	m_image = m_image.rgbSwapped();

	return m_image.bits();
}


// Qt has a friend declaration we can implement to create a method to inject
// spontaneous mouse events into the graphics view
bool	qt_sendSpontaneousEvent(QObject *receiver, QEvent *event)
{
	return QCoreApplication::sendSpontaneousEvent(receiver,event);
}

void WebKitAdapter::mouseEvent( int type, int xloc, int yloc, int modifiers )
{
	static Qt::MouseButtons s_current_mouse_buttons;

	// generate corresponding Qt event
	QEvent::Type qt_type = convert_to_qt_event_type(type);
	Qt::MouseButton qt_button = convert_to_qt_button(type);
	Qt::KeyboardModifiers qt_modifiers = convert_to_qt_modifiers(modifiers);

	switch(qt_type)
	{
	case QEvent::MouseButtonPress:
	case QEvent::MouseButtonDblClick:
		s_current_mouse_buttons |= qt_button;
		break;

	case QEvent::MouseButtonRelease:
		s_current_mouse_buttons &= ~qt_button;
		break;
	}

	QMouseEvent event(qt_type, QPoint(xloc, yloc), qt_button, s_current_mouse_buttons, qt_modifiers);
	// inject it
//	QCoreApplication::sendEvent(&m_view_scene, &event);	// now sent to scene instead of page so widgets on top of page also get events
	qt_sendSpontaneousEvent(m_view_scene.viewport(), &event);
}

void WebKitAdapter::keyEvent( int type, int key_type, int key_val, int modifiers )
{
	Q_UNUSED(type);

	Qt::KeyboardModifiers qt_modifiers = convert_to_qt_modifiers(modifiers);
	Qt::Key key_qt;

	char utf8_text[2] = {0,0};

	if (key_type == kWebKeyType_Normal)
	{
		key_qt = (Qt::Key)toupper(key_val);
		if(key_val < 0x80)
		{
			utf8_text[0] = (char)key_val;
		}
	}
	else
	{
		// kWebKeyType_Special
		// convert special key
		key_qt = convert_to_qt_key(key_val);
		// some of the special characters need more information to be successfully parsed
		if (key_qt == Qt::Key_Return || key_qt == Qt::Key_Enter)
		{
			utf8_text[0] = 0x0D;
		}
		else if (key_qt == Qt::Key_Delete)
		{
			utf8_text[0] = 0x2E;
		}
	}


	// inject it
	QString text = QString::fromUtf8(utf8_text);
	QKeyEvent event(QEvent::KeyPress, key_qt, qt_modifiers, text);
	QCoreApplication::sendEvent(&m_scene, &event);	// now sent to scene instead of page so widgets on top of page also get events
}

void WebKitAdapter::wheelEvent( int delta, int xloc, int yloc, int modifiers )
{
	Qt::MouseButtons qt_buttons = Qt::NoButton;
	Qt::KeyboardModifiers qt_modifiers = convert_to_qt_modifiers(modifiers);

	QWheelEvent event( QPoint(xloc, yloc), delta, qt_buttons, qt_modifiers);
	QCoreApplication::sendEvent(m_view_scene.viewport(), &event);	// now sent to scene instead of page so widgets on top of page also get events
}

void WebKitAdapter::triggerAction( EHeroWebAction action )
{
	QWebPage::WebAction qt_action = convert_to_qt_action(action);
	m_page.triggerAction(qt_action);
}

void WebKitAdapter::activate(void)
{
	QEvent evt_activate(QEvent::WindowActivate);
	qApp->sendEvent(&m_scene,&evt_activate);
	QEvent evt_change(QEvent::ActivationChange);
	qApp->sendEvent(&m_scene,&evt_change);

	// give the page focus so that the edit box carets will render
	QFocusEvent event( QEvent::FocusIn, Qt::ActiveWindowFocusReason);
	qApp->sendEvent(&m_page, &event);
}

void WebKitAdapter::deactivate(void)
{
	QEvent evt_deactivate(QEvent::WindowDeactivate);
	qApp->sendEvent(&m_scene,&evt_deactivate);
	QEvent evt_change(QEvent::ActivationChange);
	qApp->sendEvent(&m_scene,&evt_change);

	// take away page focus so that the edit box carets will be removed
	QFocusEvent event( QEvent::FocusOut, Qt::ActiveWindowFocusReason);
	qApp->sendEvent(&m_page, &event);
}

// returns the current partner callback
HeroWebPartnerCallback WebKitAdapter::setPartner( HeroWebPartnerCallback partnerCallback )
{
	HeroWebPartnerCallback current = m_partnerCallback;
	m_partnerCallback = partnerCallback;
	m_view_web.setPartner(partnerCallback);		// view also needs to know about partner
	return current;
}

void WebKitAdapter::addJSObject(void)
{
	// Add mp_newFeature to JavaScript Frame as member "NewFeature".
	m_page.mainFrame()->addToJavaScriptWindowObject(QString("NewFeature"), mp_newFeature);
}

WebDataRetriever::WebDataRetriever(int (*partnerCallback)(void *))
: m_netAccessManager(this), m_partnerCallback(partnerCallback)
{
	connect(&m_netAccessManager, SIGNAL(finished(QNetworkReply *)), this, SLOT(finishedSlot(QNetworkReply *)));
}

void WebDataRetriever::getWebResetData(const char * url)
{
	//request http text from url
	m_netAccessManager.get(QNetworkRequest(QUrl(url)));
}

void WebDataRetriever::finishedSlot(QNetworkReply * reply)
{
	// read requested text into stream and manipulate
	QByteArray retrieved = reply->readAll();
	std::stringstream ss(std::stringstream::in | std::stringstream::out);
	HeroWebNoticeDataGeneric data;
	char stringLine[128];
	
	ss.exceptions(std::ios::failbit);
	memset(&data, 0, sizeof(data));

	try
	{
		ss << retrieved.constData();
		ss >> data.i1;
		ss.getline(stringLine, 128); //get rid of the newline
		ss.getline(stringLine, 128);
		data.s1 = stringLine;

		// verify there's substance to send back, otherwise don't
		if (stringLine[0])
		{
			//callback using data gathered from web page
			(*m_partnerCallback)( &data );
		}
	}
	catch (...)
	{
		return;
	}
}

// include auto generated moc compiler data
#include "moc_WebKitAdapter.cpp"


