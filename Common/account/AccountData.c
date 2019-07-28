#include "AccountData.h"
#include "AccountCatalog.h"
#include "comm_backend.h"
#include "components/earray.h"
#include "entity/character_eval.h"
#include "utils/endian.h"
#include "utils/error.h"
#include "utils/eval.h"
#include "utils/SuperAssert.h"
#include "utils/textparser.h"
#include "utils/timing.h"
#include "utils/utils.h"

#ifdef CLIENT
#include "player/inventory_client.h"
#endif

#ifdef SERVER
#include "player/inventory_server.h"
#endif

#ifdef ACCOUNTSERVER
#include "account_inventory.h"
#endif

#ifndef ACCOUNTSERVER
#include "auth/authUserData.h"
#endif

// If you modify these functions, Please remember to modify:
//			passAccountServerInventoryDB in QueueServer_Passthrough.c
//			accountInventoryRelayToClient in accountservercomm.c
void AccountInventorySendItem(Packet * pak_out, AccountInventory *pInv)
{
	accountCatalogValidateSkuId(pInv->sku_id);

	pktSendBitsAuto2(pak_out, pInv->sku_id.u64);
	pktSendBitsAuto(pak_out, pInv->granted_total);
	pktSendBitsAuto(pak_out, pInv->claimed_total);
	pktSendBitsAuto(pak_out, pInv->expires);
}

void AccountInventoryReceiveItem(Packet * pak_in, AccountInventory *pInv)
{
	pInv->sku_id.u64 = pktGetBitsAuto2(pak_in);
	pInv->granted_total = pktGetBitsAuto(pak_in);
	pInv->claimed_total = pktGetBitsAuto(pak_in);
	pInv->expires = pktGetBitsAuto(pak_in);

	accountCatalogValidateSkuId(pInv->sku_id);
}

void AccountInventorySendReceive(Packet * pak_in, Packet * pak_out)
{
	pktSendGetBitsAuto2(pak_out, pak_in);// SKU
	pktSendGetBitsAuto(pak_out, pak_in); // granted_total
	pktSendGetBitsAuto(pak_out, pak_in); // claimed_total
	pktSendGetBitsAuto(pak_out, pak_in); // expires
}

void AccountInventoryCopyItem(AccountInventory *pInvSrc, AccountInventory *pInvDest)
{
	if (pInvSrc && pInvDest)
	{
		memcpy(pInvDest, pInvSrc, sizeof(AccountInventory));
	}
}

#if defined(SERVER) || defined(CLIENT)
bool AccountInventoryAddProductData(AccountInventory *pInv)
{
	const AccountProduct *pProduct = accountCatalogGetProduct(pInv->sku_id);
	if (!devassert(pProduct))
		return false;

	pInv->invType = pProduct->invType;
	pInv->recipe = pProduct->recipe;
	return true;
}
#endif

#ifndef QUEUESERVER
/////////////////////////////////////////////////////////
// Loyalty Rewards

static ParseTable parse_AccountLoyaltyRewardNode[] =  
{
	{ "Name",					TOK_STRING(LoyaltyRewardNode, name,0)									},
	{ "DisplayName",			TOK_STRING(LoyaltyRewardNode, displayName,0)							},
	{ "DisplayDescription",		TOK_STRING(LoyaltyRewardNode, displayDescription,0)						},
	{ "Product",				TOK_STRINGARRAY(LoyaltyRewardNode, storeProduct)						},
	{ "Repeatable",				TOK_BOOL(LoyaltyRewardNode, repeat, 1)									},
	{ "Purchasable",			TOK_BOOL(LoyaltyRewardNode, purchasable, 1)								},
	{ "VIPOnly",				TOK_BOOL(LoyaltyRewardNode, VIPOnly, 0)									},
	{ "End",					TOK_END																	},
	{ 0 },
};

