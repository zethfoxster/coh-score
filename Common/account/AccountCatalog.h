#ifndef ACCOUNTCATALOG_H
#define ACCOUNTCATALOG_H

#include "AccountData.h"

C_DECLARATIONS_BEGIN

typedef enum
{
	READINESS_NONE = 0,
	READINESS_CACHED = 1,
	READINESS_FULL = 2,
} AccountCatalogReadiness;

#if defined(SERVER)
typedef struct Entity Entity;
#endif

void accountCatalogInit(void);

#if defined(CLIENT) || defined(SERVER)
void accountCatalogRequest(void);
//void accountCatalogUpdate(SkuId sku_id, U32 stateFlags );
void accountCatalogDone(void);
#endif

void accountCatalogReportMissingSkuId(SkuId sku_id);

#if defined(FULLDEBUG) && !defined(DBSERVER) && !defined(QUEUESERVER)
void accountCatalogValidateSkuId(SkuId sku_id);
#else
#define accountCatalogValidateSkuId(sku_id) do {} while(0,0)
#endif

#if defined(CLIENT) || defined(SERVER)
bool accountCatalogIsUninitialized(void);
AccountCatalogReadiness accountCatalogIsReady(void);
bool accountCatalogIsEmpty(void);
bool accountCatalogIsOffline(void);
#endif

#if defined(SERVER)
bool accountCatalogServerFulfillCategory(SkuId sku_id, Entity *e, bool order_owner);
void accountCatalogServerAwardGlobalProducts(Entity *e);
#endif

void		accountCatalog_SetMtxEnvironment(const char* mtxEnvironment);
const char*	accountCatalog_GetMtxEnvironment();
void		accountCatalog_SetCatalogTimeStamp( U32 secsSince2000 );
U32			accountCatalog_GetCatalogTimeStamp( void );
void		accountCatalog_SetTimeStampTestOffsetSecs( U32 numSecs );
U32			accountCatalog_GetTimeStampTestOffsetSecs( void );


typedef enum
{
	STOREFLAG_NO_LOCALIZATION = ( 1 << 0 ),
} AccountOnlineStoreFlags;

typedef struct _AccountStoreAccessInfo
{
	// Memory management: 
	//   - Strings must be malloc'd memory. 
	//   - Call accountCatalog_ReleaseStoreAccessInfo() to release old values.
	const char* playSpanDomain;	
    const char* playSpanCatalog;
	const char* playSpanURL_Home;
	const char* playSpanURL_CategoryView;
	const char* playSpanURL_ItemView;
	const char* playSpanURL_ShowCart;
	const char* playSpanURL_AddToCart;
	const char* playSpanURL_ManageAccount;
	const char* playSpanURL_SupportPage;
	const char* playSpanURL_SupportPageDE;
	const char* playSpanURL_SupportPageFR;
	const char* playSpanURL_UpgradeToVIP;
	const char* cohURL_NewFeatures;
	const char* cohURL_NewFeaturesUpdate;
	//----------------------------
	U32			playSpanStoreFlags;	// AccountOnlineStoreFlags OR'd together
} AccountStoreAccessInfo;

const AccountStoreAccessInfo*	accountCatalog_GetStoreAccessInfo( void );
void					accountCatalog_ReleaseStoreAccessInfo( void );
void					accountCatalog_CacheAcctServerCatalogUpdate( Packet* pak_in );
void					accountCatalog_AddAcctServerCatalogToPacket( Packet* pak_out );

void					accountCatalog_RelayServerCatalogPacket( Packet* pak_in, Packet* pak_out );

bool accountCatalogIsProductAvailable(SkuId sku_id); // included, enabled, published
const char* accountCatalogGetTitle(SkuId sku_id);

bool accountCatalogIsSkuPublished(SkuId sku_id);
bool accountCatalogIsProductPublished(const AccountProduct* prod);
const char * accountCatalogGetProductTypeString(AccountInventoryType invType);

const AccountProduct* accountCatalogGetProduct(SkuId sku_id);
const AccountProduct* accountCatalogGetProductByRecipe(const char* recipe);

// Return an EArray with just the saleable products in the def file order and
// all their dynamic info as well (eg. currency, cost). Note that this points
// into the master copy of the catalog so changing the values is a bad idea.
const AccountProduct** accountCatalogGetEnabledProducts();

#ifdef SERVER
void accountCatalogGenerateServerBin(void);
#endif

C_DECLARATIONS_END

#endif
