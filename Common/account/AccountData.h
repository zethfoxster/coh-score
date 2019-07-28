#ifndef ACCOUNTDATA_H
#define ACCOUNTDATA_H

#include "StashTable.h"
#include "AccountTypes.h"

C_DECLARATIONS_BEGIN

#define MAX_SLOTS_PER_SHARD		288

#define MAX_SLOTS_TOTAL			MAX_SLOTS_PER_SHARD*15

#define REFUND_TIME_OUT (3600*24*30) // 30 days

typedef struct AccountInventorySet AccountInventorySet;

//===========================================================================
//                           PRODUCT IDENTIFICATION
//
//	A product is uniquely defined by a combination of:
//		InventoryType +
//		InventorySubType (which may be 0) +
//		ItemKey (which may be NULL)
//
//	InventoryType		Used for implementing the class of benefit the product
//						provides and/or its mode of storage.
//	InventorySubType	Providing further classification within InventoryType.
//	ItemKey				A type-specific string completing the unique key for
//						a product. For Certifications, for example, this would
//						be the "recipe" name. Can be NULL if not applicable.
//						The string (if not empty) must be unique within the
//						name space of a type + subtype. 
//
//	ProductId			We give the combination InvType + InvSubType + ItemKey
//						an integer ProductId number, which can conveniently be 
//						passed around between the servers. ProductId's must be
//						globally unique.
//						To obtain the type, subtype, and itemkey for a ProductId
//						the program logic can look up the ProductId in the 
//						Product Catalog, and vice versa. 
//===========================================================================

// when changing, modify parse_AccountInventoryType in AccountDb.c
//					modify s_invValidators in account_inventory.c
typedef enum AccountInventoryType
{
	kAccountInventoryType_Certification,
	kAccountInventoryType_Voucher,
	kAccountInventoryType_PlayerInventory,
	kAccountInventoryType_Count,
} AccountInventoryType;

typedef enum AccountRequestType
{
	kAccountRequestType_CertificationGrant,
	kAccountRequestType_CertificationClaim,
	kAccountRequestType_Rename,
	kAccountRequestType_Respec,
	kAccountRequestType_Slot,
	kAccountRequestType_ShardTransfer,
	kAccountRequestType_MultiTransaction,
	kAccountRequestType_UnknownTransaction, // used when you aren't sure which of the above the request is
	kAccountRequestType_Count,
} AccountRequestType;

// These are authbits that products can turn on. They have to be specifically
// listed here - any other ones will be rejected by the validator.
typedef enum ProductAuthBit
{
	PRODUCT_AUTHBIT_INVALID				= 0,
	PRODUCT_AUTHBIT_ROGUE_ACCESS		= 1,
	PRODUCT_AUTHBIT_ROGUE_COMPLETE		= 2,
} ProductAuthBit;

typedef enum BillingPlanID
{
	BILLING_PLAN_INACTIVE			= 0,
	BILLING_PLAN_ONE_MONTH			= 1,
	BILLING_PLAN_THREE_MONTH		= 2,
	BILLING_PLAN_SIX_MONTH			= 3,
	BILLING_PLAN_TWELVE_MONTH		= 4,
	BILLING_PLAN_FOUR_MONTH			= 5,
	BILLING_PLAN_EIGHT_MONTH		= 6,
	BILLING_PLAN_SIXTEEN_MONTH		= 7,
	BILLING_PLAN_TWO_MONTH			= 8,
	BILLING_PLAN_ONE_WEEK			= 9,
	BILLING_PLAN_TIME_CARD			= 10,
	BILLING_PLAN_TRIAL				= 11,
	BILLING_PLAN_PROMO_1			= 12,
	BILLING_PLAN_PROMO_2			= 13,
	BILLING_PLAN_GUILD_WARS			= 14,
	BILLING_PLAN_BETA				= 15,
	BILLING_PLAN_PRESALE			= 16,
	BILLING_PLAN_GUEST_ACCOUNT		= 17,
	BILLING_PLAN_NOT_DETERMINED		= 19,	//NOTICE WE SKIPPED ONE!
	BILLING_PLAN_EXTERNAL_COMPED	= 97,	//NOTICE WE SKIPPED A LOT!
	BILLING_PLAN_INTERNAL_COMPED	= 98,
	BILLING_PLAN_GM_COMPED			= 99,
} BillingPlanID;

typedef enum AccountStatusFlags
{
	ACCOUNT_STATUS_FLAG_DEFAULT			= 0,
	ACCOUNT_STATUS_FLAG_IS_VIP			= 1<<0,	// for subscribers and others who get everything free
	ACCOUNT_STATUS_FLAG_DEBUG_VIP		= 1<<1,	// debug flag which doesn't get overwritten by loyalty update logic
} AccountStatusFlags;

typedef enum _AccountProductStateFlag
{
	PRODFLAG_NOT_PUBLISHED			= 1<<0,	// item is not ready to show to the user yet
	PRODFLAG_FREE_FOR_VIP			= 1<<2,
} AccountProductStateFlag;