static ParseTable parse_AccountLoyaltyRewardTier[] =  
{
	{ "Name",					TOK_STRING(LoyaltyRewardTier, name,0)									},
	{ "DisplayName",			TOK_STRING(LoyaltyRewardTier, displayName,0)							},
	{ "DisplayDescription",		TOK_STRING(LoyaltyRewardTier, displayDescription,0)						},
	{ "NextTier",				TOK_STRING(LoyaltyRewardTier, nextTier,0)								},
	{ "NodesRequiredForNext",	TOK_INT(LoyaltyRewardTier, nodesRequiredToUnlockNextTier, 0)			},
	{ "Node",					TOK_STRUCT(LoyaltyRewardTier, nodeList, parse_AccountLoyaltyRewardNode)	},
	{ "End",					TOK_END																	},
	{ 0 },
};

static ParseTable parse_AccountLoyaltyRewardLevel[] =  
{
	{ "Name",					TOK_STRING(LoyaltyRewardLevel, name,0)									},
	{ "DisplayName",			TOK_STRING(LoyaltyRewardLevel, displayName,0)							},
	{ "DisplayVIP",				TOK_STRING(LoyaltyRewardLevel, displayVIP,0)							},
	{ "DisplayFree",			TOK_STRING(LoyaltyRewardLevel, displayFree, 0)							},
	{ "Product",				TOK_STRINGARRAY(LoyaltyRewardLevel, storeProduct)						},
	{ "LoyaltyPointsRequired",	TOK_INT(LoyaltyRewardLevel, earnedPointsRequired, 0)					},
	{ "InfluenceCap",			TOK_INT64(LoyaltyRewardLevel, influenceCap, 0)							},
	{ "End",					TOK_END																	},
	{ 0 },
};

static ParseTable parse_AccountLoyaltyRewardNodeName[] = {
	{ "",	TOK_STRUCTPARAM | TOK_NO_TRANSLATE | TOK_STRING(LoyaltyRewardNodeName, name, 0)				},
	{ "",	TOK_STRUCTPARAM | TOK_INT(LoyaltyRewardNodeName, index, 0)									},
	{ "\n",	TOK_END,	0 },
	{ "", 0, 0 }
};

static ParseTable parse_AccountLoyaltyRewardTree[] =  
{
	{ "Tier",					TOK_STRUCT(LoyaltyRewardTree, tiers, parse_AccountLoyaltyRewardTier)		},
	{ "Level",					TOK_STRUCT(LoyaltyRewardTree, levels, parse_AccountLoyaltyRewardLevel)		},
	{ "NodeNames",				TOK_STRUCT(LoyaltyRewardTree, names, parse_AccountLoyaltyRewardNodeName)	},
	{ "DefaultInfluenceCap",	TOK_INT64(LoyaltyRewardTree, defaultInfluenceCap, 0)						},
	{ "VIPInfluenceCap",		TOK_INT64(LoyaltyRewardTree, VIPInfluenceCap, 0)							},
	{ "End",					TOK_END																		},
	{ 0 },
};

LoyaltyRewardTree g_accountLoyaltyRewardTree;

LoyaltyRewardTier *accountLoyaltyRewardFindTier(const char *tierName)
{
	int tierIndex;

	if (g_accountLoyaltyRewardTree.tiers == NULL || tierName == NULL)
		return NULL;

	for (tierIndex = 0; tierIndex < eaSize(&g_accountLoyaltyRewardTree.tiers); tierIndex++)
	{
		if (stricmp(tierName, g_accountLoyaltyRewardTree.tiers[tierIndex]->name) == 0)
			return g_accountLoyaltyRewardTree.tiers[tierIndex];
	}

	return NULL;
}

