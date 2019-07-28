/***************************************************************************
 *     Copyright (c) 2003-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************
 ***************************************************************************
 ***************************************************************************
 *
 *
 * DO NOT MODIFY THIS FILE UNLESS YOU KNOW WHAT YOU ARE DOING.
 *
 * It is partially generated with a program.
 *
 *
 ***************************************************************************
 ***************************************************************************
 ***************************************************************************
 ***************************************************************************/
#if (!defined(CHARACTER_ATTRIBS_H__)) || (defined(CHARACTER_ATTRIBS_PARSE_INFO_DEFINITIONS)&&!defined(CHARACTER_ATTRIBS_PARSE_INFO_DEFINED))
#define CHARACTER_ATTRIBS_H__

// CHARACTER_ATTRIBS_PARSE_INFO_DEFINITIONS must be defined before including this file
// to get the ParseInfo structures needed to read in files.

#include <stdlib.h>     // for offsetof
#include "stdtypes.h"

/***************************************************************************/
/***************************************************************************/

typedef struct CharacterAttributes
{
	// Defines the attributes which can be modified by effects
	// This list must have the same order as CharacterAttributesTable.
	// These must all be floats since some code plays pointer games
	//   and assumes it.

	//
	// DO NOT EDIT THIS CODE UNLESS YOU KNOW WHAT YOU ARE DOING.
	//
	// It is parsed by a sleazy perl script to generate the code at the
	// end of this file and the corresponding .c file.
	//

	float fDamageType[20];    // ModBase: 0.0, Add, TimesMax, Absolute, HitPoints, DumpAttribs: NO_CUR
		// Cur: Unused
		// Mod: The number of points to add or remove from Cur.HitPoints.

		// Make sure fHitPoints immediately follows this array

	float fHitPoints;         // ModBase: 0.0, Add, TimesMax, Absolute, HitPoints, DumpAttribs: ALWAYS
		// Cur: Number of hitpoints the player currently has. Running tally.
		//      Defaults to a table lookup
		// Mod: How many points to add or remove from the tally.
		//      Defaults to 0.0

	float fAbsorb;            // ModBase: 0.0, Add, TimesMax, Absolute, HitPoints, DumpAttribs: ALWAYS
		// Max: Number of absorb points the player currently has. Running tally.
		//      Defaults to a table lookup
		// Cur: Unused
		// Mod: Unused

	float fEndurance;         // ModBase: 0.0, Add, TimesMax, Absolute, DumpAttribs: ALWAYS
		// Cur: Measure of endurance the player currently has. Running tally.
		//      Defaults to 100
		// Mod: How many points to add or remove from the tally.

	float fInsight;           // ModBase: 0.0, Add, TimesMax, Absolute, DumpAttribs: ALWAYS, Synonym: Idea
		// Cur: Measure of Insight the player currently has. Running tally.
		//      Defaults to 100
		// Mod: How many points to add or remove from the tally.

	float fRage;             // ModBase: 0.0, Add, TimesMax, Absolute, DumpAttribs: ALWAYS
		// Cur: Measure of Rage the player currently has. Running tally.
		// Mod: How many points to add or remove from the tally.

	float fToHit;             // ModBase: 0.0, Add, CLAMP_CUR: No
		// Cur: The chance to hit a target.
		//      defaults to .75==75%, min 5%, max 95%
		// Mod: This is a percentage to be added to the base percentage value

	float fDefenseType[20];   // ModBase: 0.0, Add
		// Cur: The chance to avoid being hit by a certain kind of attack
		// Mod: This is a percentage added to the base percentage value.

		// Make sure fDefense immediately follows this array

	float fDefense;           // ModBase: 0.0, Add
		// Cur: The chance of avoiding being hit by a direct attack.
		// Mod: This is a percentage to be added to the base percentage value

	float fSpeedRunning;      // ModBase: 1.0, Multiply, Synonym: RunSpeed
	float fSpeedFlying;       // ModBase: 1.0, Multiply, Synonym: FlySpeed
	float fSpeedSwimming;     // ModBase: 1.0, Multiply
	float fSpeedJumping;      // ModBase: 1.0, Multiply
		// Cur: These are how fast the character travels as a percentage of
		//      the basic character speed.
		//      Defaults to 1.0 (100%) (30ft/s)
		// Mod: These are percentages to be multiplied with the base speed values.

	float fJumpHeight;        // ModBase: 1.0, Multiply
		// Cur: These are how well the character jumps as a percentage of
		//      the basic character jump velocity.
		//      Defaults to 1.0 (100%) (12')
		// Mod: This is a percentage to be multiplied with the base value.

	float fMovementControl;   // ModBase: 1.0, Multiply
	float fMovementFriction;  // ModBase: 1.0, Multiply
		// Cur: Controls the character's ability to move.
		//      Default to 0.0, which means use the built-in values.
		//      Running is ( 1.0, 0.3 )
		//      Jump is ( .03, 0.0 )
		// Mod: This is a percentage to be multiplied with the base value.

	float fStealth;           // ModBase: 0.0, Add
		// Cur: The chance of avoiding being seen when in eyeshot of an enemy.
		// Mod: This is a percentage to be added to the base percentage value

	float fStealthRadius;     // ModBase: 0.0, Add
		// Cur: This is a distance subtracted from an enemy mob's perception distance.
		// Mod: This is a distance to be added to the base distance value.

	float fStealthRadiusPlayer;     // ModBase: 0.0, Add
		// Cur: This is a distance subtracted from an enemy player's perception distance.
		// Mod: This is a distance to be added to the base distance value.

	float fPerceptionRadius;  // ModBase: 1.0, Multiply, PlusAbsolute
		// Cur: This is the distance the character can see. (Villains only.)
		// Mod: This is percent improvement over the base.

	float fRegeneration;      // ModBase: 1.0, Multiply
		// Cur: This is the rate at which HitPoints are regenerated.
		//      (1.0 = 100% MaxHP in a min)
		// Mod: This is a rate which will be multiplied by the base rate.

	float fRecovery;          // ModBase: 1.0, Multiply
		// Cur: This is the rate at which Endurance will recover.
		//      (1.0 = 100% MaxEndurance in a min)
		// Mod: This is a rate which will be multiplied by the base rate.

	float fInsightRecovery;   // ModBase: 1.0, Multiply
		// Cur: This is the rate at which Insight will recover.
		//      (1.0 = 100% MaxInsight in a min)
		// Mod: This is a rate which will be multiplied by the base rate.

	float fThreatLevel;       // ModBase: 0.0, Add
		// Cur: The general threat level of the character. (Used by AI)
		// Mod: Not really expected to be modded

	float fTaunt;             // ModBase: 1.0, Add
		// Cur: This is how much the character is taunting a target.
		// (Not really useful as a real attrib. By modifying this attrib,
		// the AI will become more belligerent to you.)

	float fPlacate;           // ModBase: 1.0, Add
		// Cur: This is how much the character is being placated.
		// (Not really useful as a real attrib. By modifying this attrib,
		// the AI will become less belligerent to you.)

	float fConfused;    // wanders around                // ModBase: 0.0, Add, Synonym: Confuse
	float fAfraid;      // Wants to run away             // ModBase: 0.0, Add
	float fTerrorized;  // Cowers                        // ModBase: 0.0, Add, Synonym: Terrorize
	float fHeld;        // cannot move or execute powers // ModBase: 0.0, Add
	float fImmobilized; // cannot move                   // ModBase: 0.0, Add, Synonym: Immobilize
	float fStunned;     // cannot execute powers         // ModBase: 0.0, Add, Synonym: Stun
	float fSleep;       // Immobilize+Stun unless woken  // ModBase: 0.0, Add
	float fFly;         // can fly                       // ModBase: 0.0, Add
	float fJumppack;    // can use jumppack              // ModBase: 0.0, Add
	float fTeleport;    // Initiates a teleport          // ModBase: 0.0, Add
	float fUntouchable; // Only caster can hit himself   // ModBase: 0.0, Add
	float fIntangible;  // Doesn't collide with others   // ModBase: 0.0, Add
	float fOnlyAffectsSelf; // Powers only affect self   // ModBase: 0.0, Add
		// Cur: If greater than zero, then the boolean is true.
		//      Defaults to 0.0
		// Mod: These are quantities which are added to the base value.

	float fExperienceGain; // XP gain factor             // ModBase: 0.0, Add
	float fInfluenceGain;  // Inf gain factor            // ModBase: 0.0, Add
	float fPrestigeGain;   // Prestige gain factor       // ModBase: 0.0, Add

	float fNullBool;                // ModBase: 0.0, Add, Synonym: Evade
		// Doesn't do anything on it's own. It's just a place to hang extra
		//   FX and other nefarious edge cases Design needs.

	float fKnockup;                 // ModBase: 0.0, Multiply
	float fKnockback;               // ModBase: 0.0, Multiply
	float fRepel;                   // ModBase: 0.0, Multiply
		// Cur: These are how hard the character hits enemies as a percentage
		//      of the basic character abilities.
		//      Defaults to 1.0 (100%)
		// Mod: These are percentages to be multiplied with the base speed values.

	// Power facets
	float fAccuracy;                // ModBase: 1.0, Multiply, DumpAttribs: STR_RES
	float fRadius;                  // ModBase: 1.0, Multiply, DumpAttribs: STR_RES
	float fArc;                     // ModBase: 1.0, Multiply, DumpAttribs: STR_RES
	float fRange;                   // ModBase: 1.0, Multiply, DumpAttribs: STR_RES
		// Cur: Unused
		// Mod: Unused
		// Str: These are percentages which are multiplied with a power's facets.

	float fTimeToActivate;          // ModBase: 1.0, Multiply, DumpAttribs: STR_RES
	float fRechargeTime;            // ModBase: 1.0, Multiply, DumpAttribs: STR_RES
	float fInterruptTime;           // ModBase: 1.0, Multiply, DumpAttribs: STR_RES
		// Cur: Unused
		// Mod: Unused
		// Str: These are rates which will be multiplied by the base (hard-coded) rate.

	float fEnduranceDiscount;       // ModBase: 1.0, Multiply, DumpAttribs: STR_RES
		// Cur: Unused
		// Mod: Unused
		// Str: This is a magnitude which will divide into the cost.

	float fInsightDiscount;         // ModBase: 1.0, Multiply, DumpAttribs: STR_RES, NoDump
		// Cur: Unused
		// Mod: Unused
		// Str: This is a magnitude which will divide into the cost.

	float fMeter;                   // ModBase: 0.0, Add, PlusAbsolute
		// Cur: A "fake" attribute which shows up as a meter in the UI.
		// Mod: Amount to increase or decrease the meter.

	float fElusivity[20];
		// Str: Anti-Accuracy

	float fElusivityBase;
} CharacterAttributes;

