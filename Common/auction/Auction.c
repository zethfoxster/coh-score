/***************************************************************************
 *     Copyright (c) 2006-2007, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/

#include "Auction.h"
#include "estring.h"
#include "stringcache.h"
#include "net_packet.h"
#include "net_packetutil.h"
#include "textparser.h"
#include "utils.h"
#include "assert.h"
#include "error.h"
#include "mathutil.h"
#include "earray.h"
#include "MemoryPool.h"
#include "StashTable.h"
#include "trayCommon.h"

// AuctionInvItemStatusEnum //////////////////////////////////////////////////

StaticDefineInt AuctionInvItemStatusEnum [] =
{
	DEFINE_INT
	{"None",AuctionInvItemStatus_None},
	{"Stored",AuctionInvItemStatus_Stored}, 
	{"ForSale",AuctionInvItemStatus_ForSale},
	{"Sold",AuctionInvItemStatus_Sold},
	{"Bidding",AuctionInvItemStatus_Bidding},
	{"Bought",AuctionInvItemStatus_Bought},
	DEFINE_END
};
STATIC_ASSERT(ARRAY_SIZE(AuctionInvItemStatusEnum)==AuctionInvItemStatus_Count+2);
bool auctioninvitemstatus_Valid( AuctionInvItemStatus e )
{
	return INRANGE0(e, AuctionInvItemStatus_Count);
}
 
const char *AuctionInvItemStatus_ToStr(AuctionInvItemStatus status)
{
	return StaticDefineIntRevLookup(AuctionInvItemStatusEnum,status);	
}


// AuctionInvItem ////////////////////////////////////////////////////////////

TokenizerParseInfo parse_AuctionInvItem[] = 
{
	{ "AucInvItem",		TOK_START,									0},
	{ "ID",				TOK_INT(AuctionInvItem,id,0),				0},
	{ "Item",			TOK_POOL_STRING|TOK_STRING(AuctionInvItem,pchIdentifier,0),	0},
	{ "Status",			TOK_INT(AuctionInvItem,auction_status,0),	AuctionInvItemStatusEnum},
	{ "StoredCount",	TOK_INT(AuctionInvItem,amtStored,0),		0},
	{ "StoredInf",		TOK_INT(AuctionInvItem,infStored,0),		0},
	{ "OtherCount",		TOK_INT(AuctionInvItem,amtOther,0),			0},
	{ "Price",			TOK_INT(AuctionInvItem,infPrice,0),			0},
	{ "MapSideIdx",		TOK_INT(AuctionInvItem,iMapSide,0),			0},
	{ "DeleteMe",		TOK_BOOL(AuctionInvItem,bDeleteMe,false),	0},
	{ "MergedBid",		TOK_BOOL(AuctionInvItem,bMergedBid,false),	0},
	{ "CancelledCount",	TOK_INT(AuctionInvItem,amtCancelled,0),		0},
	{ "End",			TOK_END,			0},
	{ "", 0, 0 }
};

static AuctionInvItem* AuctionInvItem_CreateRaw(void)
{
	AuctionInvItem *res = StructAllocRaw(sizeof( AuctionInvItem ));
	assert(res);
	return res;
}

AuctionInvItem* AuctionInvItem_Create(char *pchIdentifier)
{
	AuctionInvItem *res = AuctionInvItem_CreateRaw();
	res->pchIdentifier = allocAddString(pchIdentifier);
	return res;
}

void AuctionInvItem_Destroy( AuctionInvItem *hItem )
{
	if(hItem)
		StructDestroy(parse_AuctionInvItem,hItem);
}

bool AuctionInvItem_ToStr(AuctionInvItem *itm, char **hestr)
{
	if(!itm||!hestr)
		return FALSE;
	if(*hestr)
		estrClear(hestr);
	return ParserWriteTextEscaped(hestr,parse_AuctionInvItem,itm,0,0);
}

bool AuctionInvItem_FromStr(AuctionInvItem **hitm, char *str)
{
	if(!hitm || !str)
		return false;
	if(!*hitm)
		*hitm = StructAllocRaw(sizeof(**hitm));
	return ParserReadTextEscaped(&str,parse_AuctionInvItem,*hitm);
} 

AuctionInvItem* AuctionInvItem_Clone(AuctionInvItem **hdest, AuctionInvItem *itm)
{
	if(!itm||!hdest)
		return NULL;
	if(!(*hdest))
		(*hdest) = AuctionInvItem_CreateRaw();
	if(!StructCopyAll(parse_AuctionInvItem,itm,sizeof(*itm),*hdest))
	{
		AuctionInvItem_Destroy(*hdest);
		(*hdest) = NULL;
	}
	return *hdest;
}

TokenizerParseInfo parse_AuctionInventory[] = 
{
	{ "InventorySize",	TOK_INT(AuctionInventory,invSize,0),						0},
	{ "Items",			TOK_STRUCT(AuctionInventory,items,parse_AuctionInvItem),	0},
	{ "End",			TOK_END,													0},
	{ "", 0, 0 }
};

TokenizerParseInfo ParseAuctionConfig[] =
{
	{ "AuctionConfig",	TOK_IGNORE, 0 },
	{ "{",				TOK_START,  0 },
	{ "SellFeePercent",	TOK_F32(AuctionConfig, fSellFeePercent, 0.0f)     },
	{ "BuyFeePercent",	TOK_F32(AuctionConfig, fBuyFeePercent, 0.0f)     },
	{ "MinFee",			TOK_INT(AuctionConfig, iMinFee, 0.0f)     },
	{ "}",				TOK_END,    0 },
	{ "", 0, 0 }
};

SHARED_MEMORY AuctionConfig g_AuctionConfig = {0};

void AuctionInventory_Send(AuctionInventory *inv, Packet *pak)
{
 	char *s = NULL;
	AuctionInventory_ToStr(inv,&s);
	pktSendString(pak,s);
	estrDestroy(&s);
}

void AuctionInventory_Recv(AuctionInventory **hinv, Packet *pak)
{
	char *s;
	
	if(!devassert(hinv) || !pak)
		return;
	
	if( !(*hinv) )
		(*hinv) = AuctionInventory_Create();
	s = pktGetString(pak);
	AuctionInventory_FromStr(hinv,s);
}

bool auctionservertype_Valid( AuctionServerType e )
{
	return (e == kAuctionServerType_Heroes) ? true : false;
}

StaticDefineInt AuctionServerTypeEnum[] =
{
	DEFINE_INT
	{"Heroes/Villains",		kAuctionServerType_Heroes},
	{"Deprecated",			kAuctionServerType_Villains},
	DEFINE_END
};
STATIC_ASSERT(ARRAY_SIZE( AuctionServerTypeEnum ) == kAuctionServerType_Count+2);

const char *AuctionServerType_ToStr( AuctionServerType s)
{
	return StaticDefineIntRevLookup(AuctionServerTypeEnum,s); 
}

AuctionInventory* AuctionInventory_Create(void)
{
	AuctionInventory *res = NULL;
	res = StructAllocRaw(sizeof( AuctionInventory ));
	if( devassert( res ))
	{
		//res->invSize = AUCTION_INV_DEFAULT_SIZE;
	}
	return res;
}

void AuctionInventory_Destroy( AuctionInventory *hItem )
{
	if(hItem)
		StructDestroy(parse_AuctionInventory,hItem);
}


void setAuctionHistoryItemDetail(AuctionHistoryItemDetail *dest, char *buyer, char *seller, U32 date, int price)
{
	if(dest->buyer)
		StructFreeString(dest->buyer);
	dest->buyer = buyer ? StructAllocString(buyer) : NULL;
	if(dest->seller)
		StructFreeString(dest->seller);
	dest->seller = seller ? StructAllocString(seller) : NULL;
	dest->date = date;
	dest->price = price;
}


AuctionHistoryItemDetail **allocatedHistoryItemDetails = NULL;

AuctionHistoryItemDetail *newAuctionHistoryItemDetail(char *buyer, char *seller, U32 date, int price)
{
	AuctionHistoryItemDetail *ret = NULL;

	if ( allocatedHistoryItemDetails && eaSize(&allocatedHistoryItemDetails) )
		ret = eaPop(&allocatedHistoryItemDetails);
	else
		ret = StructAllocRaw(sizeof(AuctionHistoryItemDetail));

	if ( ret )
		setAuctionHistoryItemDetail(ret, buyer, seller, date, price);

	return ret;
}

void freeAuctionHistoryItemDetail(AuctionHistoryItemDetail *itm)
{
	if ( itm )
	{
		if(itm->buyer)
			StructFreeString(itm->buyer);
		if(itm->seller)
			StructFreeString(itm->seller);
		eaPush(&allocatedHistoryItemDetails, itm);
	}
}

AuctionHistoryItem *newAuctionHistoryItem(const char *pchIdentifier)
{
	AuctionHistoryItem *ret = NULL;
	ret = StructAllocRaw(sizeof(AuctionHistoryItem));
	ret->pchIdentifier = allocAddString(pchIdentifier);
	return ret;
}

void freeAuctionHistoryItem(AuctionHistoryItem *itm)
{
	if(!itm)
		return;

	eaDestroyEx(&itm->histories,freeAuctionHistoryItemDetail);
	StructFree(itm);
}

void AuctionInventory_ToStr(AuctionInventory *inv, char **hestr)
{
	if(!inv||!hestr)
		return;
	ParserWriteTextEscaped(hestr,parse_AuctionInventory,inv,0,0);
}

void AuctionInventory_FromStr(AuctionInventory **hinv,char *str)
{
	if(!str||!hinv)
		return;
	if(*hinv)
		AuctionInventory_Destroy(*hinv);
	*hinv = AuctionInventory_Create();
	ParserReadTextEscaped(&str,parse_AuctionInventory,(*hinv));
}

INT64 calcAuctionSellFee(INT64 price)
{
	return max(g_AuctionConfig.iMinFee, price * g_AuctionConfig.fSellFeePercent);
}

int calcAuctionSoldFee(int price, int origPrice, int amt, int amtCancelled)
{
	INT64 fee = price * g_AuctionConfig.fBuyFeePercent - (calcAuctionSellFee(origPrice) * amt);
	if (price)
	{
		fee = max(g_AuctionConfig.iMinFee, fee);
	}

	if (amtCancelled)
	{
		fee -= calcAuctionSellFee(origPrice) * amtCancelled;
	}

	return fee;
}

// hooray for hardcoding crap.
// TODO: write new auctionserver :(
static const char *generic_enhs[] = {
	"Crafted_Damage",
	"Crafted_Recharge",
	"Crafted_Accuracy",
	"Crafted_Endurance_Discount",
	"Crafted_Heal",
	"Crafted_Recovery",
	"Crafted_Res_Damage",
	"Crafted_Defense_Buff",
	"Crafted_Run",
	"Crafted_Fly",
	"Crafted_Jump",
	"Crafted_Range",
	"Crafted_ToHit_Buff",
	"Crafted_Hold",
	"Crafted_ToHit_DeBuff",
	"Crafted_Snare",
	"Crafted_Stun",
	"Crafted_Taunt",
	"Crafted_Immobilize",
	"Crafted_Defense_DeBuff",
	"Crafted_Knockback",
	"Crafted_Interrupt",
	"Crafted_Fear",
	"Crafted_Sleep",
	"Crafted_Confuse",
	0
};

static const char *salvage_base[] = {
	"S_BrainStormIdea",
	// Base
	"S_AbberantTech",
	"S_AeonTech",
	"S_Amulet",
	"S_AncestralWeapon",
	"S_AncientWeapon",
	"S_ArachnoidBlood",
	"S_ArachnoidVenom",
	"S_ArachnosGun",
	"S_ArcanePowder",
	"S_ArmorShard",
	"S_AstrologyBook",
	"S_BabbageEngine",
	"S_BioSample",
	"S_BlackBox",
	"S_BlackMarketSuperGear",
	"S_BlasterTech",
	"S_BloodSample",
	"S_BodyArmorFragment",
	"S_BoneFragment",
	"S_BookOfBlood",
	"S_Bracer",
	"S_BrokenCreyPistol",
	"S_BrokenCreyRifle",
	"S_BrokenMask",
	"S_BrokenRiktiWeapon",
	"S_Carapace",
	"S_CeramicCompound",
	"S_Charms",
	"S_CIAFiles",
	"S_Claw",
	"S_Cloak",
	"S_Coins",
	"S_CommunicationDevice",
	"S_CoralaxBlood",
	"S_CoralShard",
	"S_CortexDevice",
	"S_CreyTech",
	"S_CrystalCodexFragments",
	"S_CrystalSkull",
	"S_CyberneticCharger",
	"S_CyberneticImplant",
	"S_DaemonHeart",
	"S_DaemonicChain",
	"S_Dagger",
	"S_DangerousChemicals",
	"S_DataFiles",
	"S_Demonlogica",
	"S_DevouringCulture",
	"S_DNAMutation",
	"S_DreadTramalian",
	"S_EbonClaw",
	"S_EChip",
	"S_Ectoplasm",
	"S_EctoplasmicResidue",
	"S_ElementalManual",
	"S_EnchantedWeapon",
	"S_EnergySource",
	"S_Etherium",
	"S_ExoticAlloy",
	"S_ExoticCompound",
	"S_FaLakAmulet",
	"S_Fetish",
	"S_FirBolgStraw",
	"S_FishScale",
	"S_FreakshowCybernetics",
	"S_Furs",
	"S_GeneticSample",
	"S_GhostTrinket",
	"S_Glittershrooms",
	"S_GoldPlatedSkin",
	"S_GraphiteComposite",
	"S_HamidonLichen",
	"S_HamidonSpore",
	"S_Hatchet",
	"S_Headset",
	"S_HellishTooth",
	"S_Hex",
	"S_IceShard",
	"S_InnovativeCode",
	"S_KheldianBloodSample",
	"S_KheldianTech",
	"S_Lens",
	"S_LevDisk",
	"S_LudditePouch",
	"S_Magma",
	"S_MeatCleaver",
	"S_MercuryCircuits",
	"S_MilitaryIntelligence",
	"S_MutatedSample",
	"S_Nanites",
	"S_NecklaceofTeeth",
	"S_NemesisStaff",
	"S_NemesisWeapon",
	"S_NeoOrganics",
	"S_NictusAmmo",
	"S_NictusMemento",
	"S_NictusTech",
	"S_Notes",
	"S_Orichalcum",
	"S_ParagonPoliceFiles",
	"S_Pelt",
	"S_PhantomTears",
	"S_PortalTech",
	"S_Potions",
	"S_PoweredArmorCircuitry",
	"S_PsionicReceptacle",
	"S_PsionizedMetal",
	"S_Pumice",
	"S_PumpkinBomb",
	"S_PumpkinShard",
	"S_RedcapPouch",
	"S_RiktiArmorFragment",
	"S_RiktiCommunicationsDevice",
	"S_RiktiControlUnit",
	"S_RiktiDataPad",
	"S_RiktiPlasmaRifle",
	"S_RiktiPlasmaWeapon",
	"S_Ring",
	"S_RylehsRain",
	"S_Saber",
	"S_SaintsMedallion",
	"S_ScentofBrimstone",
	"S_Scrap",
	"S_Scrolls",
	"S_Shillelagh",
	"S_ShivanEctoplasm",
	"S_Sigil",
	"S_SkyRaiderAntiGravUnit",
	"S_SkyRaiderDevice",
	"S_SkyRaiderWeapon",
	"S_SlagCulture",
	"S_SnakeFang",
	"S_SnakeVenom",
	"S_Spark",
	"S_Spells",
	"S_SpiderEye",
	"S_SpielmansSignet",
	"S_SpiritGuide",
	"S_StableProtonium",
	"S_StaticCharge",
	"S_SteamTechImplant",
	"S_SuperadineExtract",
	"S_SyntheticDrug",
	"S_SyntheticOrgans",
	"S_Talisman",
	"S_Tatoo",
	"S_ThornFragment",
	"S_TissueSample",
	"S_Titanium",
	"S_ToolsoftheTrade",
	"S_Treatise",
	"S_TrickArrows",
	"S_TsooInk",
	"S_UrMetal",
	"S_VacuumCircuits",
	"S_VerminousVictuals",
	"S_WitchsHat",
	"S_SpellsOfPower",
	"S_UnstableRadPistol",
	"S_WeaponOfMu",
	"S_AlienTech",
	"S_NanoFluid",
	// Component
	"S_ArcaneEssence",
	"S_ArcaneGlyph",
	"S_ExperimentalTech",
	"S_MagicalArtifact",
	"S_MagicalWard",
	"S_MysticElement",
	"S_MysticFoci",
	"S_TechHardware",
	"S_TechMaterial",
	"S_TechPower",
	"S_TechPrototype",
	"S_TechSoftware",
	// Mission
	"S_GraveDirtLostCure",
	"S_Hero1BloodLostCure",
	"S_LostSerumLostCure",
	"S_AzuriasWandLostCure",
	"S_CoTIncantationLostCure",
	0
};

static const char *salvage_common[] = {
	"S_HumanBloodSample",
	"S_InanimateCarbonRod",
	"S_ComputerVirus",
	"S_SimpleChemical",
	"S_Brass",
	"S_Boresight",
	"S_ImprovisedCybernetic",
	"S_InertGas",
	"S_ScientificTheory",
	"S_CircuitBoard",
	"S_StabilizedMutantGenome",
	"S_Iron",
	"S_KineticWeapon",
	"S_TemporalAnalyzer",
	"S_CeramicArmorPlate",
	"S_HydraulicPiston",
	"S_Silver",
	"S_MathematicProof",
	"S_AncientArtifact",
	"S_LuckCharm",
	"S_ClockworkWinder",
	"S_SpellScroll",
	"S_SpiritualEssence",
	"S_RuneboundArmor",
	"S_MasterworkWeapon",
	"S_Rune",
	"S_DemonicBloodSample",
	"S_AlchemicalSilver",
	"S_AncientBone",
	"S_SpellInk",
	"S_NevermeltingIce",
	"S_Fortune",
	"S_Ruby",
	"S_DemonicThreatReport",
	"S_RegeneratingFlesh",
	"S_SpiritThorn",
	0
};

static const char *salvage_uncommon[] = {
	"S_MutantBloodSample",
	"S_HeavyWater",
	"S_DaemonProgram",
	"S_ChemicalFormula",
	"S_Polycarbon",
	"S_Scope",
	"S_CommercialCybernetic",
	"S_CorrosiveGas",
	"S_ScientificLaw",
	"S_DataDrive",
	"S_MutantDNAStrand",
	"S_Steel",
	"S_EnergyWeapon",
	"S_TemporalTracer",
	"S_TitaniumShard",
	"S_PneumaticPiston",
	"S_Gold",
	"S_ChaosTheorem",
	"S_UnearthedRelic",
	"S_TemporalSands",
	"S_ClockworkGear",
	"S_VolumeoftheObsidianLibrum",
	"S_PsionicEctoplasm",
	"S_SoulboundArmor",
	"S_EnscorcelledWeapon",
	"S_Symbol",
	"S_BloodoftheIncarnate",
	"S_AlchemicalGold",
	"S_CarnivalofShadowsMask",
	"S_LivingTattoo",
	"S_UnquenchableFlame",
	"S_Destiny",
	"S_Sapphire",
	"S_PsionicThreatReport",
	"S_BleedingStone",
	"S_ThornTreeVine",
	0
};

static const char *salvage_rare[] = {
	"S_AlienBloodSample",
	"S_EnrichedPlutonium",
	"S_SourceCode",
	"S_ComplexChemicalFormula",
	"S_PlasmaCapacitor",
	"S_HeadsUpDisplay",
	"S_MilitaryCybernetic",
	"S_ReactiveGas",
	"S_ConspiratorialEvidence",
	"S_HolographicMemory",
	"S_MutatingGenome",
	"S_Impervium",
	"S_PhotonicWeapon",
	"S_ChronalSkip",
	"S_RiktiAlloy",
	"S_PositronicMatrix",
	"S_Platinum",
	"S_SyntheticIntelligenceUnit",
	"S_LamentBox",
	"S_StrandofFate",
	"S_PsioniclyChargedBrass",
	"S_PagefromtheMalleusMundi",
	"S_PsionicManfestation",
	"S_SymbioticArmor",
	"S_DeificWeapon",
	"S_EmpoweredSigil",
	"S_BlackBloodoftheEarth",
	"S_EnchantedImpervium",
	"S_MuVestment",
	"S_SoulTrappedGem",
	"S_PangeanSoil",
	"S_Prophecy",
	"S_Diamond",
	"S_MagicalConspiracy",
	"S_EssenceoftheFuries",
	"S_HamidonGoo",
	// PVP
	"S_AccessBypass_MM",
	"S_PrototypeElement_MM",
	0
};

static StashTable s_DoNotGroupPrefixes = 0;
static StashTable s_SalvageGroupBase = 0;
static StashTable s_SalvageGroupCommon = 0;
static StashTable s_SalvageGroupUncommon = 0;
static StashTable s_SalvageGroupRare = 0;

static void SetupPrefixes()
{
	char buf[256];
	const char **s;

	s_DoNotGroupPrefixes = stashTableCreateWithStringKeys(32, StashDeepCopyKeys);
	s = generic_enhs;
	while (s && *s) {
		sprintf_s(SAFESTR(buf), "%d Boosts.%s.%s", kTrayItemType_SpecializationInventory, *s, *s);
		stashAddInt(s_DoNotGroupPrefixes, buf, 1, false);
		s++;
	}

	s_SalvageGroupBase = stashTableCreateWithStringKeys(32, StashDeepCopyKeys);
	s = salvage_base;
	while (s && *s) {
		sprintf_s(SAFESTR(buf), "%d %s 0", kTrayItemType_Salvage, *s);
		stashAddInt(s_SalvageGroupBase, buf, 1, false);
		s++;
	}

	s_SalvageGroupCommon = stashTableCreateWithStringKeys(32, StashDeepCopyKeys);
	s = salvage_common;
	while (s && *s) {
		sprintf_s(SAFESTR(buf), "%d %s 0", kTrayItemType_Salvage, *s);
		stashAddInt(s_SalvageGroupCommon, buf, 1, false);
		s++;
	}

	s_SalvageGroupUncommon = stashTableCreateWithStringKeys(32, StashDeepCopyKeys);
	s = salvage_uncommon;
	while (s && *s) {
		sprintf_s(SAFESTR(buf), "%d %s 0", kTrayItemType_Salvage, *s);
		stashAddInt(s_SalvageGroupUncommon, buf, 1, false);
		s++;
	}

	s_SalvageGroupRare = stashTableCreateWithStringKeys(32, StashDeepCopyKeys);
	s = salvage_rare;
	while (s && *s) {
		sprintf_s(SAFESTR(buf), "%d %s 0", kTrayItemType_Salvage, *s);
		stashAddInt(s_SalvageGroupRare, buf, 1, false);
		s++;
	}
}

// Finds all instances of oldstr in src and replaces them all with newstr in buf.
// Does not check for buffer overruns, so always replace a longer bucket name with a shorter one.
static void replace_bucket(char *src, char *buf, const char *oldstr, const char *newstr)
{
	size_t newpos = 0;
	size_t oldlen = strlen(oldstr);
	size_t newlen = strlen(newstr);
	char *match;
	char *oldpos = src;
	for (; (match = strstr(oldpos, oldstr)) != NULL; oldpos = match + oldlen) {
		size_t length = match - oldpos;
		memcpy(buf + newpos, oldpos, length);
		newpos += length;
		memcpy(buf + newpos, newstr, newlen);
		newpos += newlen;
	}
	strcpy_s(buf + newpos, 1024 - newpos, oldpos);
}

const char *auctionHashKey(const char *pchIdentifier)
{
	static char *bufarray[2] = { 0 };
	static int nbuf = 0;
	char *buf, *p;
	int itemtype, itemlevel;

	if (!s_DoNotGroupPrefixes)
		SetupPrefixes();

	// rotate static buffers so function can be used twice in strcmps()
	if (!bufarray[nbuf])
		bufarray[nbuf] = malloc(1024);
	buf = bufarray[nbuf];
	nbuf = (nbuf + 1) % 2;

	// Shortest possible identifier we care about
	if (strlen(pchIdentifier) < 4 || strlen(pchIdentifier) > 1024)
		return pchIdentifier;

	// Split at first space to get serialized type
	strcpy_s(buf, 1024, pchIdentifier);
	p = strchr(buf, ' ');
	if (!p)
		return pchIdentifier;

	// Extract ID and restore original string
	*p = 0;
	itemtype = atoi(buf);
	*p = ' ';
	p++;

	switch(itemtype) {
		case kTrayItemType_Salvage:
			// Group by rarity.
			if (stashFindInt(s_SalvageGroupBase, buf, NULL))
				return "11 S_SalvageGroupBase 0";
			if (stashFindInt(s_SalvageGroupCommon, buf, NULL))
				return "11 S_SalvageGroupCommon 0";
			if (stashFindInt(s_SalvageGroupUncommon, buf, NULL))
				return "11 S_SalvageGroupUncommon 0";
			if (stashFindInt(s_SalvageGroupRare, buf, NULL))
				return "11 S_SalvageGroupRare 0";
			return pchIdentifier;
		case kTrayItemType_Recipe:
			// if this is a common IO recipe, use the full identifier
			if (!strnicmp(p, "Invention_", 10))
				return pchIdentifier;

			// extract recipe level (1 based)
			p = strchr(p, ' ');
			if (!p)
				return pchIdentifier;
			*p = 0;
			itemlevel = atoi(p + 1);

			// make sure the level is in a sane range
			if (itemlevel < 1 || itemlevel > 50)
				return pchIdentifier;

			// it's a recipe, so chop off the level
			p = strrchr(buf, '_');
			if (!p)
				return pchIdentifier;
			*p = 0;
			return buf;
		case kTrayItemType_SpecializationInventory:
			if (!strnicmp(p, "Boosts.Crafted_", 15))
			{
				// extract enhancement level (0 based)
				p = strchr(p, ' ');
				if (!p)
					return pchIdentifier;
				*p = 0;
				itemlevel = atoi(p + 1);

				// make sure the level is in a sane range
				if (itemlevel < 0 || itemlevel > 49)
					return pchIdentifier;

				// exclude generic IOs
				if (stashFindInt(s_DoNotGroupPrefixes, buf, NULL))
					return pchIdentifier;

				// level is already removed by *p = 0 above
				return buf;
			}
			else if (!strnicmp(p, "Boosts.Synthetic_Hamidon_", 25))
			{
				replace_bucket((char*)pchIdentifier, buf, ".Synthetic_Hamidon_", ".Titan_");
				return buf;
			}
			else if (!strnicmp(p, "Boosts.Hamidon_", 15))
			{
				replace_bucket((char*)pchIdentifier, buf, ".Hamidon_", ".Titan_");
				return buf;
			}
			else if (!strnicmp(p, "Boosts.Hydra_", 13))
			{
				replace_bucket((char*)pchIdentifier, buf, ".Hydra_", ".Titan_");
				return buf;
			}
			else if (!strnicmp(p, "Boosts.Attuned_", 15))
			{
				replace_bucket((char*)pchIdentifier, buf, ".Attuned_", ".Crafted_");
				p = strchr(buf + 10, ' ');	// +10 to skip over itemtype
				if (!p)
					return pchIdentifier;
				*p = 0;
				return buf;
			}
			else if (!strnicmp(p, "Boosts.Superior_Attuned_", 24))
			{
				// This won't bucket ATOs with their superior versions because the superior version
				// starts with Superior_Attuned_Superior_, which translates to Crafted_Superior_.
				// It does bucket regular purples (such as Crafted_Hecatomb_A) with their Attuned
				// versions (Superior_Attuned_Hecatomb_A).
				replace_bucket((char*)pchIdentifier, buf, ".Superior_Attuned_", ".Crafted_");
				p = strchr(buf + 10, ' ');	// +10 to skip over itemtype
				if (!p)
					return pchIdentifier;
				*p = 0;
				return buf;
			}
	}

	return pchIdentifier;
}