#ifndef ENTITY_ENUM_H
#define ENTITY_ENUM_H

typedef enum EntType
{
	ENTTYPE_NOT_SPECIFIED = 0,
	ENTTYPE_NPC,
	ENTTYPE_PLAYER,
	ENTTYPE_HERO,           // AI controlled Hero
	ENTTYPE_CRITTER,
	ENTTYPE_CAR,
	ENTTYPE_MOBILEGEOMETRY,
	ENTTYPE_MISSION_ITEM,
	ENTTYPE_DOOR,           // Just a simple door that players may interact with.

	ENTTYPE_COUNT,
} EntType;

typedef enum OnClickConditionType
{
	ONCLICK_HAS_REWARD_TOKEN,
	ONCLICK_HAS_BADGE,
	ONCLICK_DEFAULT,
} OnClickConditionType;

typedef enum EntMode
{
	ENT_DEAD = 1,
	ENT_AWAITING_TRADE_ANSWER,
	TOTAL_ENT_MODES,
}EntMode;

typedef enum EFloaterStyle
{
	kFloaterStyle_Damage,
	kFloaterStyle_Chat,
	kFloaterStyle_Chat_Private,
	kFloaterStyle_Icon,
	kFloaterStyle_Emote,
	kFloaterStyle_Info,
	kFloaterStyle_Attention,
	kFloaterStyle_AFK,
	kFloaterStyle_DeathRattle,
	kFloaterStyle_Caption,
	kFloaterStyle_PriorityAlert,

	kFloaterStyle_Count,
} EFloaterStyle; // Must be before PART_3

#define GANG_OF_PLAYERS_WHEN_IN_COED_TEAMUP 1

#endif