#ifdef CHARACTER_ATTRIBS_PARSE_INFO_DEFINITIONS
TokenizerParseInfo ParseCharacterAttributes[] =
{
	{ "{",               TOK_START,  0 },

	// START ParseCharacterAttributes
	{ "DamageType00",    TOK_F32                      (CharacterAttributes, fDamageType[0], 0)     },
	{ "DamageType01",    TOK_F32                      (CharacterAttributes, fDamageType[1], 0)     },
	{ "DamageType02",    TOK_F32                      (CharacterAttributes, fDamageType[2], 0)     },
	{ "DamageType03",    TOK_F32                      (CharacterAttributes, fDamageType[3], 0)     },
	{ "DamageType04",    TOK_F32                      (CharacterAttributes, fDamageType[4], 0)     },
	{ "DamageType05",    TOK_F32                      (CharacterAttributes, fDamageType[5], 0)     },
	{ "DamageType06",    TOK_F32                      (CharacterAttributes, fDamageType[6], 0)     },
	{ "DamageType07",    TOK_F32                      (CharacterAttributes, fDamageType[7], 0)     },
	{ "DamageType08",    TOK_F32                      (CharacterAttributes, fDamageType[8], 0)     },
	{ "DamageType09",    TOK_F32                      (CharacterAttributes, fDamageType[9], 0)     },
	{ "DamageType10",    TOK_F32                      (CharacterAttributes, fDamageType[10], 0)    },
	{ "DamageType11",    TOK_F32                      (CharacterAttributes, fDamageType[11], 0)    },
	{ "DamageType12",    TOK_F32                      (CharacterAttributes, fDamageType[12], 0)    },
	{ "DamageType13",    TOK_F32                      (CharacterAttributes, fDamageType[13], 0)    },
	{ "DamageType14",    TOK_F32                      (CharacterAttributes, fDamageType[14], 0)    },
	{ "DamageType15",    TOK_F32                      (CharacterAttributes, fDamageType[15], 0)    },
	{ "DamageType16",    TOK_F32                      (CharacterAttributes, fDamageType[16], 0)    },
	{ "DamageType17",    TOK_F32                      (CharacterAttributes, fDamageType[17], 0)    },
	{ "DamageType18",    TOK_F32                      (CharacterAttributes, fDamageType[18], 0)    },
	{ "DamageType19",    TOK_F32                      (CharacterAttributes, fDamageType[19], 0)    },
	{ "HitPoints",       TOK_F32                      (CharacterAttributes, fHitPoints, 0)         },
	{ "Absorb",          TOK_F32                      (CharacterAttributes, fAbsorb, 0)          },
	{ "Endurance",       TOK_F32                      (CharacterAttributes, fEndurance, 0)         },
	{ "Insight",         TOK_F32                      (CharacterAttributes, fInsight, 0)           },
	{ "Idea",            TOK_REDUNDANTNAME|TOK_F32    (CharacterAttributes, fInsight, 0)           },
	{ "Rage",            TOK_F32                      (CharacterAttributes, fRage, 0)              },
	{ "ToHit",           TOK_F32                      (CharacterAttributes, fToHit, 0)             },
	{ "DefenseType00",   TOK_F32                      (CharacterAttributes, fDefenseType[0], 0)    },
	{ "DefenseType01",   TOK_F32                      (CharacterAttributes, fDefenseType[1], 0)    },
	{ "DefenseType02",   TOK_F32                      (CharacterAttributes, fDefenseType[2], 0)    },
	{ "DefenseType03",   TOK_F32                      (CharacterAttributes, fDefenseType[3], 0)    },
	{ "DefenseType04",   TOK_F32                      (CharacterAttributes, fDefenseType[4], 0)    },
	{ "DefenseType05",   TOK_F32                      (CharacterAttributes, fDefenseType[5], 0)    },
	{ "DefenseType06",   TOK_F32                      (CharacterAttributes, fDefenseType[6], 0)    },
	{ "DefenseType07",   TOK_F32                      (CharacterAttributes, fDefenseType[7], 0)    },
	{ "DefenseType08",   TOK_F32                      (CharacterAttributes, fDefenseType[8], 0)    },
	{ "DefenseType09",   TOK_F32                      (CharacterAttributes, fDefenseType[9], 0)    },
	{ "DefenseType10",   TOK_F32                      (CharacterAttributes, fDefenseType[10], 0)   },
	{ "DefenseType11",   TOK_F32                      (CharacterAttributes, fDefenseType[11], 0)   },
	{ "DefenseType12",   TOK_F32                      (CharacterAttributes, fDefenseType[12], 0)   },
	{ "DefenseType13",   TOK_F32                      (CharacterAttributes, fDefenseType[13], 0)   },
	{ "DefenseType14",   TOK_F32                      (CharacterAttributes, fDefenseType[14], 0)   },
	{ "DefenseType15",   TOK_F32                      (CharacterAttributes, fDefenseType[15], 0)   },
	{ "DefenseType16",   TOK_F32                      (CharacterAttributes, fDefenseType[16], 0)   },
	{ "DefenseType17",   TOK_F32                      (CharacterAttributes, fDefenseType[17], 0)   },
	{ "DefenseType18",   TOK_F32                      (CharacterAttributes, fDefenseType[18], 0)   },
	{ "DefenseType19",   TOK_F32                      (CharacterAttributes, fDefenseType[19], 0)   },
	{ "Defense",         TOK_F32                      (CharacterAttributes, fDefense, 0)           },
	{ "SpeedRunning",    TOK_F32                      (CharacterAttributes, fSpeedRunning, 0)      },
	{ "RunSpeed",        TOK_REDUNDANTNAME|TOK_F32    (CharacterAttributes, fSpeedRunning, 0)      },
	{ "SpeedFlying",     TOK_F32                      (CharacterAttributes, fSpeedFlying, 0)       },
	{ "FlySpeed",        TOK_REDUNDANTNAME|TOK_F32    (CharacterAttributes, fSpeedFlying, 0)       },
	{ "SpeedSwimming",   TOK_F32                      (CharacterAttributes, fSpeedSwimming, 0)     },
	{ "SpeedJumping",    TOK_F32                      (CharacterAttributes, fSpeedJumping, 0)      },
	{ "JumpHeight",      TOK_F32                      (CharacterAttributes, fJumpHeight, 0)        },
	{ "MovementControl", TOK_F32                      (CharacterAttributes, fMovementControl, 0)   },
	{ "MovementFriction", TOK_F32                     (CharacterAttributes, fMovementFriction, 0)  },
	{ "Stealth",         TOK_F32                      (CharacterAttributes, fStealth, 0)           },
	{ "StealthRadius",   TOK_F32                      (CharacterAttributes, fStealthRadius, 0)     },
	{ "StealthRadiusPlayer", TOK_F32                  (CharacterAttributes, fStealthRadiusPlayer, 0) },
	{ "PerceptionRadius", TOK_F32                     (CharacterAttributes, fPerceptionRadius, 0)  },
	{ "Regeneration",    TOK_F32                      (CharacterAttributes, fRegeneration, 0)      },
	{ "Recovery",        TOK_F32                      (CharacterAttributes, fRecovery, 0)          },
	{ "InsightRecovery", TOK_F32                      (CharacterAttributes, fInsightRecovery, 0)   },
	{ "ThreatLevel",     TOK_F32                      (CharacterAttributes, fThreatLevel, 0)       },
	{ "Taunt",           TOK_F32                      (CharacterAttributes, fTaunt, 0)             },
	{ "Placate",         TOK_F32                      (CharacterAttributes, fPlacate, 0)           },
	{ "Confused",        TOK_F32                      (CharacterAttributes, fConfused, 0)          },
	{ "Confuse",         TOK_REDUNDANTNAME|TOK_F32    (CharacterAttributes, fConfused, 0)          },
	{ "Afraid",          TOK_F32                      (CharacterAttributes, fAfraid, 0)            },
	{ "Terrorized",      TOK_F32                      (CharacterAttributes, fTerrorized, 0)        },
	{ "Terrorize",       TOK_REDUNDANTNAME|TOK_F32    (CharacterAttributes, fTerrorized, 0)        },
	{ "Held",            TOK_F32                      (CharacterAttributes, fHeld, 0)              },
	{ "Immobilized",     TOK_F32                      (CharacterAttributes, fImmobilized, 0)       },
	{ "Immobilize",      TOK_REDUNDANTNAME|TOK_F32    (CharacterAttributes, fImmobilized, 0)       },
	{ "Stunned",         TOK_F32                      (CharacterAttributes, fStunned, 0)           },
	{ "Stun",            TOK_REDUNDANTNAME|TOK_F32    (CharacterAttributes, fStunned, 0)           },
	{ "Sleep",           TOK_F32                      (CharacterAttributes, fSleep, 0)             },
	{ "Fly",             TOK_F32                      (CharacterAttributes, fFly, 0)               },
	{ "Jumppack",        TOK_F32                      (CharacterAttributes, fJumppack, 0)          },
	{ "Teleport",        TOK_F32                      (CharacterAttributes, fTeleport, 0)          },
	{ "Untouchable",     TOK_F32                      (CharacterAttributes, fUntouchable, 0)       },
	{ "Intangible",      TOK_F32                      (CharacterAttributes, fIntangible, 0)        },
	{ "OnlyAffectsSelf", TOK_F32                      (CharacterAttributes, fOnlyAffectsSelf, 0)   },
	{ "ExperienceGain",  TOK_F32                      (CharacterAttributes, fExperienceGain, 0)    },
	{ "InfluenceGain",   TOK_F32                      (CharacterAttributes, fInfluenceGain, 0)     },
	{ "PrestigeGain",    TOK_F32                      (CharacterAttributes, fPrestigeGain, 0)      },
	{ "NullBool",        TOK_F32                      (CharacterAttributes, fNullBool, 0)          },
	{ "Evade",           TOK_REDUNDANTNAME|TOK_F32    (CharacterAttributes, fNullBool, 0)          },
	{ "Knockup",         TOK_F32                      (CharacterAttributes, fKnockup, 0)           },
	{ "Knockback",       TOK_F32                      (CharacterAttributes, fKnockback, 0)         },
	{ "Repel",           TOK_F32                      (CharacterAttributes, fRepel, 0)             },
	{ "Accuracy",        TOK_F32                      (CharacterAttributes, fAccuracy, 0)          },
	{ "Radius",          TOK_F32                      (CharacterAttributes, fRadius, 0)            },
	{ "Arc",             TOK_F32                      (CharacterAttributes, fArc, 0)               },
	{ "Range",           TOK_F32                      (CharacterAttributes, fRange, 0)             },
	{ "TimeToActivate",  TOK_F32                      (CharacterAttributes, fTimeToActivate, 0)    },
	{ "RechargeTime",    TOK_F32                      (CharacterAttributes, fRechargeTime, 0)      },
	{ "InterruptTime",   TOK_F32                      (CharacterAttributes, fInterruptTime, 0)     },
	{ "EnduranceDiscount", TOK_F32                    (CharacterAttributes, fEnduranceDiscount, 0) },
	{ "InsightDiscount", TOK_F32                      (CharacterAttributes, fInsightDiscount, 0)   },
	{ "Meter",           TOK_F32                      (CharacterAttributes, fMeter, 0)             },
	{ "Elusivity00",     TOK_F32                      (CharacterAttributes, fElusivity[0], 0)    },
	{ "Elusivity01",     TOK_F32                      (CharacterAttributes, fElusivity[1], 0)    },
	{ "Elusivity02",     TOK_F32                      (CharacterAttributes, fElusivity[2], 0)    },
	{ "Elusivity03",     TOK_F32                      (CharacterAttributes, fElusivity[3], 0)    },
	{ "Elusivity04",     TOK_F32                      (CharacterAttributes, fElusivity[4], 0)    },
	{ "Elusivity05",     TOK_F32                      (CharacterAttributes, fElusivity[5], 0)    },
	{ "Elusivity06",     TOK_F32                      (CharacterAttributes, fElusivity[6], 0)    },
	{ "Elusivity07",     TOK_F32                      (CharacterAttributes, fElusivity[7], 0)    },
	{ "Elusivity08",     TOK_F32                      (CharacterAttributes, fElusivity[8], 0)    },
	{ "Elusivity09",     TOK_F32                      (CharacterAttributes, fElusivity[9], 0)    },
	{ "Elusivity10",     TOK_F32                      (CharacterAttributes, fElusivity[10], 0)   },
	{ "Elusivity11",     TOK_F32                      (CharacterAttributes, fElusivity[11], 0)   },
	{ "Elusivity12",     TOK_F32                      (CharacterAttributes, fElusivity[12], 0)   },
	{ "Elusivity13",     TOK_F32                      (CharacterAttributes, fElusivity[13], 0)   },
	{ "Elusivity14",     TOK_F32                      (CharacterAttributes, fElusivity[14], 0)   },
	{ "Elusivity15",     TOK_F32                      (CharacterAttributes, fElusivity[15], 0)   },
	{ "Elusivity16",     TOK_F32                      (CharacterAttributes, fElusivity[16], 0)   },
	{ "Elusivity17",     TOK_F32                      (CharacterAttributes, fElusivity[17], 0)   },
	{ "Elusivity18",     TOK_F32                      (CharacterAttributes, fElusivity[18], 0)   },
	{ "Elusivity19",     TOK_F32                      (CharacterAttributes, fElusivity[19], 0)   },
	{ "ElusivityBase",   TOK_F32                      (CharacterAttributes, fElusivityBase, 0)   },
	// END ParseCharacterAttributes

	{ "}",                 TOK_END,   -1  },
	{ 0 }
};
#endif

