
#ifndef _CMDCOMMON_ENUM_H
#define _CMDCOMMON_ENUM_H

enum
{
	CONCMD_THIRD = 1,
	CONCMD_FORWARD,
	CONCMD_BACKWARD,
	CONCMD_LEFT,
	CONCMD_RIGHT,
	CONCMD_UP,
	CONCMD_DOWN,
	CONCMD_NOCOLL,
	CONCMD_FIRST,
	CONCMD_NOTIMEOUT,
	CONCMD_FORWARD_MOUSECTRL,
	
	// This must be last!
	CONCMD_LASTCMD,
};

typedef enum ClientAccountCmd
{
	ClientAccountCmd_None = CONCMD_LASTCMD,
	ClientAccountCmd_ServerStatus,
	ClientAccountCmd_CsrSearchForAccount,
	ClientAccountCmd_CsrAccountInfo,
	ClientAccountCmd_BuyProductFromStore,
	ClientAccountCmd_PublishProduct,
	ClientAccountCmd_ShardXferSrc,
	ClientAccountCmd_ShardXferDst,
	ClientAccountCmd_ShardXferLink,
	ClientAccountCmd_ShardXferNoCharge,
	ClientAccountCmd_TokenInfo,
	ClientAccountCmd_SetAccountFlags,
	ClientAccountCmd_CsrAccountLoyaltyShow,
	ClientAccountCmd_CsrAccountProductShow,
	ClientAccountCmd_CsrAccountQueryInventory,
	ClientAccountCmd_CsrAccountLoyaltyRefund,
	ClientAccountCmd_CsrAccountProductChange,
	ClientAccountCmd_CsrAccountProductReconcile,
	//-------------------------------------
	ClientAccountCmd_Count,
} ClientAccountCmd;

typedef enum AccountCmdRes
{
	AccountCmdRes_None = ClientAccountCmd_Count,
	AccountCmdRes_Count,	
} AccountCmdRes;

enum
{
	SVCMD_TIME = AccountCmdRes_Count,
	SVCMD_TIMESCALE,
	SVCMD_TIMESTEPSCALE,
	SVCMD_DISABLE_DYNAMIC_COLLISIONS,
	SVCMD_PVPMAP,
	SVCMD_FOG_DIST,
	SVCMD_FOG_COLOR,
	
	// This must be last!
	SVCMD_LASTCMD,
};


#endif