typedef struct AccountProduct
{
	SkuId sku_id;
	AccountInventoryType invType;
	
	const char* recipe;		// type-specific unique string. e.g., for certifications, the recipe name.
	U32 productFlags;	// AccountProductDefFlag flags; static product definiton from local data file, sometimes updated on the fly

	const char* title;
	int invCount;
	int grantLimit;
	U32 expirationSecs;
	bool isGlobal;
	U32 authBit;
	U32 productStartDt;	// seconds since 2000
	U32 productEndDt;	// seconds since 2000
} AccountProduct;

/**
* @note Matches [inventory] in the SQL database
*/
typedef struct AccountInventory
{
	/// The SKU for this inventory record
	/// @note Matches [sku_id] in the SQL database
	SkuId sku_id;

	/// The total number of times this inventory type was purchased
	/// @note Matches [granted_total] in the SQL database
	long granted_total;

	/// The total number of times this this inventory type was claimed
	/// @note Matches [claimed_total] in the SQL database
	long claimed_total;

#if defined(ACCOUNTSERVER)
	/// The total number of times this this inventory type was correctly saved on the dbserver
	/// @note Matches [saved_total] in the SQL database
	long saved_total;
#endif

	/// Expiration date for this inventory type
	/// @note Matches [expires] in the SQL database
	U32 expires;

#if defined(CLIENT) || defined(SERVER)
	/// Recipe for this object
	const char * recipe;

	/// Copy of the @ref AccountInventoryType from the @ref AccountProduct
	AccountInventoryType invType;
#endif

} AccountInventory;

typedef struct AccountInventorySet {
	//
	//	Creating an inventory set -
	//		-	Method 1: 
	//				1. eaPush() the AccountInventory* objects on the invArr.
	//				2. Call AccountInventorySet_Sort() to index the entire array.
	//		-	Method 2:
	//				1. Call AccountInventorySet_AddAndSort() for each item you want to add.
	//
	//	Destroying an inventory set -
	//		-	Call AccountInventorySet_Release() to release the index data and.
	//
	//	Finding an item in an inventory set -
	//		-	***DO NOT*** use a for-loop! That technique won't scale as users potentially come
	//			to own dozens or even hundreds of products.
	//		-	DO use the inventory set lookup functions:
	//				AccountInventorySet_Find() - to find an AcccountInventory item.
	//
	AccountInventory**		invArr;

	/// @ref AccountStatusFlags for this inventory set
	U32	accountStatusFlags;
} AccountInventorySet;

typedef struct Packet Packet;
void AccountInventorySendItem(Packet * pak_out, AccountInventory *pInv);
void AccountInventoryReceiveItem(Packet * pak_in, AccountInventory *pInv);
void AccountInventorySendReceive(Packet * pak_in, Packet * pak_out);

/////////////////////////////////////////////////////////
// Loyalty Rewards

typedef struct LoyaltyRewardNode
{
	// Loaded from def file
	const char				*name;
	const char				*displayName;
	const char				*displayDescription;
	const char				**storeProduct;
	bool				repeat;
	bool				purchasable;
	bool				VIPOnly;

	// Not Persisted
	int		index;
	struct LoyaltyRewardTier	*tier;
} LoyaltyRewardNode;

typedef struct LoyaltyRewardTier
{
	// Loaded from def file
	LoyaltyRewardNode	**nodeList;
	const char				*name;
	const char				*displayName;
	const char				*displayDescription;
	const char				*nextTier;
	int					nodesRequiredToUnlockNextTier;

	// Not Persisted
	struct LoyaltyRewardTier	*previousTier;
} LoyaltyRewardTier;

typedef struct LoyaltyRewardLevel
{
	// Loaded from def file
	const char				*name;
	const char				*displayName;
	const char				*displayVIP;
	const char				*displayFree;
	int					    earnedPointsRequired;
	const char				**storeProduct;
	S64					influenceCap;
} LoyaltyRewardLevel;

typedef struct LoyaltyRewardNodeName
{
	const char				*name;
	int				    	index;
} LoyaltyRewardNodeName;

typedef struct LoyaltyRewardTree
{
	LoyaltyRewardTier		**tiers;
	LoyaltyRewardLevel		**levels;
	LoyaltyRewardNodeName	**names;
	S64						defaultInfluenceCap;
	S64						VIPInfluenceCap;
} LoyaltyRewardTree;

extern LoyaltyRewardTree g_accountLoyaltyRewardTree;

LoyaltyRewardNode *accountLoyaltyRewardFindNode(const char *nodeName);
LoyaltyRewardTier *accountLoyaltyRewardFindTier(const char *tierName);
LoyaltyRewardLevel *accountLoyaltyRewardFindLevel(const char *levelName);
LoyaltyRewardNode *accountLoyaltyRewardFindNodeByIndex(int index);