/***************************************************************************/
/***************************************************************************/

// This is no longer auto-generated and was hand optimized.
#define IS_HITPOINTS(x) ( \
	((x)>=offsetof(CharacterAttributes, fDamageType[0]) \
		&& (x)<=offsetof(CharacterAttributes, fAbsorb)) \
	)

#define IS_KNOCK(x) ( \
	(x)>=offsetof(CharacterAttributes, fKnockup) \
		&& (x)<=offsetof(CharacterAttributes, fRepel) \
	)

#define IS_DISABLINGSTATUS(x) ( \
	(x)>=offsetof(CharacterAttributes, fHeld) \
	&& (x)<=offsetof(CharacterAttributes, fSleep) \
	)

#define IS_MODIFIER(x) ( \
	(x)==offsetof(CharacterAttribSet, pattrAbsolute) \
		|| (x)==offsetof(CharacterAttribSet, pattrMod) \
	)

#define IS_CHAR_ATTRIBUTE(x,y) ((x)==offsetof(CharacterAttributes, y))

#define CHAR_ATTRIB_COUNT (sizeof(CharacterAttributes)/sizeof(float))

/***************************************************************************/
/***************************************************************************/

typedef enum ESpecialAttrib
{
	kSpecialAttrib_Translucency			= sizeof(CharacterAttributes),
	kSpecialAttrib_EntCreate			= sizeof(CharacterAttributes) + 1,
	kSpecialAttrib_ClearDamagers		= sizeof(CharacterAttributes) + 2,
	kSpecialAttrib_SilentKill			= sizeof(CharacterAttributes) + 3,
	kSpecialAttrib_XPDebtProtection		= sizeof(CharacterAttributes) + 4,
	kSpecialAttrib_SetMode				= sizeof(CharacterAttributes) + 5,
	kSpecialAttrib_SetCostume			= sizeof(CharacterAttributes) + 6,
	kSpecialAttrib_Glide				= sizeof(CharacterAttributes) + 7,
	kSpecialAttrib_Null					= sizeof(CharacterAttributes) + 8,
	kSpecialAttrib_Avoid				= sizeof(CharacterAttributes) + 9,
	kSpecialAttrib_Reward				= sizeof(CharacterAttributes) + 10,
	kSpecialAttrib_XPDebt				= sizeof(CharacterAttributes) + 11,
	kSpecialAttrib_DropToggles			= sizeof(CharacterAttributes) + 12,
	kSpecialAttrib_GrantPower			= sizeof(CharacterAttributes) + 13,
	kSpecialAttrib_RevokePower			= sizeof(CharacterAttributes) + 14,
	kSpecialAttrib_UnsetMode			= sizeof(CharacterAttributes) + 15,
	kSpecialAttrib_GlobalChanceMod		= sizeof(CharacterAttributes) + 16,
	kSpecialAttrib_PowerChanceMod		= sizeof(CharacterAttributes) + 17,
	kSpecialAttrib_GrantBoostedPower	= sizeof(CharacterAttributes) + 18,
	kSpecialAttrib_ViewAttrib			= sizeof(CharacterAttributes) + 19,
	kSpecialAttrib_RewardSource			= sizeof(CharacterAttributes) + 20,
	kSpecialAttrib_RewardSourceTeam		= sizeof(CharacterAttributes) + 21,
	kSpecialAttrib_ClearFog				= sizeof(CharacterAttributes) + 22,
	kSpecialAttrib_CombatPhase			= sizeof(CharacterAttributes) + 23,
	kSpecialAttrib_CombatModShift		= sizeof(CharacterAttributes) + 24,
	kSpecialAttrib_RechargePower		= sizeof(CharacterAttributes) + 25,
	kSpecialAttrib_VisionPhase			= sizeof(CharacterAttributes) + 26,
	kSpecialAttrib_NinjaRun				= sizeof(CharacterAttributes) + 27,
	kSpecialAttrib_Walk					= sizeof(CharacterAttributes) + 28,
	kSpecialAttrib_BeastRun				= sizeof(CharacterAttributes) + 29,
	kSpecialAttrib_SteamJump			= sizeof(CharacterAttributes) + 30,
	kSpecialAttrib_DesignerStatus		= sizeof(CharacterAttributes) + 31,
	kSpecialAttrib_ExclusiveVisionPhase	= sizeof(CharacterAttributes) + 32,
	kSpecialAttrib_HoverBoard			= sizeof(CharacterAttributes) + 33,
	kSpecialAttrib_SetSZEValue			= sizeof(CharacterAttributes) + 34,
	kSpecialAttrib_AddBehavior			= sizeof(CharacterAttributes) + 35,
	kSpecialAttrib_MagicCarpet			= sizeof(CharacterAttributes) + 36,
	kSpecialAttrib_TokenAdd				= sizeof(CharacterAttributes) + 37,
	kSpecialAttrib_TokenSet				= sizeof(CharacterAttributes) + 38,
	kSpecialAttrib_TokenClear			= sizeof(CharacterAttributes) + 39,
	kSpecialAttrib_LuaExec				= sizeof(CharacterAttributes) + 40,
	kSpecialAttrib_ForceMove			= sizeof(CharacterAttributes) + 41,
	kSpecialAttrib_ParkourRun			= sizeof(CharacterAttributes) + 42,
	kSpecialAttrib_CancelMods			= sizeof(CharacterAttributes) + 43,
	kSpecialAttrib_ExecutePower			= sizeof(CharacterAttributes) + 44,

	// temp compat hack!
	kSpecialAttrib_PowerRedirect		= sizeof(CharacterAttributes) + 1000,
} ESpecialAttrib;