LoyaltyRewardNode *accountLoyaltyRewardFindNode(const char *nodeName)
{
	int tierIdx, nodeIdx;

	if (g_accountLoyaltyRewardTree.tiers == NULL || nodeName == NULL)
		return NULL;

	for (tierIdx = 0; tierIdx < eaSize(&(g_accountLoyaltyRewardTree.tiers)); tierIdx++)
	{
		LoyaltyRewardTier *tier = g_accountLoyaltyRewardTree.tiers[tierIdx];

		for (nodeIdx = 0; nodeIdx < eaSize(&tier->nodeList); nodeIdx++)
		{
			LoyaltyRewardNode *node = tier->nodeList[nodeIdx];

			if (stricmp(nodeName, node->name) == 0)	
				return node;
		}
	}
	return NULL;
}

LoyaltyRewardLevel *accountLoyaltyRewardFindLevel(const char *levelName)
{
	int levelIndex;

	if (g_accountLoyaltyRewardTree.levels == NULL || levelName == NULL)
		return NULL;

	for (levelIndex = 0; levelIndex < eaSize(&g_accountLoyaltyRewardTree.levels); levelIndex++)
	{
		if (stricmp(levelName, g_accountLoyaltyRewardTree.levels[levelIndex]->name) == 0)
			return g_accountLoyaltyRewardTree.levels[levelIndex];
	}

	return NULL;

}


LoyaltyRewardNode *accountLoyaltyRewardFindNodeByIndex(int index)
{
	int tierIdx, nodeIdx;

	if (g_accountLoyaltyRewardTree.tiers == NULL)
		return NULL;

	for (tierIdx = 0; tierIdx < eaSize(&(g_accountLoyaltyRewardTree.tiers)); tierIdx++)
	{
		LoyaltyRewardTier *tier = g_accountLoyaltyRewardTree.tiers[tierIdx];

		for (nodeIdx = 0; nodeIdx < eaSize(&tier->nodeList); nodeIdx++)
		{
			LoyaltyRewardNode *node = tier->nodeList[nodeIdx];

			if (node->index == index)	
				return node;
		}
	}
	return NULL;
}


int accountLoyaltyRewardTreeLoad()
{
	int retval;
	int tierIdx, nodeIdx, nameIdx;
	StructInit(&g_accountLoyaltyRewardTree, sizeof(LoyaltyRewardTree), parse_AccountLoyaltyRewardTree);
	retval = ParserLoadFiles(NULL, "defs/account/LoyaltyRewardTree.def", "LoyaltyReward.bin", 0, parse_AccountLoyaltyRewardTree, &g_accountLoyaltyRewardTree, NULL, NULL, NULL, NULL);

	// validate names and set indexes
	for (tierIdx = 0; tierIdx < eaSize(&(g_accountLoyaltyRewardTree.tiers)); tierIdx++)
	{
		LoyaltyRewardTier *tier = g_accountLoyaltyRewardTree.tiers[tierIdx];
		LoyaltyRewardTier *nextTier;

		for (nodeIdx = 0; nodeIdx < eaSize(&tier->nodeList); nodeIdx++)
		{
			LoyaltyRewardNode *node = tier->nodeList[nodeIdx];

			node->index = -1;

			for (nameIdx = 0; nameIdx < eaSize(&g_accountLoyaltyRewardTree.names); nameIdx++)
			{
				if (strcmp(g_accountLoyaltyRewardTree.names[nameIdx]->name, node->name) == 0)	
				{
					node->index = g_accountLoyaltyRewardTree.names[nameIdx]->index;
					node->tier = tier;
					break;
				}
			}

			if (node->index == -1)
			{
				Errorf("Warning: Loyalty reward %s does not have a matching index entry", node->name);
			}
		}

		// link previous - if it exist
		nextTier = accountLoyaltyRewardFindTier(tier->nextTier);
		if (nextTier != NULL)
			nextTier->previousTier = tier;
	
	}
	return retval;
}

int accountLoyaltyRewardHasThisNode(U8 *loyalty, LoyaltyRewardNode *node)
{
	return true;
}

bool accountLoyaltyRewardHasNode(U8 *loyalty, const char *nodeName)
{
	return true;
}

