#ifndef _WEBKITADAPTER_H__
#define _WEBKITADAPTER_H__

#include <qwebpage.h>
#include <qnetworkaccessmanager.h>
#include <qnetworkdiskcache.h>
#include <qwebview.h>
#include <qgraphicsscene.h>
#include <qgraphicsview.h>
#include <qgraphicswebview.h>
#include <qnetworkreply.h>
#include <qnetworkcookiejar.h>
#include <qmutex.h>
#include <qsettings.h>
#include "HeroBrowser.h"

// debug build will redirect qt messaging streams (e.g. qDebug()) to debugger outpu
#ifndef NDEBUG
#define ENABLE_QT_MESSAGE_OUTPUT 1
#endif

class PersistentCookieJar : public QNetworkCookieJar {
public:
	PersistentCookieJar(const QString &in_registryBase) : QNetworkCookieJar(0), registryBase(in_registryBase) { load(); }
	~PersistentCookieJar() { save(); }

	virtual QList<QNetworkCookie> cookiesForUrl(const QUrl &url) const
	{
		QMutexLocker lock(&mutex);
		return QNetworkCookieJar::cookiesForUrl(url);
	}

	virtual bool setCookiesFromUrl(const QList<QNetworkCookie> &cookieList, const QUrl &url)
	{
		QMutexLocker lock(&mutex);
		return QNetworkCookieJar::setCookiesFromUrl(cookieList, url);
	}

	void save()
	{
		QMutexLocker lock(&mutex);
		QList<QNetworkCookie> list = allCookies();
		QByteArray data;
		foreach (QNetworkCookie cookie, list) {
			if (!cookie.isSessionCookie()) {
				data.append(cookie.toRawForm());
				data.append("\n");
			}
		}
		QSettings settings(registryBase, QSettings::NativeFormat);
		settings.setValue("Cookies",data);
	}

	// clear the cookie jar
	void clear()
	{
		QList<QNetworkCookie> emptyList;
		setAllCookies(emptyList);
	}

private:
	void load()
	{
		QMutexLocker lock(&mutex);
		QSettings settings(registryBase, QSettings::NativeFormat);
		QByteArray data = settings.value("Cookies").toByteArray();
		setAllCookies(QNetworkCookie::parseCookies(data));
	}

	mutable QMutex mutex;
	QString registryBase;
};

// We use a QWebView subclass to 'complete the circuit' for events coming from WebCore to change
// the cursor. Setting a view creates a QWebPageClient on the page and forwards the cursor change
// events along to the view.
// Everything else has been done directly and simply through a QWebPage instance.
// If we want to render form controls into the surface (e.g. select combo box) along with the
// web content then we change tactics to use a QGraphicsScene...so our web view needs to be
// one that we can add to the scene hence change of base class from QWebView to QGraphicsWebView
class HeroWebView : public QGraphicsWebView
{
	Q_OBJECT

public:
	HeroWebView();
	HeroWebPartnerCallback setPartner( HeroWebPartnerCallback partnerCallback );

protected:
	bool event(QEvent* event);

private:
	HeroWebPartnerCallback m_partnerCallback;
};

// details of custom PlayNC http digest authorization
struct HeroAuth
{
	QByteArray m_username;
	QByteArray m_response;
	QUrl		m_url;
	bool		m_is_credential_request_pending;
	QHash<QByteArray, QString> m_options;
	QNetworkAccessManager::Operation m_op;
	QByteArray m_post_payload;

	HeroAuth();

	void clear();
	bool is_challenge_in_progress() {return m_is_credential_request_pending;}
	bool is_challenge_complete();
	void set_credentials( QByteArray const& username, QByteArray const& response );
	
	// parse the contents of challenge header, return true if it is a valid HeroAuth challenge request
	bool		parse_authenticate_request( QNetworkReply* reply );
	QByteArray	get_challenge_to_digest(void);
	QByteArray	get_authorization_header(void);

	QNetworkAccessManager::Operation	get_operation(void) { return m_op; }
	QByteArray const&	get_post_data(void) { return m_post_payload; }
	void set_post_playload( QByteArray const& post_data ) { m_post_payload = post_data; }
};