/***************************************************************************/
/***************************************************************************/

extern CharacterAttributes g_attrModBase;
extern CharacterAttributes g_attrZeroes;
extern CharacterAttributes g_attrOnes;

/***************************************************************************/
/***************************************************************************/

typedef struct CharacterAttribSet
{
	CharacterAttributes *pattrMod;
	CharacterAttributes *pattrMax;
	CharacterAttributes *pattrStrength;
	CharacterAttributes *pattrResistance;
	CharacterAttributes *pattrAbsolute;
} CharacterAttribSet;

/***************************************************************************/
/***************************************************************************/

typedef struct CharacterAttributesTable
{
	// Defines the attributes which can be modified by effects
	// This list must have the same order as CharacterAttributes.
	// These must all be float *s since some code plays pointer games
	//   and assumes it.

	// START CharacterAttributesTable
	float *pfDamageType[20];
	float *pfHitPoints;
	float *pfEndurance;
	float *pfInsight;
	float *pfRage;
	float *pfToHit;
	float *pfDefenseType[20];
	float *pfDefense;
	float *pfSpeedRunning;
	float *pfSpeedFlying;
	float *pfSpeedSwimming;
	float *pfSpeedJumping;
	float *pfJumpHeight;
	float *pfMovementControl;
	float *pfMovementFriction;
	float *pfStealth;
	float *pfStealthRadius;
	float *pfStealthRadiusPlayer;
	float *pfPerceptionRadius;
	float *pfRegeneration;
	float *pfRecovery;
	float *pfInsightRecovery;
	float *pfThreatLevel;
	float *pfTaunt;
	float *pfPlacate;
	float *pfConfused;
	float *pfAfraid;
	float *pfTerrorized;
	float *pfHeld;
	float *pfImmobilized;
	float *pfStunned;
	float *pfSleep;
	float *pfFly;
	float *pfJumppack;
	float *pfTeleport;
	float *pfUntouchable;
	float *pfIntangible;
	float *pfOnlyAffectsSelf;
	float *pfExperienceGain;
	float *pfInfluenceGain;
	float *pfPrestigeGain;
	float *pfNullBool;
	float *pfKnockup;
	float *pfKnockback;
	float *pfRepel;
	float *pfAccuracy;
	float *pfRadius;
	float *pfArc;
	float *pfRange;
	float *pfTimeToActivate;
	float *pfRechargeTime;
	float *pfInterruptTime;
	float *pfEnduranceDiscount;
	float *pfInsightDiscount;
	float *pfMeter;
	float *pfElusivity[20];
	float *pfElusivityBase;
	float *pfAbsorb;
	// END CharacterAttributesTable

} CharacterAttributesTable;