int accountLoyaltyRewardMasteredThisTier(U8 *loyalty, LoyaltyRewardTier *tier)
{
	return true;
}

int accountLoyaltyRewardMasteredTier(U8 *loyalty, const char *tierName)
{
	return true;
}


int accountLoyaltyRewardUnlockedThisTier(U8 *loyalty, LoyaltyRewardTier *tier)
{
	return true;
}


int accountLoyaltyRewardUnlockedTier(U8 *loyalty, const char *tierName)
{
	return true;
}

int accountLoyaltyRewardHasThisLevel(int pointEarned, LoyaltyRewardLevel *level)
{
	return true;
}

int accountLoyaltyRewardHasLevel(int pointEarned, const char *levelName)
{
	return true;
}

bool accountLoyaltyRewardCanBuyNode(AccountInventorySet* invSet, U32 accountStatusFlags, U8 *loyalty, U32 unspentLoyalty, LoyaltyRewardNode *node)
{
	return false;
}


int accountLoyaltyRewardGetNodesBought(U8 *loyalty)
{
	U64 * loyaltyQWords = (U64*)loyalty;
	unsigned i;
	unsigned count = 0;
	for(i=0; i<LOYALTY_BITS/sizeof(U64)/8; i+=1){
		U64 bits = loyaltyQWords[i];

		// this could be switched to use __popcnt64(), but it is not supported in vs2005
		while(bits) {
			count++;
			bits &= (bits - 1);
		}
	}
	return count;
}

/////////////////////////////////////////////////////////
// Loyalty Helper Functions

int AccountIsVIP(AccountInventorySet* invSet, U32 accountStatusFlags)
{
	return true;
}

int AccountIsVIPByPayStat(int payStat)
{
	return true;
}

int AccountIsPremiumByLoyalty(int loyalty)
{
	return true;
}

int AccountHasEmailAccess(AccountInventorySet* invSet, int pointsEarned, U32 accountStatusFlags)
{
	return true;
}

int AccountCanCreateSG(AccountInventorySet* invSet, int pointsEarned, U32 accountStatusFlags)
{
	return true;
}

int AccountCanWhisper(AccountInventorySet* invSet, int pointsEarned, U32 accountStatusFlags)
{
	return true;
}

int AccountCanPublicChannelChat(AccountInventorySet* invSet, int pointsEarned, U32 accountStatusFlags)
{
	return true;
}

int AccountCanCreatePublicChannel(AccountInventorySet* invSet, int pointsEarned, U32 accountStatusFlags)
{
	return true;
}

int AccountCanCreateHugeBody(AccountInventorySet* invSet, int pointsEarned, U32 accountStatusFlags)
{
	return true;
}

int AccountCanEarnMerits(AccountInventorySet* invSet, int pointsEarned, U32 accountStatusFlags)
{
	return true;
}

#if defined(CLIENT) || defined(SERVER) || defined(ACCOUNTSERVER) || defined(TEST_CLIENT)

#ifndef ACCOUNTSERVER
int AccountCanAccessGenderChange(AccountInventorySet* invSet, U32* auth_user_data, int access_level)
{

	if (authUserCanGenderChange(auth_user_data))
		return true;
	
	if (AccountHasStoreProduct(invSet, SKU("CUCPSSST")))
		return true;
	
	if (access_level >= 9) 
		return true;

	return false;
}
#endif

int AccountCanPlayMissionArchitect(AccountInventorySet* invSet, int pointsEarned, U32 accountStatusFlags)
{
	return AccountIsVIP(invSet, accountStatusFlags) || accountLoyaltyRewardHasLevel(pointsEarned, "Level2") || AccountHasStoreProduct(invSet, SKU("colimarc"));
}

