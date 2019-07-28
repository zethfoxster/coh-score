#ifndef UITRADELOGIC_H
#define UITRADELOGIC_H

#include "net_structdefs.h"
#include "powers.h"
#include "character_base.h"
#include "salvage.h"
#include "DetailRecipe.h"

int	 canTradeWithTarget(void *foo);
void tradeWithTarget(void *foo);

void trade_init( int partnerId );
void trade_cancel( char * reason );
void trade_sendUpdate( Packet * pak );
void trade_receiveUpdate( Packet *pak );
void trade_end( void );


typedef struct TradeObj
{
	int iCol; // also index
	int iRow; // also rarity

	int iLevel;
	int iNumCombines; //also count for salvage and charges for temp powers

	const BasePower * ppow;
	const SalvageItem *sal;
	const DetailRecipe *pRec;

} TradeObj;

typedef struct SalvageTrade
{
	const SalvageItem *sal;
	int count;
} SalvageTrade;

typedef struct RecipeTrade
{
	const DetailRecipe *pRec;
	int count;
} RecipeTrade;


#define SALV_TRADE_MAX 40 //totally arbitrary

// a - refers to player varibales
// b - refers to partner variables
typedef struct Trade 
{
	int partnerDbid;
	int aAccepted;
	int	bAccepted;

	int aInfluence;
	int bInfluence;

	int state;

	// screw it, this will just be static
	int aNumInsp;
	int aNumSpec;
	int aNumSalv;
	int aNumTpow;
	int aNumReci;

	int bNumInsp;
	int bNumSpec;
	int bNumSalv;
	int bNumTpow;
	int bNumReci;

	TradeObj	 aInsp[CHAR_INSP_MAX];
	const BasePower	 *bInsp[CHAR_INSP_MAX];

	TradeObj	 aSpec[CHAR_BOOST_MAX];
	Boost		 bSpec[CHAR_BOOST_MAX];

	TradeObj     aSalv[SALV_TRADE_MAX];
	SalvageTrade bSalv[SALV_TRADE_MAX];

	TradeObj	 aTpow[SALV_TRADE_MAX];
	Power		 bTpow[SALV_TRADE_MAX];

	TradeObj	 aReci[SALV_TRADE_MAX];
	RecipeTrade	 bReci[SALV_TRADE_MAX];

} Trade;


extern Trade trade;

#endif