#ifdef CHARACTER_ATTRIBS_PARSE_INFO_DEFINITIONS
TokenizerParseInfo ParseCharacterAttributesTable[] =
{
	{ "{",                 TOK_START,    0 },

	// START ParseCharacterAttributesTable
	{ "DamageType00",    TOK_F32ARRAY(CharacterAttributesTable, pfDamageType[0])     },
	{ "DamageType01",    TOK_F32ARRAY(CharacterAttributesTable, pfDamageType[1])     },
	{ "DamageType02",    TOK_F32ARRAY(CharacterAttributesTable, pfDamageType[2])     },
	{ "DamageType03",    TOK_F32ARRAY(CharacterAttributesTable, pfDamageType[3])     },
	{ "DamageType04",    TOK_F32ARRAY(CharacterAttributesTable, pfDamageType[4])     },
	{ "DamageType05",    TOK_F32ARRAY(CharacterAttributesTable, pfDamageType[5])     },
	{ "DamageType06",    TOK_F32ARRAY(CharacterAttributesTable, pfDamageType[6])     },
	{ "DamageType07",    TOK_F32ARRAY(CharacterAttributesTable, pfDamageType[7])     },
	{ "DamageType08",    TOK_F32ARRAY(CharacterAttributesTable, pfDamageType[8])     },
	{ "DamageType09",    TOK_F32ARRAY(CharacterAttributesTable, pfDamageType[9])     },
	{ "DamageType10",    TOK_F32ARRAY(CharacterAttributesTable, pfDamageType[10])    },
	{ "DamageType11",    TOK_F32ARRAY(CharacterAttributesTable, pfDamageType[11])    },
	{ "DamageType12",    TOK_F32ARRAY(CharacterAttributesTable, pfDamageType[12])    },
	{ "DamageType13",    TOK_F32ARRAY(CharacterAttributesTable, pfDamageType[13])    },
	{ "DamageType14",    TOK_F32ARRAY(CharacterAttributesTable, pfDamageType[14])    },
	{ "DamageType15",    TOK_F32ARRAY(CharacterAttributesTable, pfDamageType[15])    },
	{ "DamageType16",    TOK_F32ARRAY(CharacterAttributesTable, pfDamageType[16])    },
	{ "DamageType17",    TOK_F32ARRAY(CharacterAttributesTable, pfDamageType[17])    },
	{ "DamageType18",    TOK_F32ARRAY(CharacterAttributesTable, pfDamageType[18])    },
	{ "DamageType19",    TOK_F32ARRAY(CharacterAttributesTable, pfDamageType[19])    },
	{ "HitPoints",       TOK_F32ARRAY(CharacterAttributesTable, pfHitPoints)         },
	{ "Endurance",       TOK_F32ARRAY(CharacterAttributesTable, pfEndurance)         },
	{ "Insight",         TOK_F32ARRAY(CharacterAttributesTable, pfInsight)           },
	{ "Idea",            TOK_REDUNDANTNAME|TOK_F32ARRAY(CharacterAttributesTable, pfInsight)           },
	{ "Rage",            TOK_F32ARRAY(CharacterAttributesTable, pfRage)              },
	{ "ToHit",           TOK_F32ARRAY(CharacterAttributesTable, pfToHit)             },
	{ "DefenseType00",   TOK_F32ARRAY(CharacterAttributesTable, pfDefenseType[0])    },
	{ "DefenseType01",   TOK_F32ARRAY(CharacterAttributesTable, pfDefenseType[1])    },
	{ "DefenseType02",   TOK_F32ARRAY(CharacterAttributesTable, pfDefenseType[2])    },
	{ "DefenseType03",   TOK_F32ARRAY(CharacterAttributesTable, pfDefenseType[3])    },
	{ "DefenseType04",   TOK_F32ARRAY(CharacterAttributesTable, pfDefenseType[4])    },
	{ "DefenseType05",   TOK_F32ARRAY(CharacterAttributesTable, pfDefenseType[5])    },
	{ "DefenseType06",   TOK_F32ARRAY(CharacterAttributesTable, pfDefenseType[6])    },
	{ "DefenseType07",   TOK_F32ARRAY(CharacterAttributesTable, pfDefenseType[7])    },
	{ "DefenseType08",   TOK_F32ARRAY(CharacterAttributesTable, pfDefenseType[8])    },
	{ "DefenseType09",   TOK_F32ARRAY(CharacterAttributesTable, pfDefenseType[9])    },
	{ "DefenseType10",   TOK_F32ARRAY(CharacterAttributesTable, pfDefenseType[10])   },
	{ "DefenseType11",   TOK_F32ARRAY(CharacterAttributesTable, pfDefenseType[11])   },
	{ "DefenseType12",   TOK_F32ARRAY(CharacterAttributesTable, pfDefenseType[12])   },
	{ "DefenseType13",   TOK_F32ARRAY(CharacterAttributesTable, pfDefenseType[13])   },
	{ "DefenseType14",   TOK_F32ARRAY(CharacterAttributesTable, pfDefenseType[14])   },
	{ "DefenseType15",   TOK_F32ARRAY(CharacterAttributesTable, pfDefenseType[15])   },
	{ "DefenseType16",   TOK_F32ARRAY(CharacterAttributesTable, pfDefenseType[16])   },
	{ "DefenseType17",   TOK_F32ARRAY(CharacterAttributesTable, pfDefenseType[17])   },
	{ "DefenseType18",   TOK_F32ARRAY(CharacterAttributesTable, pfDefenseType[18])   },
	{ "DefenseType19",   TOK_F32ARRAY(CharacterAttributesTable, pfDefenseType[19])   },
	{ "Defense",         TOK_F32ARRAY(CharacterAttributesTable, pfDefense)           },
	{ "SpeedRunning",    TOK_F32ARRAY(CharacterAttributesTable, pfSpeedRunning)      },
	{ "RunSpeed",        TOK_REDUNDANTNAME|TOK_F32ARRAY(CharacterAttributesTable, pfSpeedRunning)      },
	{ "SpeedFlying",     TOK_F32ARRAY(CharacterAttributesTable, pfSpeedFlying)       },
	{ "FlySpeed",        TOK_REDUNDANTNAME|TOK_F32ARRAY(CharacterAttributesTable, pfSpeedFlying)       },
	{ "SpeedSwimming",   TOK_F32ARRAY(CharacterAttributesTable, pfSpeedSwimming)     },
	{ "SpeedJumping",    TOK_F32ARRAY(CharacterAttributesTable, pfSpeedJumping)      },
	{ "JumpHeight",      TOK_F32ARRAY(CharacterAttributesTable, pfJumpHeight)        },
	{ "MovementControl", TOK_F32ARRAY(CharacterAttributesTable, pfMovementControl)   },
	{ "MovementFriction", TOK_F32ARRAY(CharacterAttributesTable, pfMovementFriction)  },
	{ "Stealth",         TOK_F32ARRAY(CharacterAttributesTable, pfStealth)           },
	{ "StealthRadius",   TOK_F32ARRAY(CharacterAttributesTable, pfStealthRadius)     },
	{ "StealthRadiusPlayer", TOK_F32ARRAY(CharacterAttributesTable, pfStealthRadiusPlayer) },
	{ "PerceptionRadius", TOK_F32ARRAY(CharacterAttributesTable, pfPerceptionRadius)  },
	{ "Regeneration",    TOK_F32ARRAY(CharacterAttributesTable, pfRegeneration)      },
	{ "Recovery",        TOK_F32ARRAY(CharacterAttributesTable, pfRecovery)          },
	{ "InsightRecovery", TOK_F32ARRAY(CharacterAttributesTable, pfInsightRecovery)   },
	{ "ThreatLevel",     TOK_F32ARRAY(CharacterAttributesTable, pfThreatLevel)       },
	{ "Taunt",           TOK_F32ARRAY(CharacterAttributesTable, pfTaunt)             },
	{ "Placate",         TOK_F32ARRAY(CharacterAttributesTable, pfPlacate)           },
	{ "Confused",        TOK_F32ARRAY(CharacterAttributesTable, pfConfused)          },
	{ "Confuse",         TOK_REDUNDANTNAME|TOK_F32ARRAY(CharacterAttributesTable, pfConfused)          },
	{ "Afraid",          TOK_F32ARRAY(CharacterAttributesTable, pfAfraid)            },
	{ "Terrorized",      TOK_F32ARRAY(CharacterAttributesTable, pfTerrorized)        },
	{ "Terrorize",       TOK_REDUNDANTNAME|TOK_F32ARRAY(CharacterAttributesTable, pfTerrorized)        },
	{ "Held",            TOK_F32ARRAY(CharacterAttributesTable, pfHeld)              },
	{ "Immobilized",     TOK_F32ARRAY(CharacterAttributesTable, pfImmobilized)       },
	{ "Immobilize",      TOK_REDUNDANTNAME|TOK_F32ARRAY(CharacterAttributesTable, pfImmobilized)       },
	{ "Stunned",         TOK_F32ARRAY(CharacterAttributesTable, pfStunned)           },
	{ "Stun",            TOK_REDUNDANTNAME|TOK_F32ARRAY(CharacterAttributesTable, pfStunned)           },
	{ "Sleep",           TOK_F32ARRAY(CharacterAttributesTable, pfSleep)             },
	{ "Fly",             TOK_F32ARRAY(CharacterAttributesTable, pfFly)               },
	{ "Jumppack",        TOK_F32ARRAY(CharacterAttributesTable, pfJumppack)          },
	{ "Teleport",        TOK_F32ARRAY(CharacterAttributesTable, pfTeleport)          },
	{ "Untouchable",     TOK_F32ARRAY(CharacterAttributesTable, pfUntouchable)       },
	{ "Intangible",      TOK_F32ARRAY(CharacterAttributesTable, pfIntangible)        },
	{ "OnlyAffectsSelf", TOK_F32ARRAY(CharacterAttributesTable, pfOnlyAffectsSelf)   },
	{ "ExperienceGain",  TOK_F32ARRAY(CharacterAttributesTable, pfExperienceGain)    },
	{ "InfluenceGain",   TOK_F32ARRAY(CharacterAttributesTable, pfInfluenceGain)     },
	{ "PrestigeGain",    TOK_F32ARRAY(CharacterAttributesTable, pfPrestigeGain)      },
	{ "NullBool",        TOK_F32ARRAY(CharacterAttributesTable, pfNullBool)          },
	{ "Evade",           TOK_REDUNDANTNAME|TOK_F32ARRAY(CharacterAttributesTable, pfNullBool)          },
	{ "Knockup",         TOK_F32ARRAY(CharacterAttributesTable, pfKnockup)           },
	{ "Knockback",       TOK_F32ARRAY(CharacterAttributesTable, pfKnockback)         },
	{ "Repel",           TOK_F32ARRAY(CharacterAttributesTable, pfRepel)             },
	{ "Accuracy",        TOK_F32ARRAY(CharacterAttributesTable, pfAccuracy)          },
	{ "Radius",          TOK_F32ARRAY(CharacterAttributesTable, pfRadius)            },
	{ "Arc",             TOK_F32ARRAY(CharacterAttributesTable, pfArc)               },
	{ "Range",           TOK_F32ARRAY(CharacterAttributesTable, pfRange)             },
	{ "TimeToActivate",  TOK_F32ARRAY(CharacterAttributesTable, pfTimeToActivate)    },
	{ "RechargeTime",    TOK_F32ARRAY(CharacterAttributesTable, pfRechargeTime)      },
	{ "InterruptTime",   TOK_F32ARRAY(CharacterAttributesTable, pfInterruptTime)     },
	{ "EnduranceDiscount", TOK_F32ARRAY(CharacterAttributesTable, pfEnduranceDiscount) },
	{ "InsightDiscount", TOK_F32ARRAY(CharacterAttributesTable, pfInsightDiscount)   },
	{ "Meter",           TOK_F32ARRAY(CharacterAttributesTable, pfMeter)             },
	{ "Elusivity00",     TOK_F32ARRAY(CharacterAttributesTable, pfElusivity[0])    },
	{ "Elusivity01",     TOK_F32ARRAY(CharacterAttributesTable, pfElusivity[1])    },
	{ "Elusivity02",     TOK_F32ARRAY(CharacterAttributesTable, pfElusivity[2])    },
	{ "Elusivity03",     TOK_F32ARRAY(CharacterAttributesTable, pfElusivity[3])    },
	{ "Elusivity04",     TOK_F32ARRAY(CharacterAttributesTable, pfElusivity[4])    },
	{ "Elusivity05",     TOK_F32ARRAY(CharacterAttributesTable, pfElusivity[5])    },
	{ "Elusivity06",     TOK_F32ARRAY(CharacterAttributesTable, pfElusivity[6])    },
	{ "Elusivity07",     TOK_F32ARRAY(CharacterAttributesTable, pfElusivity[7])    },
	{ "Elusivity08",     TOK_F32ARRAY(CharacterAttributesTable, pfElusivity[8])    },
	{ "Elusivity09",     TOK_F32ARRAY(CharacterAttributesTable, pfElusivity[9])    },
	{ "Elusivity10",     TOK_F32ARRAY(CharacterAttributesTable, pfElusivity[10])   },
	{ "Elusivity11",     TOK_F32ARRAY(CharacterAttributesTable, pfElusivity[11])   },
	{ "Elusivity12",     TOK_F32ARRAY(CharacterAttributesTable, pfElusivity[12])   },
	{ "Elusivity13",     TOK_F32ARRAY(CharacterAttributesTable, pfElusivity[13])   },
	{ "Elusivity14",     TOK_F32ARRAY(CharacterAttributesTable, pfElusivity[14])   },
	{ "Elusivity15",     TOK_F32ARRAY(CharacterAttributesTable, pfElusivity[15])   },
	{ "Elusivity16",     TOK_F32ARRAY(CharacterAttributesTable, pfElusivity[16])   },
	{ "Elusivity17",     TOK_F32ARRAY(CharacterAttributesTable, pfElusivity[17])   },
	{ "Elusivity18",     TOK_F32ARRAY(CharacterAttributesTable, pfElusivity[18])   },
	{ "Elusivity19",     TOK_F32ARRAY(CharacterAttributesTable, pfElusivity[19])   },
	{ "Defense",         TOK_F32ARRAY(CharacterAttributesTable, pfDefense)         },
	{ "Absorb",          TOK_F32ARRAY(CharacterAttributesTable, pfAbsorb)          },

	// END ParseCharacterAttributesTable

	{ "}",                 TOK_END,      0 },
	{ 0 }
};
#endif