// Exp, Tickets, Influence, Prestige
int AccountCanGainAllMissionArchitectRewards(AccountInventorySet* invSet, int pointsEarned, U32 accountStatusFlags)
{
	return AccountIsVIP(invSet, accountStatusFlags) || accountLoyaltyRewardHasLevel(pointsEarned, "Level4") || AccountHasStoreProduct(invSet, SKU("colimarc"));
}

int AccountCanPublishOnMissionArchitect(AccountInventorySet* invSet, int pointsEarned, U32 accountStatusFlags)
{
	return AccountIsVIP(invSet, accountStatusFlags) || accountLoyaltyRewardHasLevel(pointsEarned, "Level6") || AccountHasStoreProduct(invSet, SKU("colimarc"));
}

/////////////////////////////////////////////////////////
// Account Store 

bool AccountHasStoreProduct(AccountInventorySet* invSet, SkuId sku_id)
{
	return ( AccountGetStoreProductCount( invSet, sku_id, false ) > 0 );
}

bool AccountHasStoreProductOrIsPublished( AccountInventorySet * invSet, SkuId sku_id )
{
	return (AccountHasStoreProduct(invSet, sku_id) || accountCatalogIsSkuPublished(sku_id));
}

int AccountGetStoreProductCount(AccountInventorySet* invSet, SkuId sku_id, bool bActive)
{
	const AccountProduct* pProd = accountCatalogGetProduct(sku_id);
	return AccountGetStoreProductCount2( invSet, pProd, bActive );
}

int AccountGetStoreProductCount2(AccountInventorySet* invSet, const AccountProduct* pProd, bool bActive)
{
	int numOwned = 0;

	if (( invSet ) && ( pProd ))
	{
		AccountInventory* pInv = AccountInventorySet_Find(invSet, pProd->sku_id);
		if (( pInv ) && ( ! AccountInventoryHasProductExpired( pInv, pProd ) ))
		{
			int count = pInv->granted_total;
			if (bActive)
			{
				count -= pInv->claimed_total;
			}

			numOwned = count;
		}
	}

	if (( numOwned == 0 ) &&
		( pProd != NULL ) &&
		( pProd->productFlags & PRODFLAG_FREE_FOR_VIP ) &&
		( AccountIsVIP(invSet, invSet->accountStatusFlags)) &&
		( accountCatalogIsProductPublished(pProd) ))
	{
		//
		//	As a last resort, if the product is marked as free for VIPs and
		//	this user is a VIP, say that he owns one.
		//
		numOwned = 1;
	}

	return numOwned;
}

#if defined(SERVER) || (defined(CLIENT) && !defined(FINAL))
bool AccountStoreBuyProduct(U32 auth_id, SkuId sku_id, int quantity)
{
#if defined(CLIENT)
	return inventoryClient_BuyProduct(auth_id, sku_id, quantity);
#elif defined(SERVER)
	return inventoryServer_BuyProduct(auth_id, sku_id, quantity);
#endif
}

bool AccountStorePublishProduct(U32 auth_id, SkuId sku_id, bool bPublish)
{
#if defined(CLIENT)
	return inventoryClient_PublishProduct(auth_id, sku_id, bPublish);
#elif defined(SERVER)
	return inventoryServer_PublishProduct(auth_id, sku_id, bPublish);
#endif
}
#endif

static bool AccountInventoryIsExpiredAtTimeStamp( AccountInventory* pInv, const AccountProduct* prodInfo, U32 timeStamp )
{
	if ( prodInfo->expirationSecs == 0 )
	{
		return false;
	}
	else
	{
		return pInv->expires < timeStamp;
	}
}

bool AccountInventoryHasProductExpired( AccountInventory* pInv, const AccountProduct* prodInfo )
{
	return AccountInventoryIsExpiredAtTimeStamp( pInv, prodInfo, timerSecondsSince2000() );
}
#endif

/////////////////////////////////////////////////////////
// Account Requires

#if defined CLIENT || defined SERVER || defined TEST_CLIENT

