#ifndef	_Exception_h_
#define	_Exception_h_

#define EXCEPTION
//#define SEH_EXCEPTION

#ifdef EXCEPTION
extern const char *mailServer;
extern LONG Terminating;

#ifdef SEH_EXCEPTION
#pragma warning(disable: 4509)
// nonstandard extension used: 'function' uses SEH and 'object' has destructor
int exception_filter(LPEXCEPTION_POINTERS pExp);
#define BEFORE	__try {
#define AFTER	} __except (exception_filter(GetExceptionInformation())) { \
				ELOG(" %s:%d(%s) exception\n", __FILE__, __LINE__, __TIMESTAMP__);
#define	FIN   }
#define _BEFORE	__try {
#define _AFTER	} __except (exception_filter(GetExceptionInformation())) { \
				ELOG("*%s:%d(%s) exception\n", __FILE__, __LINE__, __TIMESTAMP__);
#define _FIN 	send_exception(true); }
#define	AUTH_THROW_EXCEPTION	RaiseException(0xe0000001, 0, 0, 0)

#else // SEH_EXCEPTION

void trans_func( unsigned int u, _EXCEPTION_POINTERS* pExp );

class CSystemException
{
public:

// Constructor
	CSystemException( unsigned int n ) : m_nSE( n ) {}

// Attributes
	unsigned int m_nSE;

// Implementation
public:
	BOOL GetErrorMessage(LPSTR lpszError, UINT nMaxError,
			PUINT pnHelpContext = NULL);
};

#define BEFORE	try {
#define AFTER	} catch ( CSystemException* e ) { \
				char   szCause[255];\
				e->GetErrorMessage( szCause, 255 );\
				delete e;\
				ELOG(" %s:%d(%s) %s\n", __FILE__, __LINE__, __TIMESTAMP__, szCause);
#define	FIN   }
#define _BEFORE	_se_translator_function tr_func_org = _set_se_translator( trans_func ); \
				try {

#define _AFTER	} catch ( CSystemException* e ) { \
				char   szCause[255];\
				e->GetErrorMessage( szCause, 255 );\
				delete e;\
				ELOG( "*%s:%d(%s) %s\n",  __FILE__, __LINE__, __TIMESTAMP__, szCause);

#define _FIN 	send_exception(true);\
				} catch (...) {\
				ELOG( " %s:%d(%s) UNHANDLED\n", __FILE__, __LINE__, __TIMESTAMP__);\
				send_exception(true);\
				}\
				_set_se_translator( tr_func_org );
#define	AUTH_THROW_EXCEPTION	throw new CSystemException(0)
#endif // SEH_EXCEPTION

#define AFTER_FIN AFTER FIN
#define _AFTER_FIN _AFTER _FIN
#define CHECK_INIT int __check__ = __LINE__;
#define CHECK __check__ = __LINE__;
#define CHECK_LOG ELOG("Check %d\n", __check__);

void violation();
#define EASSERT(expr) if (!(expr)) violation()

void ELOG(LPCSTR lpszFormat, ...);
void send_exception(bool bExit);
void exception_init();

void	DumpStack();

#else 
#define BEFORE	{
#define _BEFORE	{
#define AFTER	} if (0) {
#define _AFTER	} if (0) {
#define FIN		}
#define FIN_BREAK	}
#define _FIN	}
#define AFTER_FIN	}	
#define _AFTER_FIN	}
#define CHECK_INIT
#define CHECK_LOG
#define CHECK
#define ELOG	(1) ? 0 : _ELOG
#define EASSERT(expr)
#define ETHROW
inline void _ELOG(LPCTSTR lpszFormat, ...) {}
inline void send_exception(bool bExit) {}
inline void exception_init() {}
#endif // EXCEPTION


#endif