/***************************************************************************/
/***************************************************************************/

typedef struct DimReturn
{
	float fStart;
	float fHandicap;
	float fBasis;
} DimReturn;

typedef struct AttribDimReturnSet
{
	int bDefault;
	int *offAttribs;
	DimReturn **pReturns;
} AttribDimReturnSet;

typedef struct DimReturnSet
{
	int bDefault;
	int *piBoosts;
	AttribDimReturnSet **ppReturns;
} DimReturnSet;

typedef struct DimReturnList
{
	const DimReturnSet **ppSets;
} DimReturnList;

extern SHARED_MEMORY DimReturnList g_DimReturnList;

#ifdef CHARACTER_ATTRIBS_PARSE_INFO_DEFINITIONS
extern TokenizerParseInfo ParseDimReturnList[]; // in powers_load.c
#endif

/***************************************************************************/
/***************************************************************************/

static INLINEDBG void SetCharacterAttributesToOne(void *dst)
{
	int i;
	int *pi = (int *)dst;
	for(i=sizeof(CharacterAttributes)/4; i>0; i--)
	{
		*pi++ = 0x3f800000; // hex for 1.0f
	}
}


static INLINEDBG void CLAMPF32_IN_PLACE(F32 *var, F32 min, F32 max)
{
	if(*var < min)
		*var = min;
	else if(*var > max)
		*var = max;
}

/***************************************************************************/
/***************************************************************************/

const CharacterAttributes *CreateCharacterAttributes(const CharacterAttributesTable **pptab, int iNum, float fDefault);

typedef struct Character Character;
typedef struct BasePower BasePower;

char *DumpAttribs(Character *pchar, char *curPos);
char *DumpStrAttribs(Character *pchar, CharacterAttributes *pattrStrength, char *curPos);

void SetModBase(CharacterAttributes *pattr);
void ClampMax(Character *p, CharacterAttribSet *pset);
void CalcResistance(Character *p, CharacterAttribSet *pset, CharacterAttributes *pattrResBuffs, CharacterAttributes *pattrResDebuffs);
void ClampStrength(Character *p, CharacterAttributes *pattrStr);
void CalcCur(Character *p, CharacterAttribSet *pset);
void ClampCur(Character *p);

void AggregateStrength(Character *p, const BasePower *ppowBase, CharacterAttributes *pDest, CharacterAttributes *pBoosts);

#ifdef CHARACTER_ATTRIBS_PARSE_INFO_DEFINITIONS
#define CHARACTER_ATTRIBS_PARSE_INFO_DEFINED
#endif

#endif /* #ifndef CHARACTER_ATTRIBS_H__ */

/* End of File */