static EvalContext *pAccountExpressionEvalContext = NULL;
static AccountInventorySet* s_pAccountExpressionCurrInv = NULL;

static void AccountExpression_StoreEval(EvalContext *pcontext)
{
	const char *productKey = eval_StringPop(pcontext);
	eval_IntPush(pcontext, AccountHasStoreProduct(s_pAccountExpressionCurrInv, skuIdFromString(productKey))); // HYBRID : Add check to see if account has this product or not
}

static void AccountExpression_LoyaltyEval(EvalContext *pcontext)
{
	const char *nodeName = eval_StringPop(pcontext);
	U8 *loyalty;

	eval_FetchPointer(pcontext, "Loyalty", &loyalty);
	eval_IntPush(pcontext, accountLoyaltyRewardHasNode(loyalty, nodeName));
}

static void AccountExpression_LoyaltyTierEval(EvalContext *pcontext)
{
	const char *tierName = eval_StringPop(pcontext);
	U8 *loyalty;

	eval_FetchPointer(pcontext, "Loyalty", &loyalty);
	eval_IntPush(pcontext, accountLoyaltyRewardUnlockedTier(loyalty, tierName));
}

static void AccountExpression_LoyaltyLevelEval(EvalContext *pcontext)
{
	const char *levelName = eval_StringPop(pcontext);
	int pointsEarned;

	eval_FetchInt(pcontext, "PointsEarned", &pointsEarned);
	eval_IntPush(pcontext, accountLoyaltyRewardHasLevel(pointsEarned, levelName));
}

static void AccountExpression_AuthEval(EvalContext *pcontext)
{
	const char *rhs; 
	U32 *authBits;
	bool bFound;

	rhs = eval_StringPop(pcontext);
	bFound = eval_FetchPointer(pcontext, "AuthBits", &authBits);

	if(bFound && rhs)
	{
		chareval_AuthFetchHelper(pcontext, authBits, rhs);
	}
	else
	{
		eval_IntPush(pcontext, 0);
	}
}

static void AccountExpression_IsReactivationActive(EvalContext *pcontext)
{
	eval_IntPush(pcontext, authUserIsReactivationActive());
}

int accountExpressionEval(AccountInventorySet* invSet, U8 *loyalty, int pointsEarned, U32* authBits, char **expr)
{
	s_pAccountExpressionCurrInv = invSet;
	
	if (pAccountExpressionEvalContext == NULL)
	{
		pAccountExpressionEvalContext = eval_Create();

		// initialize context
		eval_Init(pAccountExpressionEvalContext);
		eval_RegisterFunc(pAccountExpressionEvalContext, "ProductOwned?",			AccountExpression_StoreEval,			1, 1);
		eval_RegisterFunc(pAccountExpressionEvalContext, "StoreOwned?",				AccountExpression_StoreEval,			1, 1);
		eval_RegisterFunc(pAccountExpressionEvalContext, "LoyaltyOwned?",			AccountExpression_LoyaltyEval,			1, 1);
		eval_RegisterFunc(pAccountExpressionEvalContext, "LoyaltyTierOwned?",		AccountExpression_LoyaltyTierEval,		1, 1);
		eval_RegisterFunc(pAccountExpressionEvalContext, "LoyaltyLevelOwned?",		AccountExpression_LoyaltyLevelEval,		1, 1);
		eval_RegisterFunc(pAccountExpressionEvalContext, "auth>",					AccountExpression_AuthEval,				1, 1);
		eval_RegisterFunc(pAccountExpressionEvalContext, "isReactivationActive?",	AccountExpression_IsReactivationActive, 0, 1);
	}

	eval_ForgetPointer(pAccountExpressionEvalContext, "Loyalty");
	eval_StorePointer(pAccountExpressionEvalContext, "Loyalty", loyalty);
	eval_ForgetInt(pAccountExpressionEvalContext, "PointsEarned");
	eval_StoreInt(pAccountExpressionEvalContext, "PointsEarned", pointsEarned);
	eval_ForgetPointer(pAccountExpressionEvalContext, "AuthBits");
	eval_StorePointer(pAccountExpressionEvalContext, "AuthBits", authBits);
	eval_ClearStack(pAccountExpressionEvalContext);

	return eval_Evaluate(pAccountExpressionEvalContext, expr);
}