// A QNetworkAccessManager subclass is handy for logging requests made by
// the browser and to have control over details of individual requests with
// regard to caching behavior, 404, 401 auth requests, custom http headers, etc.
class HeroNetworkAccessManager: public QNetworkAccessManager
{
	Q_OBJECT
public:
	HeroNetworkAccessManager(const char* language);

	HeroAuth	m_current_hero_auth;
	QByteArray	m_plaync_post_contents;	// hack for authorization on POST

	bool setSteamAuthSessionTicket(const QByteArray &steamSessionAuthTicket);
	void handleRedirect();

protected:
	virtual QNetworkReply *createRequest(Operation op, const QNetworkRequest &request, QIODevice *outgoingData = 0);
private slots:
//	void finishedSlot(QNetworkReply* reply);

private:
	QByteArray	m_language;				// locale language for browser (ISO 639-1 alpha-2 codes)
	QByteArray	m_SteamSessionAuthTicket;
	QByteArray	m_PrevSteamSessionAuthTicket;
};

// New Feature Bridge object
class NewFeature : public QObject
{
	Q_OBJECT

public slots:
	void Open(int featureID);

Q_SIGNALS:
	void newFeatureClicked(int featureID);
};

// Adapter for embedding a QtWebKit browser into City of Heroes
class WebKitAdapter : public QObject
{
	Q_OBJECT

public:
	WebKitAdapter(int width, int height, const char* language, const QString &registryBase);
	~WebKitAdapter();

	void load(const char* uri);
	void setHtml(const char* html);

	void* render(void);

	void mouseEvent( int type, int xloc, int yloc, int modifiers );
	void keyEvent( int type, int key_type, int key_val, int modifiers );
	void wheelEvent( int delta, int xloc, int yloc, int modifiers );

	void triggerAction( EHeroWebAction action );

	void activate(void);	// activate browser page (e.g. keyboard focus)
	void deactivate(void);

	bool authorize(char const* username, char const* response, char const* challenge);  // HeroAuth credentials

	bool setSteamAuthSessionTicket(const QByteArray &steamSessionAuthTicket);

	HeroWebPartnerCallback setPartner( HeroWebPartnerCallback partnerCallback );

private slots:

	void addJSObject(void);
	// page signal slots
	void loadStartedSlot();
	void loadProgressSlot(int progress);
	void loadFinishedSlot(bool ok);
//	void repaintRequestedSlot(const QRect& dirtyRect);
	void sceneChangedSlot(const QList<QRectF> & region);
	void linkHoveredSlot(const QString &link, const QString &title, const QString &textContent);
	void newFeatureClickedSlot(int id);

	// network access slots
	void net_finishedSlot(QNetworkReply* reply);
	void sslErrorsSlot(QNetworkReply* reply, const QList<QSslError>& errors);
	void authenticationRequiredSlot(QNetworkReply *reply, QAuthenticator *authenticator);

private:
	// N.B. Order is important here so that dependent object are constructed
	// and destructed in the correct sequence
	PersistentCookieJar			m_cookieJar;
	HeroNetworkAccessManager	m_netAccessManager;
//	QNetworkDiskCache			m_netDiskCache;
	QSize						m_pageSize;
	QImage						m_image;
	HeroWebView					m_view_web;
	QGraphicsView				m_view_scene;
	QGraphicsScene				m_scene;
	QWebPage					m_page;
	QString						m_language;			// locale language for browser (ISO 639-1 alpha-2 codes)
	HeroWebPartnerCallback		m_partnerCallback;
	NewFeature					* mp_newFeature;
};

//object for retrieving data through http
class WebDataRetriever : public QObject
{
	Q_OBJECT
public:
	WebDataRetriever(int (*partnerCallback)(void *));
	void getWebResetData(const char * url);
public slots:
	void finishedSlot(QNetworkReply * pReply);
private:
	QNetworkAccessManager m_netAccessManager;
	int (*m_partnerCallback)(void * data );
};

#endif // _WEBKITADAPTER_H__

