/***************************************************************************
 *     Copyright (c) 2005-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#ifndef AUTHUSERDATA_H__
#define AUTHUSERDATA_H__

#include "stdtypes.h"
#include "auth.h"

typedef enum
{
	PREORDER_NONE                = 0,
	PREORDER_ELECTRONICSBOUTIQUE = 1,
	PREORDER_GAMESTOP            = 2,
	PREORDER_BESTBUY             = 3,
	PREORDER_GENERIC             = 4,
} PreorderCode;

int authUserGetFieldByName(U32 *data, const char *field); // returns -1 on failure
bool authUserSetFieldByName(U32 *data, const char *field, int i); // returns false on failure
void authUserSetReactivationActive(int isActive);
int authUserIsReactivationActive();

#ifndef DBSERVER
#ifdef SERVER
#include "svr_base.h"
bool conShowAuthUserData(ClientLink *client);
bool conSetAuthUserData(ClientLink *client, char *pch, int i);
#else
bool conShowAuthUserData();
bool conSetAuthUserData(char *pch, int i);
#endif
#endif

extern U32 g_overridden_auth_bits[AUTH_DWORDS];

// these two functions have a granularity of 3 because of how they are stored
int  authUserMonthsPlayed(U32 *data);
void authUserSetMonthsPlayed(U32 *data, int i);

bool authUserHasRogueCompleteBox(U32 *data);
bool authUserHasLoyaltyReward(U32 *data);
bool authUserHasCon2009Bit1(U32 *data);
bool authUserHasCon2009Bit2(U32 *data);
bool authUserHasCon2009Bit3(U32 *data);
bool authUserHasCon2009Bit4(U32 *data);
bool authUserHasCon2009Bit5(U32 *data);
bool authUserHasMagicPack(U32 *data);
bool authUserHasSupersciencePack(U32 *data);
bool authUserHasMartialArtsPack(U32 *data);
bool authUserHasGRPreOrderSyndicateTechniques(U32 *data);
bool authUserHasCyborgPack(U32 *data);
bool authUserHasMacPack(U32 *data);
bool authUserHasIsRetailAccount(U32 *data);
bool authUserHasTargetPromo(U32 *data);
bool authUserHasPaxPromo(U32 *data);
bool authUserHasGenConPromo(U32 *data);
bool authUserHasComiconPromo(U32 *data);
bool authUserHasArachnosAccess(U32 *data);
bool authUserHasWeddingPack(U32 *data);
bool authUserHasTrialAccount(U32 *data);
bool authUserHasPocketDVIP(U32 *data);
bool authUserHasJumpPack(U32 *data);
bool authUserHasJetPack(U32 *data);
bool authUserHasKoreanLevelMinimum(U32 *data);
bool authUserHasValentine2006(U32 *data);
bool authUserHasCovBeta(U32 *data);
bool authUserHasCovPreorder1(U32 *data);
bool authUserHasCovPreorder2(U32 *data);
bool authUserHasCovPreorder3(U32 *data);
bool authUserHasCovPreorder4(U32 *data);
bool authUserHasCovSpecialEdition(U32 *data);
bool authUserHasVillainAccess(U32 *data);
PreorderCode authUserPreorderCode(U32 *data);
bool authUserHasDVDSpecialEdition(U32 *data);
bool authUserHasKheldian(U32 *data);
bool authUserHasBeta(U32 *data);
bool authUserHasTransferToEU(U32 *data);
bool authUserHasHeroAccess(U32 *data);
bool authUserCanGenderChange(U32*data);
bool authUserHasGRPreOrderResistanceTactics(U32 *data);
bool authUserHasGRPreOrderMightOfTheEmpire(U32 *data);
bool authUserHasGRPreOrderClockworkEfficiency(U32 *data);
bool authUserHasGRPreOrderWillOfTheSeers(U32 *data);
bool authUserHasRogueAccess(U32 *data);
bool authUserHasProvisionalRogueAccess(U32 *data); // has rogue access, a trial account, or reactivation is active
//bool authUserHasCreatePrimals(U32 *data);
bool authUserHasMutantPack(U32 *data);
bool authUserHasGhoulCostume(U32 *data);
bool authUserHasPraetoriaPDCostume(U32 *data);
bool authUserHasRiktiCostume(U32 *data);
bool authUserHasVahzilokMeatDocCostume(U32 *data);
bool authUserHasCoralaxBossCostume(U32 *data);
bool authUserHasRecreationPack(U32 *data);
bool authUserHasHolidayPack(U32 *data);
bool authUserHasAuraPack(U32 *data);
bool authUserHasVanguardPack(U32 *data);
bool authUserHasFacepalm(U32 *data);
bool authUserHasAnimalPack(U32 *data);
bool authUserHasLimitedRogueAccess(U32 *data);
bool authUserHasSteamPunk(U32 *data);

bool authUserHasPromo2011A(U32 *data);
bool authUserHasPromo2011B(U32 *data);
bool authUserHasPromo2011C(U32 *data);
bool authUserHasPromo2011D(U32 *data);
bool authUserHasPromo2011E(U32 *data);

bool authUserHasPromo2012A(U32 *data);
bool authUserHasPromo2012B(U32 *data);
bool authUserHasPromo2012C(U32 *data);
bool authUserHasPromo2012D(U32 *data);
bool authUserHasPromo2012E(U32 *data);

bool authUserHasAceInHoleA(U32 *data);
bool authUserHasAceInHoleB(U32 *data);
bool authUserHasAceInHoleC(U32 *data);
bool authUserHasAceInHoleD(U32 *data);
bool authUserHasAceInHoleE(U32 *data);


void authUserSetRogueCompleteBox(U32 *data, int i);
void authUserSetLoyaltyReward(U32 *data, int i);
void authUserSetCon2009Bit1(U32 *data, int i);
void authUserSetCon2009Bit2(U32 *data, int i);
void authUserSetCon2009Bit3(U32 *data, int i);
void authUserSetCon2009Bit4(U32 *data, int i);
void authUserSetCon2009Bit5(U32 *data, int i);
void authUserSetMagicPack(U32 *data, int i);
void authUserSetSupersciencePack(U32 *data, int i);
void authUserSetMartialArtsPack(U32 *data, int i);
void authUserSetGRPreOrderSyndicateTechniques(U32 *data, int i);
void authUserSetCyborgPack(U32 *data, int i);
void authUserSetMacPack(U32 *data, int i);
void authUserSetIsRetailAccount(U32 *data, int i);
void authUserSetTargetPromo(U32 *data, int i);
void authUserSetPaxPromo(U32 *data, int i);
void authUserSetGenConPromo(U32 *data, int i);
void authUserSetComiconPromo(U32 *data, int i);
void authUserSetArachnosAccess(U32 *data, int i);
void authUserSetWeddingPack(U32 *data, int i);
void authUserSetTrialAccount(U32 *data, int i);
void authUserSetPocketDVIP(U32 *data, int i);
void authUserSetJumpPack(U32 *data, int i);
void authUserSetJetPack(U32 *data, int i);
void authUserSetKoreanLevelMinimum(U32 *data, int i);
void authUserSetValentine2006(U32 *data, int i);
void authUserSetCovBeta(U32 *data, int i);
void authUserSetCovPreorder1(U32 *data, int i);
void authUserSetCovPreorder2(U32 *data, int i);
void authUserSetCovPreorder3(U32 *data, int i);
void authUserSetCovPreorder4(U32 *data, int i);
void authUserSetCovSpecialEdition(U32 *data, int i);
void authUserSetVillainAccess(U32 *data, int i);
void authUserSetPreorderCode(U32 *data, int i);
void authUserSetDVDSpecialEdition(U32 *data, int i);
void authUserSetKheldian(U32 *data, int i);
void authUserSetBeta(U32 *data, int i);
void authUserSetTransferToEU(U32 *data, int i);
void authUserSetHeroAccess(U32 *data, int i);
void authUserSetGRPreOrderClockworkEfficiency(U32 *data, int i);
void authUserSetGRPreOrderResistanceTactics(U32 *data, int i);
void authUserSetGRPreOrderMightOfTheEmpire(U32 *data, int i);
void authUserSetGRPreOrderWillOfTheSeers(U32 *data, int i);
void authUserSetRogueAccess(U32 *data, int i);
//void authUserSetCreatePrimals(U32 *data, int i);
void authUserSetMutantPack(U32 *data, int i);
void authUserSetGhoulCostume(U32 *data, int i);
void authUserSetPraetoriaPDCostume(U32 *data, int i);
void authUserSetRiktiCostume(U32 *data, int i);
void authUserSetVahzilokMeatDocCostume(U32 *data, int i);
void authUserSetCoralaxBossCostume(U32 *data, int i);
void authUserSetRecreationPack(U32 *data, int i);
void authUserSetHolidayPack(U32 *data, int i);
void authUserSetAuraPack(U32 *data, int i);
void authUserSetVanguardPack(U32 *data, int i);
void authUserSetFacepalm(U32 *data, int i);
void authUserSetAnimalPack(U32 *data, int i);
void authUserSetLimitedRogueAccess(U32 *data, int i);
void authUserSetSteamPunk(U32 *data, int i);

void authUserSetPromo2011A(U32 *data, int i);
void authUserSetPromo2011B(U32 *data, int i);
void authUserSetPromo2011C(U32 *data, int i);
void authUserSetPromo2011D(U32 *data, int i);
void authUserSetPromo2011E(U32 *data, int i);

void authUserSetPromo2012A(U32 *data, int i);
void authUserSetPromo2012B(U32 *data, int i);
void authUserSetPromo2012C(U32 *data, int i);
void authUserSetPromo2012D(U32 *data, int i);
void authUserSetPromo2012E(U32 *data, int i);

void authUserSetAceInHoleA(U32 *data, int i);
void authUserSetAceInHoleB(U32 *data, int i);
void authUserSetAceInHoleC(U32 *data, int i);
void authUserSetAceInHoleD(U32 *data, int i);
void authUserSetAceInHoleE(U32 *data, int i);

#ifdef DBSERVER
#define AUTH_USER_AUTHSERVER 0
#define AUTH_USER_DBSERVER 1 
void authUserMaskAuthBits(U8 *bitsToMask, int owner);
#endif

#endif /* #ifndef AUTHUSERDATA_H__ */

/* End of File */