int accountEval(AccountInventorySet* invSet, U8 *loyalty, int pointsEarned, U32 *authBits, const char *requires)
{
	char		*requires_copy;
	char		*argv[100];
	StringArray expr;
	int			argc;
	int			i;
	int			result;

	if (invSet == NULL || loyalty == NULL || requires == NULL)
		return false;

	result = false;
	requires_copy = strdup(requires);
	argc = tokenize_line(requires_copy, argv, 0);
	if (argc > 0)
	{
		eaCreate(&expr);
		for (i = 0; i < argc; i++)
			eaPush(&expr, argv[i]);

		result = accountExpressionEval(invSet, loyalty, pointsEarned, authBits, expr);
		eaDestroy(&expr);
	}
	if (requires_copy)
		free(requires_copy);

	return result;
}
#endif
#endif

//---------------------------------------------------------------------------
//
//	AccountInventorySet - Public API
//
//---------------------------------------------------------------------------

static int cmp_AccountInventory(const AccountInventory** a, const AccountInventory** b)
{
	if ((*a)->sku_id.u64 == (*b)->sku_id.u64)
		return 0;
	return ((*a)->sku_id.u64 > (*b)->sku_id.u64) ? 1 : -1;
}

static int find_AccountInventory(const SkuId* key, const AccountInventory** value)
{
	if (key->u64 == (*value)->sku_id.u64)
		return 0;
	return (key->u64 > (*value)->sku_id.u64) ? 1 : -1;
}

#if defined(FULLDEBUG) && !defined(QUEUESERVER)
static void AccountInventorySet_validate(AccountInventorySet* invSet)
{
	int i;

	for (i=0; i<eaSize(&invSet->invArr); i++) {
		AccountInventory *inv = invSet->invArr[i];
		assert(inv);

		accountCatalogValidateSkuId(inv->sku_id);

		assert(inv->granted_total >=0);
		assert(inv->claimed_total >=0);

		if (i) {
			assert(cmp_AccountInventory(&invSet->invArr[i], &invSet->invArr[i-1]) == 1);
		}
	}
}
#else
#define AccountInventorySet_validate(invSet) do {} while(0)
#endif

void AccountInventorySet_InitMem( AccountInventorySet* invSet )
{
	memset(invSet, 0, sizeof(AccountInventorySet));
}

void AccountInventorySet_AddAndSort( AccountInventorySet* invSet, AccountInventory* item )
{
	int index = (int)eaBFind(invSet->invArr, cmp_AccountInventory, item);

	AccountInventorySet_validate(invSet);
	eaInsert(&invSet->invArr, item, index);
	AccountInventorySet_validate(invSet);
}

void AccountInventorySet_Sort( AccountInventorySet* invSet )
{
	eaQSort(invSet->invArr, cmp_AccountInventory);
	AccountInventorySet_validate(invSet);
}

#ifndef ACCOUNTSERVER
static void destroy_AccountInventory(AccountInventory* inv) {
	free(inv);
}

void AccountInventorySet_Release( AccountInventorySet* invSet )
{
	if (!invSet || !invSet->invArr)
		return;

	AccountInventorySet_validate(invSet);
	eaDestroyEx(&invSet->invArr, destroy_AccountInventory);
}
#endif

AccountInventory* AccountInventorySet_Find( AccountInventorySet* invSet, SkuId sku_id)
{
	AccountInventory** ret;
	AccountInventorySet_validate(invSet);
	ret = eaBSearch(invSet->invArr, find_AccountInventory, sku_id);
	return ret ? *ret : NULL;
}
