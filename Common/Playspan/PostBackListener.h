#ifndef POSTBACKLISTENER_H
#define POSTBACKLISTENER_H

C_DECLARATIONS_BEGIN

typedef struct PostbackAddress {
	U32 refCnt;
	U8 identity[255];
	U8 size;
} PostbackAddress;

typedef struct PostbackMessage {
	PostbackAddress *addr;	
	U32 refCnt;
	U8 data[8];
} PostbackMessage;

typedef struct PostBackRelay
{
	char *ip;
	int port;
} PostBackRelay;
typedef void (*PostbackCallback)(PostbackMessage * message, void * data, size_t size);

#define DEFAULT_PLAYSPAN_PARTNER_ID "NCSF"
#define DEFAULT_PLAYSPAN_CATALOG_ID DEFAULT_PLAYSPAN_PARTNER_ID

void postback_start(int ioThreadCount, char *mtxEnvironment, char *mtxSecretKey, char *playSpanDomain, PostBackRelay **relays, PostbackCallback callback);
void postback_stop();
void postback_reconfig();
bool postback_tick();
void postback_ack(PostbackMessage * message);
void postback_reconcile(U32 auth_id);
const char * postback_update_stats();

C_DECLARATIONS_END

#endif