int accountLoyaltyRewardTreeLoad(void);
int accountLoyaltyRewardHasThisNode(U8 *loyalty, LoyaltyRewardNode *node);
bool accountLoyaltyRewardHasNode(U8 *loyalty, const char *nodeName);
bool accountLoyaltyRewardCanBuyNode(AccountInventorySet* invSet, U32 accountStatusFlags, U8 *loyalty, U32 unspentLoyalty, LoyaltyRewardNode *node);

int accountLoyaltyRewardUnlockedThisTier(U8 *loyalty, LoyaltyRewardTier *tier);
int accountLoyaltyRewardUnlockedTier(U8 *loyalty, const char *tierName);
int accountLoyaltyRewardMasteredThisTier(U8 *loyalty, LoyaltyRewardTier *tier);
int accountLoyaltyRewardMasteredTier(U8 *loyalty, const char *tierName);

int accountLoyaltyRewardHasLevel(int pointEarned, const char *levelName);
int accountLoyaltyRewardHasThisLevel(int pointEarned, LoyaltyRewardLevel *level);

int accountLoyaltyRewardGetNodesBought(U8 *loyalty);

/////////////////////////////////////////////////////////
// Loyalty Helper Functions

int AccountIsVIP(AccountInventorySet* invSet, U32 accountStatusFlags);
int AccountIsVIPByPayStat(int payStat);
int AccountIsPremiumByLoyalty(int loyalty);
int AccountHasEmailAccess(AccountInventorySet* invSet, int pointsEarned, U32 accountStatusFlags);
int AccountCanCreateSG(AccountInventorySet* invSet, int pointsEarned, U32 accountStatusFlags);
int AccountCanWhisper(AccountInventorySet* invSet, int pointsEarned, U32 accountStatusFlags);
int AccountCanPublicChannelChat(AccountInventorySet* invSet, int pointsEarned, U32 accountStatusFlags);
int AccountCanCreatePublicChannel(AccountInventorySet* invSet, int pointsEarned, U32 accountStatusFlags);
int AccountCanCreateHugeBody(AccountInventorySet* invSet, int pointsEarned, U32 accountStatusFlags);
int AccountCanEarnMerits(AccountInventorySet* invSet, int pointsEarned, U32 accountStatusFlags);

#if defined(CLIENT) || defined(SERVER) || defined(ACCOUNTSERVER) || defined(TEST_CLIENT)

#ifndef ACCOUNTSERVER
int AccountCanAccessGenderChange(AccountInventorySet* invSet, U32* auth_user_data, int access_level);
#endif

int AccountCanPlayMissionArchitect(AccountInventorySet* invSet, int pointsEarned, U32 accountStatusFlags);
int AccountCanGainAllMissionArchitectRewards(AccountInventorySet* invSet, int pointsEarned, U32 accountStatusFlags); // Exp, Tickets, Influence, Prestige
int AccountCanPublishOnMissionArchitect(AccountInventorySet* invSet, int pointsEarned, U32 accountStatusFlags);

/////////////////////////////////////////////////////////
// Account Store 
bool AccountHasStoreProduct( AccountInventorySet* invSet, SkuId sku_id);		// boolean (false for expired temporary items)
bool AccountHasStoreProductOrIsPublished( AccountInventorySet * invSet, SkuId sku_id );
int AccountGetStoreProductCount( AccountInventorySet* invSet, SkuId sku_id, bool bActive);	// 0 for expired temporary items
int AccountGetStoreProductCount2( AccountInventorySet* invSet, const AccountProduct* pProd, bool bActive );

#if defined(SERVER) || (defined(CLIENT) && !defined(FINAL))
		// Initiates a purchase transaction.
bool AccountStoreBuyProduct(U32 auth_id, SkuId sku_id, int quantity);
bool AccountStorePublishProduct(U32 auth_id, SkuId sku_id, bool bPublish);
#endif

		// Returns 'false' if the product has no expiration, or if the inventory item has not expired.
bool AccountInventoryHasProductExpired( AccountInventory* pInv, const AccountProduct* prodInfo );
#endif

void					AccountInventoryCopyItem(AccountInventory *pInvSrc, AccountInventory *pInvDest);

#if defined(SERVER) || defined(CLIENT)
bool					AccountInventoryAddProductData(AccountInventory *pInv);
#endif

						// Initializes memory pointed to by invSet.
void					AccountInventorySet_InitMem( AccountInventorySet* invSet );
						// An alternate strategy is to build invSet->invArr and then AccountInventorySet_BuildIndex(). 
void					AccountInventorySet_AddAndSort( AccountInventorySet* invSet, AccountInventory* item );
void					AccountInventorySet_Sort( AccountInventorySet* invSet );
void					AccountInventorySet_Release( AccountInventorySet* invSet );
AccountInventory*		AccountInventorySet_Find( AccountInventorySet* invSet, SkuId sku_id);

/////////////////////////////////////////////////////////
// Account Expression Evaluator

#if defined CLIENT || defined SERVER || defined TEST_CLIENT
int accountEval(AccountInventorySet* invSet, U8 *loyalty, int pointsEarned, U32 *authBits, const char *requires);
#endif

C_DECLARATIONS_END

#endif
