// Some helpers for formatting mined data
// {category}.{name}[.{level}] : {category}.{context}
// For example Salvage.RiktiRifle : in.drop
//
// Note that only 'context' is evaluated more than once

#ifndef CLIENT
#include "miningaccumulator.h"
#include "strings_opt.h"

// char *name1, char *name2
#define DATA_MINE_NAME2_SEND(name1,name2,field,amount,link)	\
{															\
	char name[MINEACC_NAME_LEN];							\
	STR_COMBINE_SSS(name,name1,".",name2);					\
	MiningAccumulatorSendImmediate(name,field,amount,link)	\
}

// char *name1, char *name2, int level
#define DATA_MINE_NAME3_SEND(name1,name2,level,field,amount,link)	\
{																	\
	char name[MINEACC_NAME_LEN];									\
	STR_COMBINE_SSSSD(name,name1,".",name2,".",level);				\
	MiningAccumulatorSendImmediate(name,field,amount,link)			\
}

#define DATA_MINE_FIELD2(name,field1,field2,amount)	\
{													\
	char field[MINEACC_FIELD_LEN];					\
	STR_COMBINE_SSS(field,field1,".",field2);		\
	MiningAccumulatorAdd(name,field,amount);		\
}

// SalvageItem *salvage_item
#define DATA_MINE_SALVAGE(salvage_item,cat,context,amount)		\
{																\
	SalvageItem const *minee = salvage_item;					\
	if(minee && cat && context)									\
	{															\
		char name[MINEACC_NAME_LEN];							\
		STR_COMBINE_SS(name,"salvage.",minee->pchName);			\
		DATA_MINE_FIELD2(name,cat,context,amount);				\
	}															\
}

// SalvageItem *salvage_item
#define DATA_MINE_STOREDSALVAGE(salvage_item,cat,context,amount)		\
{																\
	SalvageItem const *minee = salvage_item;					\
	if(minee && cat && context)									\
	{															\
		char name[MINEACC_NAME_LEN];							\
		STR_COMBINE_SS(name,"storedsalvage.",minee->pchName);			\
		DATA_MINE_FIELD2(name,cat,context,amount);				\
	}															\
}

// DetailRecipe *detail_recipe
#define DATA_MINE_RECIPE(detail_recipe,cat,context,amount)					\
{																			\
	DetailRecipe const *minee = detail_recipe;								\
	if(minee && cat && context)												\
	{																		\
		char name[MINEACC_NAME_LEN];										\
		STR_COMBINE_SSSD(name,"recipe.",minee->pchName,".",minee->level);	\
		DATA_MINE_FIELD2(name,cat,context,amount);							\
	}																		\
}

// BasePower *base_power
#define DATA_MINE_ENHANCEMENT(base_power,level,cat,context,amount)			\
{																			\
	BasePower const *minee = base_power;									\
	if(minee && cat && context)												\
	{																		\
		char name[MINEACC_NAME_LEN];										\
		STR_COMBINE_SSSD(name,"enhancement.",minee->pchName,".",(level)+1);	\
		DATA_MINE_FIELD2(name,cat,context,amount);							\
	}																		\
}

// BasePower *base_power
#define DATA_MINE_INSPIRATION(base_power,cat,context,amount)	\
{																\
	BasePower const *minee = base_power;						\
	if(minee && cat && context)									\
	{															\
		char name[MINEACC_NAME_LEN];							\
		STR_COMBINE_SS(name,"inspiration.",minee->pchName);		\
		DATA_MINE_FIELD2(name,cat,context,amount);				\
	}															\
}

#else
#define DATA_MINE_FIELD2(name,field1,field2,amount)
#define DATA_MINE_SALVAGE(salvage_item,cat,context,amount)
#define DATA_MINE_RECIPE(detail_recipe,cat,context,amount)
#define DATA_MINE_ENHANCEMENT(base_power,level,cat,context,amount)
#define DATA_MINE_INSPIRATION(base_power,cat,context,amount)
#define DATA_MINE_STOREDSALVAGE(salvage_item,cat,context,amount)
#endif
