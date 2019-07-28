/***************************************************************************
 *     Copyright (c) 2013 Abandonware Holdings, Ltd.
 *     All Rights Reserved
 ***************************************************************************/
#if (!defined(ATTRIBMOD_H__)) || (defined(ATTRIBMOD_PARSE_INFO_DEFINITIONS)&&!defined(ATTRIBMOD_PARSE_INFO_DEFINED))
#define ATTRIBMOD_H__

// ATTRIBMOD_PARSE_INFO_DEFINITIONS must be defined before including this file
// to get the ParseInfo structures needed to read in files.

#include <stdlib.h>   // for offsetof
#include <stdarg.h>

#include "stdtypes.h"  // for bool
#include "EntityRef.h" // for EntityRef
#include "earray.h"
#include "character_attribs.h"
#include "Color.h"
#include "textparser.h"

typedef struct Character Character;
typedef struct CharacterClass CharacterClass;
typedef struct Power Power;
typedef struct CharacterAttributes CharacterAttributes;
typedef struct CharacterAttribSet CharacterAttribSet;
typedef struct PowerCategory PowerCategory;
typedef struct BasePowerSet BasePowerSet;
typedef struct BasePower BasePower;
typedef struct PowerFX PowerFX;
typedef int BoostType;

/***************************************************************************/
/***************************************************************************/

#define ASPECT_ATTRIB_GET_PTR(pattribset, offAspect, offAttr) \
	((float *)((*(char **)(((char *)(pattribset))+(offAspect)))+(offAttr)))

#define ATTRIB_GET_PTR(pattr, offAttr) \
	((float *)(((char *)(pattr))+(offAttr)))

#define ATTRIBMOD_DURATION_FOREVER (999999)
	// A very large number of seconds which is essentially forever. This is
	// used as a flag; anything larger than or equal to this value will
	// be handled specially.

#define ACTIVATE_PERIOD_FOREVER (99999)
	// A smaller but still very large number of seconds which is essentially forever.
	// Powers which only have ATTRIBMOD_DURATION_FOREVER mods should have an activation period
	// of this or greater.

/***************************************************************************/
/***************************************************************************/

typedef enum ModApplicationType
{
	kModApplicationType_OnTick,			// while the power is running
	kModApplicationType_OnActivate,		// when the power is turned on
	kModApplicationType_OnDeactivate,	// when the power is turned off
	kModApplicationType_OnExpire,		// a limited version of onDeactivate

	// ARM NOTE:  There are some strange edge cases around character_Reset() and related
	//   functionality where enable/disable appear to be triggering more often than they should,
	//   and in improper order.  At some point we're going to need to fix those edge cases,
	//   but I've invested enough time into this right now that I'm just going to let them be.
	//   For current usage (just granting a power on enable and revoking the power on disable),
	//   the current code appears to have the correct output, even if it doesn't get there in
	//   any understandable way...
	kModApplicationType_OnEnable,		// when the power becomes able to be turned on
	kModApplicationType_OnDisable,		// when the power becomes no longer able to be turned on
	
	// Keep this last
	kModApplicationType_Count,
} ModApplicationType;

typedef enum ModTarget
{
	kModTarget_Caster,
	kModTarget_CastersOwnerAndAllPets,
	kModTarget_Focus,
	kModTarget_FocusOwnerAndAllPets,
	kModTarget_Affected,
	kModTarget_AffectedsOwnerAndAllPets,
	kModTarget_Marker,

	// Keep this last
	kModTarget_Count,
} ModTarget;

typedef enum ModType
{
	kModType_Duration,
	kModType_Magnitude,
	kModType_Constant,
	kModType_Expression,
	kModType_SkillMagnitude,

	// Keep this last
	kModType_Count,
} ModType;

typedef enum CasterStackType
{
	kCasterStackType_Individual,	// Stacking is handled for each caster individually
	kCasterStackType_Collective,	// Stacking is handled for all casters collectively

	// Keep this last
	kCasterStackType_Count
} CasterStackType;

typedef enum StackType
{
	// Determines how multiple identical AttribMods from the same power and
	//   caster are handled.

	kStackType_Stack,			// Stack up (allow multiples)
	kStackType_Ignore,			// Ignore the new duplicate (do nothing)
	kStackType_Extend,			// Update the parameters in and extend the existing AttribMod
	kStackType_Replace,			// Update the parameters and replace the existing AttribMod
	kStackType_Overlap,			// Update the parameters in the existing AttribMod, don't extend the duration
	kStackType_StackThenIgnore,	// Stack up to StackCount times (if count < StackCount, then stack, else ignore)
	kStackType_Refresh,			// Update the duration in all copies of the existing matching AttribMods, then add a new copy on
	kStackType_RefreshToCount,	// If count < StackCount, then Refresh and add a new copy, else just Refresh
	kStackType_Maximize,		// If mag is greater Replace, else Ignore
	kStackType_Suppress,		// Keep all, but suppress all but the greatest magnitude

	// Keep this last
	k_StackType_Count
} StackType;

typedef enum PowerEvent
{
	kPowerEvent_Activate,    // Must be first invoke-related event
	kPowerEvent_ActivateAttackClick,
	kPowerEvent_Attacked,
	kPowerEvent_AttackedNoException,
	kPowerEvent_Helped,
	kPowerEvent_Hit,
	kPowerEvent_Miss,
	kPowerEvent_EndActivate, // Must be last invoke-related event

	kPowerEvent_AttackedByOther, // Must be the first apply-related event
	kPowerEvent_AttackedByOtherClick,
	kPowerEvent_HelpedByOther,
	kPowerEvent_HitByOther,
	kPowerEvent_HitByFriend,
	kPowerEvent_HitByFoe,
	kPowerEvent_MissByOther,
	kPowerEvent_MissByFriend,
	kPowerEvent_MissByFoe,       // Must be the last apply-related event

	kPowerEvent_Damaged,
	kPowerEvent_Healed,

	kPowerEvent_Stunned,     // Must be first status event for macro below
	kPowerEvent_Immobilized,
	kPowerEvent_Held,
	kPowerEvent_Sleep,
	kPowerEvent_Terrorized,
	kPowerEvent_Confused,
	kPowerEvent_Untouchable,
	kPowerEvent_Intangible,
	kPowerEvent_OnlyAffectsSelf,
	kPowerEvent_AnyStatus,   // Leave as last status event

	kPowerEvent_Knocked,

	kPowerEvent_Defeated,

	kPowerEvent_MissionObjectClick,

	kPowerEvent_Moved,

	kPowerEvent_Defiant,

	// Keep this last
	kPowerEvent_Count
} PowerEvent;

#define POWEREVENT_STATUS(x) ((x)>=kPowerEvent_Stunned && (x)<kPowerEvent_AnyStatus)
#define POWEREVENT_INVOKE(x) ((x)>=kPowerEvent_Activate && (x)<=kPowerEvent_EndActivate)
#define POWEREVENT_APPLY(x) ((x)>=kPowerEvent_AttackedByOther && (x)<=kPowerEvent_MissByFoe)

/***************************************************************************/
/***************************************************************************/

typedef enum AttribModFlag {
	// --- general flags for all attribmods ---
	AttribModFlag_NoFloaters,
		// If set, hides floaters (damage and healing numbers, for example)
		//   over the affected's head. If specified, pchDisplayFloat is always
		//   shown, even if this is set.

	AttribModFlag_BoostIgnoreDiminishing,
		// Determines whether this attribmod ignores diminishing returns on boosts
		//   (aka Enhancement Diversification). Only used if aspect is Strength.

	AttribModFlag_CancelOnMiss,
		// If set and the test governed by fTickChance fails, the attribmod
		//   will be removed.

	AttribModFlag_NearGround,
		// If set, only applies the attrib modifier if the target is on
		//   the ground.

	AttribModFlag_IgnoreStrength,
		// If set, the attacker's strength is not used to modify the scale
		//   of the effects.

	AttribModFlag_IgnoreResistance,
		// If set, the target's resistance is not used to modify the scale
		//   of the applied effects.

	AttribModFlag_IgnoreCombatMods,
		// If set, the level difference between the source and the target
		//   is not used to modify the effect's magnitude and/or duration.

	AttribModFlag_ResistMagnitude,
	AttribModFlag_ResistDuration,
	AttribModFlag_CombatModMagnitude,
	AttribModFlag_CombatModDuration,
		// If one of these is set, forces resistance and/or combat mods
		//   to apply to something other than the default (based on the type
		//   of the attribmod).

	AttribModFlag_Boost,
		// If true, this is supposed to be a boost template.  This is used
		//   in boosts and powers used as set bonuses, which can include both
		//   boost templates and additional normal templates.

	AttribModFlag_HideZero,
		// If true and pchDisplayFloat, pchDisplayAttackerHit or pchDisplayVictimHit
		//	is specified, it will only display if the attribute evaluates to a non-zero value.

	AttribModFlag_KeepThroughDeath,
		// If true, do not clean out this attribmod when the entity dies.

	AttribModFlag_DelayEval,
		// If true, delay any evaluations associated with this attribmod until the last
		//   possible moment in mod_process.
		// This means you will have to store all the Eval stashes until that time comes.
		//   Note that this can cause desynchronization with other members of the same
		//   effect group.

	AttribModFlag_NoHitDelay,
		// Do not add the FramesBeforeHit delay from the power to this attribmod.

	AttribModFlag_NoProjectileDelay,
		// Do not add the projectile distance delay from the power to this attribmod.

	AttribModFlag_StackByAttribAndKey,
		// When using StackKey stacking, also compare the Aspect/Attribute in
		// addition to the key.

	AttribModFlag_StackExactPower,
		// Apply stacking rules per power instance rather than by the template.
		// Implies individual caster stacking.

	AttribModFlag_IgnoreSuppressErrors,
		// Designer laziness flag.

	AttribModFlag_LastGeneralFlag,
		// End of non-effect-specific flags, marker for static assert.

	// --- effect-specific flags that depend on the Attrib ---

	AttribModFlag_EffectSpecific01			= 32,
	AttribModFlag_EffectSpecific02,
	AttribModFlag_EffectSpecific03,
	AttribModFlag_EffectSpecific04,
	AttribModFlag_EffectSpecific05,
	AttribModFlag_EffectSpecific06,
	AttribModFlag_EffectSpecific07,
	AttribModFlag_EffectSpecific08,

	// ------------------------- Effect Specific Flag 1 -------------------------
	AttribModFlag_VanishEntOnTimeout		= AttribModFlag_EffectSpecific01,
		// Valid for: EntCreate
		// If true, if the pet times out or is otherise destroyed by the
		//   server (as opposed to being defeated) then the entity is
		//   vanished as opposed to going through the usual DieNow code.
		// (Only for powers which spawn entities.)

	AttribModFlag_DoNotDisplayShift			= AttribModFlag_EffectSpecific01,
		// Valid for: CombatModShift
		// Causes this mod shift not to be added to the total reported to
		//   the client.

	AttribModFlag_NoTokenTime				= AttribModFlag_EffectSpecific01,
		// Valid for: TokenAdd, TokenSet
		// Don't update the token timer.

	AttribModFlag_RevokeAll                 = AttribModFlag_EffectSpecific01,
	    // Valid for: RevokePower
		// Revokes all copies of the power, ignoring Count.

	// ------------------------- Effect Specific Flag 2 -------------------------
	AttribModFlag_DoNotTintCostume			= AttribModFlag_EffectSpecific02,
		// Valid for: EntCreate
		// If true, do not apply custom tinting to the spawned pet's costume.

	// ------------------------- Effect Specific Flag 3 -------------------------
	AttribModFlag_CopyBoosts				= AttribModFlag_EffectSpecific03,
		// Valid for: ExecutePower, EntCreate
		// Copy enhancements to the resulting power(s) if they are accepted
		//   by its allowed types.

	// ------------------------- Effect Specific Flag 4 -------------------------
	AttribModFlag_CopyCreatorMods			= AttribModFlag_EffectSpecific04,
		// Valid for: EntCreate
		// Copy strength buff mods from the creator of this entity.

	// ------------------------- Effect Specific Flag 5 -------------------------
	AttribModFlag_NoCreatorModFX			= AttribModFlag_EffectSpecific05,
		// Valid for: EntCreate
		// Suppresses FX on mods copied from creator. Only has an effect if
		//   CopyCreatorMods is also set.

	// ------------------------- Effect Specific Flag 6 -------------------------
	AttribModFlag_PseudoPet					= AttribModFlag_EffectSpecific06,
		// Valid for: EntCreate
		// Ignores pchVillainDef and pchClass, creates a generic entity the
		//   same class as its creator. Implies NoCreatorModFX.

	// ------------------------- Effect Specific Flag 7 -------------------------
	AttribModFlag_PetVisible				= AttribModFlag_EffectSpecific07,
		// Valid for: EntCreate
		// Forces the summoned entity to show up in a player's pet window.

	// ------------------------- Effect Specific Flag 8 -------------------------
	AttribModFlag_PetCommandable			= AttribModFlag_EffectSpecific08,
		// Valid for: EntCreate
		// Forces the summoned entity to be commandable like a mastermind pet.

	AttribModFlag_Count
} AttribModFlag;

// Change this when adding more bytes to the flag array
#define ATTRIBMOD_FLAGS_SIZE 2
STATIC_ASSERT(AttribModFlag_LastGeneralFlag <= AttribModFlag_EffectSpecific01);
STATIC_ASSERT(AttribModFlag_Count <= (ATTRIBMOD_FLAGS_SIZE * 32));

// Always use this
#define TemplateHasFlag(pModTemplate, flagName) ( ((pModTemplate)->iFlags[AttribModFlag_##flagName / 32] & (1 << (AttribModFlag_##flagName % 32))) != 0 )
#define TemplateSetFlag(pModTemplate, flagName) ( (pModTemplate)->iFlags[AttribModFlag_##flagName / 32] |= (1 << (AttribModFlag_##flagName % 32)) )

/***************************************************************************/
/***************************************************************************/

typedef struct SuppressPair
{
	int idxEvent;
		// The index of the event to check. (See PowerEvent enum in character_base.h)

	U32 ulSeconds;
		// How many seconds it must be after the event before this attribmod
		//   is allowed to go off.

	bool bAlways;
		// If true, the attribmod will always be suppressed when in the
		//  event window. If false, then if the attribmod has already been
		//  applied once, it continue to gets applied.

} SuppressPair;

typedef enum EffectGroupFlag {
	EffectGroupFlag_PVEOnly,
		// If true, this effect group is ignored while on PVP maps.

	EffectGroupFlag_PVPOnly,
		// If true, this effect group is ignored while on PVE maps.

	EffectGroupFlag_Fallback,
		// Fallback effect groups are normally ignored. If no non-fallback effect
		// groups within the same collection are eligible to be applied

	EffectGroupFlag_LinkedChance,
		// If true, ensure that the Chance is always evaluated consistently for
		// all LinkedChance effect groups from the same power activation.
		// DEPRECATED: Will be removed in a future version.

	EffectGroupFlag_Count
} EffectGroupFlag;
// Change this when adding more bytes to the flag array
STATIC_ASSERT(EffectGroupFlag_Count <= 32);

// Always use this so that if we get too many flags and need to move to a flagarray
// it won't be painful to switch over.
#define EFFECTGROUP_FLAG_VALUE(flagName) ( 1 << (EffectGroupFlag_##flagName) )
#define EffectHasFlag(pEffectGroup, flagName) ( ((pEffectGroup)->iFlags & (EFFECTGROUP_FLAG_VALUE(flagName))) != 0 )

typedef struct AttribModTemplate AttribModTemplate;
typedef struct EffectGroup EffectGroup;

typedef struct EffectGroup
{
	// An effect group is a group of AttribMod templates that are always applied
	// together.

	const char **ppchTags;
		// Effect tags (for chance mods, etc)

	float fChance;
		// The chance that this attrib modifier will be applied to the
		//   target. (1.0 == 100%)

	float fProcsPerMinute;
		// If set, this will cause the chance to activate to be calculated automatically
		//   to result in the approximate number of procs per minute.

	float fDelay;
		// How long to wait before applying the attrib modifier for the
		//   first time.

	float fRadiusInner;
	float fRadiusOuter;
		// If set, these values define the inner and outer radii of a spherical shell
		//   that limits the targets affected by this effect group. An inner radius of 0
		//   indicates a plain sphere. Setting both to 0 limits it to the main target
		//   only.

	const char **ppchRequires;
		// An expression which describes the conditions under which the templates
		//   in the effect group may be applied. If there is no expression, then
		//   the templates are always applied.

	U32 iFlags;
		// Boolean flags for this effect group.

	const AttribModTemplate **ppTemplates;
		// AttribMod templates to be applied by this effect group.

	const EffectGroup **ppEffects;
		// Child effect groups.

	// --- bin-only derived fields ---
	U32	evalFlags;
		// Flags created at bin time based upon what special combat eval parameters
		//   need to be pushed for evaluation.

	// --- runtime fields ---
	const BasePower *ppowBase;
		// The power which contains this effect group

	const EffectGroup *peffParent;
		// Pointer to the parent effect group.
} EffectGroup;

/* Attribmod ****************************************************************/

// Messages

typedef struct AttribModMessages
{
	const char *pchDisplayAttackerHit;
		// Message displayed to the attacker when he hits with this power.

	const char *pchDisplayVictimHit;
		// Message displayed to the victim when he gets hits with this power.

	const char *pchDisplayFloat;
		// Message displayed over the victim's head when this attrib mod
		//   goes off.

	const char *pchDisplayDefenseFloat;
		// Message displayed over the victim's head when this attrib mod
		//   is the defense that caused some attack to miss the victim.
} AttribModMessages;

// FX

typedef struct AttribModFX
{
	const int *piContinuingBits;
		// Sets the given bits for the lifetime of the AttribMod.

	const char *pchContinuingFX;
		// If non-NULL, plays maintained FX for the lifetime of the AttribMod.
		//   When it times out, the FX is killed.

	const int *piConditionalBits;
		// Sets the given bits while the AttribMod is alive and the attrCur
		//   for the modified attribute is greater than zero.

	const char *pchConditionalFX;
		// If non-NULL, plays maintained FX while the AttribMod is alive and
		//   the attrCur for the modified attribute is greater than zero.
} AttribModFX;

// Extended Targeting Info

typedef struct AttribModTargetInfo
{
	const char **ppchMarkerNames;
	int *piMarkerCount;
} AttribModTargetInfo;

// A PowerSpec is a list of categories, powersets, and individual powers
// in a convenient package that can be parsed.
// It's here instead of powers.h because some Params need it fully realized,
// and this file gets included by powers.h anyway.

typedef struct PowerSpec
{
	const char **ppchCategoryNames;
	const char **ppchPowersetNames;
	const char **ppchPowerNames;

	// --- runtime only
	// can't use TOK_LINK for these because the powers dict itself can use this
	const BasePower **ppPowers;
} PowerSpec;

#define ParsePowerSpecInternal(type, prefix) \
	{ "PowerCategory",   TOK_STRINGARRAY(type, prefix##ppchCategoryNames) }, \
	{ "Powerset",        TOK_STRINGARRAY(type, prefix##ppchPowersetNames) }, \
	{ "Power",           TOK_STRINGARRAY(type, prefix##ppchPowerNames) }, \
	{ "",                TOK_NO_BIN|TOK_AUTOINTEARRAY(type, prefix##ppPowers ) },

#define ParsePowerSpecEmbed(type, var) ParsePowerSpecInternal(type, var##.)
extern ParseTable ParsePowerSpec[];

// Params

typedef enum AttribModParamType {
	ParamType_None,
	ParamType_Costume,
	ParamType_Reward,
	ParamType_EntCreate,
	ParamType_Power,
	ParamType_Phase,
	ParamType_Teleport,
	ParamType_Behavior,
	ParamType_SZEValue,
	ParamType_Token,
	ParamType_EffectFilter,
} AttribModParamType;

// Effect-specific data

// *** Param: Costume ***
typedef struct cCostume cCostume;
typedef struct AttribModParam_Costume {
	AttribModParamType eType;				// must be first for dynamicstruct
	int iPriority;

	// --- runtime only
	int npcIndex;
	const cCostume *npcCostume;
} AttribModParam_Costume;

typedef struct AttribModParam_Costume_loadonly {
	const char *pchCostumeName;
} AttribModParam_Costume_loadonly;
extern ParseTable ParseAttribModParam_Costume[];

// *** Param: Reward ***
typedef struct AttribModParam_Reward {
	AttribModParamType eType;
	const char **ppchRewards;
} AttribModParam_Reward;
extern ParseTable ParseAttribModParam_Reward[];

// *** Param: EntCreate ***
typedef struct VillainDef VillainDef;
typedef struct AttribModParam_EntCreate {
	AttribModParamType eType;
	const char *pchDisplayName;
	const char *pchPriorityList;
	const char *pchAIConfig;
	PowerSpec ps;
		// For powers which spawn entities, the name of the priority list
		//   the new entity will run.

	// --- runtime only
	const VillainDef *pVillain;
	const CharacterClass *pClass;
	int npcIndex;
	const cCostume *npcCostume;
	const char *pchDoppelganger;
} AttribModParam_EntCreate;

typedef struct AttribModParam_EntCreate_loadonly {
	const char *pchEntityDef;
		// For powers which spawn entities, the name of the VillainDef which
		//   defines the basic entity being spawned.
	const char *pchClass;
		// If EntityDef is not specified, a Class of which to create an entity
		//   must be.
	const char *pchCostumeName;
		// Overrides the costume for the spawned entity.
} AttribModParam_EntCreate_loadonly;
extern ParseTable ParseAttribModParam_EntCreate[];

// *** Param: Power ***
typedef struct AttribModParam_Power {
	AttribModParamType eType;
	PowerSpec ps;
	int iCount;
} AttribModParam_Power;
extern ParseTable ParseAttribModParam_Power[];

// *** Param: Phase ***
typedef struct AttribModParam_Phase {
	AttribModParamType eType;				// must be first for dynamicstruct
	int *piCombatPhases;
	int *piVisionPhases;
	int iExclusiveVisionPhase;
} AttribModParam_Phase;

typedef struct AttribModParam_Phase_loadonly {
	// --- text only
	const char **ppchCombatPhaseNames;
	const char **ppchVisionPhaseNames;
	const char *pchExclusiveVisionPhaseName;
} AttribModParam_Phase_loadonly;
extern ParseTable ParseAttribModParam_Phase[];

// *** Param: Teleport ***
typedef struct AttribModParam_Teleport {
	AttribModParamType eType;
	const char *pchDestination;
} AttribModParam_Teleport;
extern ParseTable ParseAttribModParam_Teleport[];

// *** Param: Behavior ***
typedef struct AttribModParam_Behavior {
	AttribModParamType eType;
	const char **ppchBehaviors;
} AttribModParam_Behavior;
extern ParseTable ParseAttribModParam_Behavior[];

// *** Param: SZEValue ***
typedef struct AttribModParam_SZEValue {
	AttribModParamType eType;
	const char **ppchScriptID;
	const char **ppchScriptValue;
} AttribModParam_SZEValue;
extern ParseTable ParseAttribModParam_SZEValue[];

// *** Param: Token ***
typedef struct AttribModParam_Token {
	AttribModParamType eType;
	const char **ppchTokens;
} AttribModParam_Token;
extern ParseTable ParseAttribModParam_Token[];

// *** Param: EffectFilter ***
typedef struct AttribModParam_EffectFilter {
	AttribModParamType eType;
	PowerSpec ps;
	const char **ppchTags;
} AttribModParam_EffectFilter;
extern ParseTable ParseAttribModParam_EffectFilter[];
bool effectfilter_Test(const EffectGroup *effect, const AttribModParam_EffectFilter *filter);

// Template

typedef struct AttribModTemplate
{
	// This defines an actual effect of a power. A power may have multiple
	//   AttribModTemplates. When a power is used, these AMTemplates are
	//   pared down to AttribMods and attached to the targeted character.

	size_t *piAttrib;
		// Array of byte offsets to the attribute in the CharacterAttributes struct.
	size_t offAspect;
		// Byte offset to the structure in the Character to the CharacterAttributes
		//   to modify

	ModApplicationType eApplicationType;
		// Determines when this attrib mod is applied during the lifecycle of a power.
	ModType eType;
		// Determines if the duration or the magnitude is what is calculated.
	ModTarget eTarget;
		// Who is the target of this particular modifier.
	const AttribModTargetInfo *pTargetInfo;
		// Extra targeting info for this mod.

	const char *pchTable;
		// The name of the table to use for scaling the power by level.
		// Tables are defined in the class.
	float fScale;
		// How much to scale the basic value given by the class table for
		//   the given attribute.
	float fDuration;
		// How long the effect lasts on the target. Booleans calculate this
		//   value, others use it directly.
	float fMagnitude;
		// Default for how much to change the attribute. Booleans use this
		//   value, others calculate it.
	const char **ppchDuration;
		// An expression which calculates the duration of the attribmod.
		//   If empty, the fDuration field is used instead.
	const char **ppchMagnitude;
		// An expression which calculates the magnitude of the attribmod. 
		//   If empty, the fMagnitude field is used instead.
		//   Only used for ModType_Expression.

	float fDelay;
		// How long to wait before applying the attrib modifier for the
		//   first time. Stacks with the delay from the parent EffectGroup.
	float fPeriod;
		// The attrib modifier is applied every fPeriod seconds.
	float fTickChance;
		// If less than 1.0, the chance for an individual tick to apply.
	const char **ppchDelayedRequires;
		// An expression which describes the conditions under which this template
		//   is applied. Delayed Requires are checked "just in time" (right before
		//   the mod ticks for the first time) and cancel the mod if the expression
		//   evaluates to false.

	CasterStackType eCasterStack;
		// Determines how identical AttribMods from the same power
		//   but from different casters are handled.
	StackType eStack;
		// Determines how multiple AttribMods that pass the CasterStackType check are handled.
	int iStackLimit;
		// Used for kStackType_StackThenIgnore.
		// Determines how many times the AttribMod should stack before it is ignored.
	int iStackKey;
		// If this is not zero, we compare this instead of stacking by the template.

	const int *piCancelEvents;
		// A list of PowerEvents which will cancel this attribmod outright.
	const SuppressPair **ppSuppress;
		// An earray of events to check against to determine if this attribmod
		//   is allowed to go off. This doesn't reject the attribmod entirely
		//   (like the Requires does). If the time passes after the event and
		//   the attribmod still has time left on it, it will work.

	BoostType boostModAllowed;
		// If specified, a power must specifically allow this BoostType in order for this mod to apply as part of a Boost, even if the Boost itself applies
		// This is for Boosts with mixed BoostTypes such as Hami-Os where Damage boosts can be slotted into Damage Resist powers for exploitative gain

	U32 iFlags[ATTRIBMOD_FLAGS_SIZE];
		// Boolean flags for this attribmod

	const AttribModMessages *pMessages;
	const AttribModFX *pFX;
		// Some items moved to optional structures.

	const void *pParams;
		// Effect specific parameters

	// --- runtime fields ---
	const EffectGroup *peffParent;
		// Pointer to the parent effect group.

	const BasePower *ppowBase;
		// The power which contains this attribmod

	int ppowBaseIndex;
		// The index into the power's list of attrib mods that corresponds to this mod

	int iVarIndex;
		// If >=0, then use a instance-specific value instead of the table.

	bool bSuppressMethodAlways;
		// Determined from the bAlways tags in ppSuppress.  If all suppressions
		//   have a matching bAlways tag, we set this appropriately.  If there
		//   is a mix, we complain loudly.

} AttribModTemplate;

// Some helper functions to deal with Attrib now being an array.
static INLINEDBG bool TemplateHasAttrib(const AttribModTemplate *ptemplate, size_t offAttrib) {
	int i;
	for (i = eaiSize((int**)&ptemplate->piAttrib) - 1; i >= 0; i--) {
		if (ptemplate->piAttrib[i] == offAttrib)
			return true;
	}
	return false;
}
static INLINEDBG bool TemplateHasAttribBetween(const AttribModTemplate *ptemplate, size_t lower, size_t upper) {
	int i;
	for (i = eaiSize((int**)&ptemplate->piAttrib) - 1; i >= 0; i--) {
		if (ptemplate->piAttrib[i] >= lower && ptemplate->piAttrib[i] <= upper)
			return true;
	}
	return false;
}
static INLINEDBG bool TemplateHasSpecialAttribs(const AttribModTemplate *ptemplate) {
	int i;
	for (i = eaiSize((int**)&ptemplate->piAttrib) - 1; i >= 0; i--) {
		if (ptemplate->piAttrib[i] >= sizeof(CharacterAttributes))
			return true;
	}
	return false;
}

static INLINEDBG bool _TemplateHasAttribFromList_internal(const AttribModTemplate *ptemplate, ...) {
	int i;
	bool ret = false;
	size_t offAttrib;
	va_list arglist;
	va_start(arglist, ptemplate);

	while ( (offAttrib = va_arg(arglist, int)) != (size_t)-1 ) {
		for (i = eaiSize((int**)&ptemplate->piAttrib) - 1; i >= 0; i--) {
			if (ptemplate->piAttrib[i] == offAttrib) {
				ret = true;
				goto out;
			}
		}
	}

out:
	va_end(arglist);
	return ret;
}

#define TemplateHasAttribFromList(...) _TemplateHasAttribFromList_internal(__VA_ARGS__,(size_t)-1)

#define TemplateGetSub(t, s, m) ( (t->s) ? (t->s->m) : NULL )
#define TemplateGetParam(t, p, m) ( (t->pParams && (*(int*)t->pParams == ParamType_##p)) ? (((const AttribModParam_##p*)t->pParams)->m) : NULL )
#define TemplateGetParams(t, p) ( (t->pParams && (*(int*)t->pParams == ParamType_##p)) ? ((const AttribModParam_##p*)t->pParams) : NULL )

// For compatibility with old powerdef files
typedef struct LegacyAttribModTemplate {
	EffectGroup eg;
	AttribModTemplate am;
	AttribModMessages msgs;
	AttribModFX fx;

	// Legacy parameters moved elsewhere or with changed semantics
	size_t offAttrib;
	float fChance;

	bool bShowFloaters;
	bool bIgnoresBoostDiminishing;
	bool bCancelOnMiss;
	bool bNearGround;
	bool bAllowStrength;
	bool bAllowResistance;
	bool bAllowCombatMods;
	bool bBoostTemplate;
	bool bMatchExactPower;
	bool bDisplayTextOnlyIfNotZero;
	bool bVanishEntOnTimeout;
	bool bDoNotTintCostume;
	bool bKeepThroughDeath;
	bool bDelayEval;
	bool bLinkedChance;
	bool bStackByAttrib;

	// these were never really bools...
	bool bUseDurationCombatMods;
	bool bUseDurationResistance;
	bool bUseMagnitudeCombatMods;
	bool bUseMagnitudeResistance;

	const char *pchIgnoreSuppressErrors;

	const char *pchCostumeName;
	const char *pchReward;
	const char *pchParams;
	const char *pchEntityDef;
	const char *pchPriorityList;
	const char **ppchPrimaryStringList;
	const char **ppchSecondaryStringList;
} LegacyAttribModTemplate;

// AtrribModTemplate evalFlags
#define ATTRIBMOD_EVAL_DELAY		(1 << 0)
#define ATTRIBMOD_EVAL_HIT_ROLL		(1 << 1)
#define ATTRIBMOD_EVAL_CALC_INFO	(1 << 2)
#define ATTRIBMOD_EVAL_CUSTOMFX		(1 << 3)
#define ATTRIBMOD_EVAL_TARGETSHIT	(1 << 4)
#define ATTRIBMOD_EVAL_CHANCE		(1 << 5)
#define ATTRIBMOD_EVAL_CHAIN		(1 << 6)

typedef struct ModFillContext {
	Power *pPow;
	Character *pPrevTarget;
	unsigned int uiInvokeID;
	Vec3 vec;
	float fTimeBeforeHit;
	float fDelay;
	float fEffectiveness;
	float fToHitRoll;
	float fToHit;
	float fTickChance;
	int iChainJump;
	float fChainEff;
} ModFillContext;

#ifdef ATTRIBMOD_PARSE_INFO_DEFINITIONS

extern StaticDefine ParsePowerDefines[]; // defined in load_def.c

//
// All these things with ks on the front probably don't need them any more.
//   This is a leftover from when they were globally defined and they needed
//   to be mutually exclusive.
//

StaticDefineInt ModApplicationEnum[] =
{
	DEFINE_INT
	{ "kOnTick",				kModApplicationType_OnTick			},
	{ "kOnActivate",			kModApplicationType_OnActivate		},
	{ "kOnDeactivate",			kModApplicationType_OnDeactivate	},
	{ "kOnExpire",				kModApplicationType_OnExpire		},
	{ "kOnEnable",				kModApplicationType_OnEnable		},
	{ "kOnDisable",				kModApplicationType_OnDisable		},
	// The following are deprecated, please use the above instead.
	{ "kOnIncarnateEquip",		kModApplicationType_OnEnable		},
	{ "kOnIncarnateUnequip",	kModApplicationType_OnDisable		},
	DEFINE_END
};

StaticDefineInt DurationEnum[] =
{
	DEFINE_INT
	{ "kInstant",            -1  },
	{ "kUntilKilled",        ATTRIBMOD_DURATION_FOREVER },
	{ "kUntilShutOff",       ATTRIBMOD_DURATION_FOREVER },
	DEFINE_END
};

StaticDefineInt ModTargetEnum[] =
{
	DEFINE_INT
	{ "kCaster",						kModTarget_Caster						},
	{ "kCastersOwnerAndAllPets",		kModTarget_CastersOwnerAndAllPets		},
	{ "kFocus",							kModTarget_Focus						},
	{ "kFocusOwnerAndAllPets",			kModTarget_FocusOwnerAndAllPets			},
	{ "kAffected",						kModTarget_Affected						},
	{ "kAffectedsOwnerAndAllPets",		kModTarget_AffectedsOwnerAndAllPets		},
	{ "kMarker",						kModTarget_Marker						},
	// the below are deprecated, please use the above from now on...
	{ "kSelf",							kModTarget_Caster						},
	{ "kSelfsOwnerAndAllPets",			kModTarget_CastersOwnerAndAllPets		},
	{ "kTarget",						kModTarget_Affected						},
	{ "kTargetsOwnerAndAllPets",		kModTarget_AffectedsOwnerAndAllPets		},
	DEFINE_END
};

StaticDefineInt ModTypeEnum[] =
{
	DEFINE_INT
	{ "kDuration",           kModType_Duration        },
	{ "kMagnitude",          kModType_Magnitude       },
	{ "kConstant",           kModType_Constant        },
	{ "kExpression",         kModType_Expression      },
	{ "kSkillMagnitude",     kModType_SkillMagnitude  },
	DEFINE_END
};

StaticDefineInt CasterStackTypeEnum[] =
{
	DEFINE_INT
	{ "kIndividual",		kCasterStackType_Individual	},
	{ "kCollective",		kCasterStackType_Collective },
	DEFINE_END
};

StaticDefineInt StackTypeEnum[] =
{
	DEFINE_INT
	{ "kStack",				kStackType_Stack   },
	{ "kIgnore",			kStackType_Ignore  },
	{ "kExtend",			kStackType_Extend  },
	{ "kReplace",			kStackType_Replace },
	{ "kOverlap",			kStackType_Overlap },
	{ "kStackThenIgnore",	kStackType_StackThenIgnore },
	{ "kRefresh",			kStackType_Refresh },
	{ "kRefreshToCount",	kStackType_RefreshToCount },
	{ "kMaximize",			kStackType_Maximize },
	{ "kSuppress",			kStackType_Suppress },
	DEFINE_END
};

StaticDefineInt AspectEnum[] =
{
	DEFINE_INT
	// Aspects
	{ "kCurrent",            offsetof(CharacterAttribSet, pattrMod)           },
	{ "kMaximum",            offsetof(CharacterAttribSet, pattrMax)           },
	{ "kStrength",           offsetof(CharacterAttribSet, pattrStrength)      },
	{ "kResistance",         offsetof(CharacterAttribSet, pattrResistance)    },
	{ "kAbsolute",           offsetof(CharacterAttribSet, pattrAbsolute)      },
	{ "kCurrentAbsolute",    offsetof(CharacterAttribSet, pattrAbsolute)      },
	{ "kCur",                offsetof(CharacterAttribSet, pattrMod)           },
	{ "kMax",                offsetof(CharacterAttribSet, pattrMax)           },
	{ "kStr",                offsetof(CharacterAttribSet, pattrStrength)      },
	{ "kRes",                offsetof(CharacterAttribSet, pattrResistance)    },
	{ "kAbs",                offsetof(CharacterAttribSet, pattrAbsolute)      },
	{ "kCurAbs",             offsetof(CharacterAttribSet, pattrAbsolute)      },
	DEFINE_END
};

StaticDefineInt ModBoolEnum[] =
{
	DEFINE_INT
	{ "false",               0  },
	{ "kFalse",              0  },
	{ "true",                1  },
	{ "kTrue",               1  },
	DEFINE_END
};

StaticDefineInt PowerEventEnum[] =
{
	DEFINE_INT
	{ "Activate",             kPowerEvent_Activate        },
	{ "ActivateAttackClick",  kPowerEvent_ActivateAttackClick },
	{ "EndActivate",          kPowerEvent_EndActivate     },

	{ "Hit",                  kPowerEvent_Hit             },
	{ "HitByOther",           kPowerEvent_HitByOther      },
	{ "HitByFriend",          kPowerEvent_HitByFriend     },
	{ "HitByFoe",             kPowerEvent_HitByFoe        },
	{ "Miss",                 kPowerEvent_Miss            },
	{ "MissByOther",          kPowerEvent_MissByOther     },
	{ "MissByFriend",         kPowerEvent_MissByFriend    },
	{ "MissByFoe",            kPowerEvent_MissByFoe       },
	{ "Attacked",             kPowerEvent_Attacked        },
	{ "AttackedNoException",  kPowerEvent_AttackedNoException  },

	{ "AttackedByOther",      kPowerEvent_AttackedByOther },
	{ "AttackedByOtherClick", kPowerEvent_AttackedByOtherClick },
	{ "Helped",               kPowerEvent_Helped          },
	{ "HelpedByOther",        kPowerEvent_HelpedByOther   },

	{ "Damaged",              kPowerEvent_Damaged         },
	{ "Healed",               kPowerEvent_Healed          },

	{ "Stunned",              kPowerEvent_Stunned         },
	{ "Stun",                 kPowerEvent_Stunned         },
	{ "Immobilized",          kPowerEvent_Immobilized     },
	{ "Immobilize",           kPowerEvent_Immobilized     },
	{ "Held",                 kPowerEvent_Held            },
	{ "Sleep",                kPowerEvent_Sleep           },
	{ "Terrorized",           kPowerEvent_Terrorized      },
	{ "Terrorize",            kPowerEvent_Terrorized      },
	{ "Confused",             kPowerEvent_Confused        },
	{ "Confuse",              kPowerEvent_Confused        },
	{ "Untouchable",          kPowerEvent_Untouchable     },
	{ "Intangible",           kPowerEvent_Intangible      },
	{ "OnlyAffectsSelf",      kPowerEvent_OnlyAffectsSelf },
	{ "AnyStatus",            kPowerEvent_AnyStatus       },

	{ "Knocked",              kPowerEvent_Knocked         },

	{ "Defeated",             kPowerEvent_Defeated        },

	{ "MissionObjectClick",   kPowerEvent_MissionObjectClick },

	{ "Moved",                kPowerEvent_Moved   },

	{ "Defiant",              kPowerEvent_Defiant },

	DEFINE_END
};

StaticDefineInt SupressAlwaysEnum[] =
{
	DEFINE_INT
	{ "WhenInactive",       0  },
	{ "Always",             1  },
	DEFINE_END
};

TokenizerParseInfo ParseSuppressPair[] =
{
	{ "Event",      TOK_STRUCTPARAM|TOK_INT(SuppressPair, idxEvent, 0), PowerEventEnum  },
	{ "Seconds",    TOK_STRUCTPARAM|TOK_INT(SuppressPair, ulSeconds, 0) },
	{ "Always",     TOK_STRUCTPARAM|TOK_BOOL(SuppressPair, bAlways, 0), SupressAlwaysEnum },
	{ "\n",         TOK_END,                         0 },
	{ "", 0, 0 }
};

#define DEFINE_FLAG(x) { #x, EFFECTGROUP_FLAG_VALUE(x) }
StaticDefineInt ParseEffectGroupFlag[] = {
	DEFINE_INT
	DEFINE_FLAG(PVEOnly),
	DEFINE_FLAG(PVPOnly),
	DEFINE_FLAG(Fallback),
//	DEFINE_FLAG(LinkedChance),
	DEFINE_END
};
#undef DEFINE_FLAG

#define DEFINE_FLAG(x) { #x, AttribModFlag_##x }
StaticDefineInt ParseAttribModFlag[] = {
	DEFINE_INT
	DEFINE_FLAG(NoFloaters),
	DEFINE_FLAG(BoostIgnoreDiminishing),
	DEFINE_FLAG(CancelOnMiss),
	DEFINE_FLAG(NearGround),
	DEFINE_FLAG(IgnoreStrength),
	DEFINE_FLAG(IgnoreResistance),
	DEFINE_FLAG(IgnoreCombatMods),
	DEFINE_FLAG(ResistMagnitude),
	DEFINE_FLAG(ResistDuration),
	DEFINE_FLAG(CombatModMagnitude),
	DEFINE_FLAG(CombatModDuration),
	DEFINE_FLAG(Boost),
	DEFINE_FLAG(HideZero),
	DEFINE_FLAG(KeepThroughDeath),
	DEFINE_FLAG(DelayEval),
	DEFINE_FLAG(NoHitDelay),
	DEFINE_FLAG(NoProjectileDelay),
	DEFINE_FLAG(StackByAttribAndKey),
	DEFINE_FLAG(StackExactPower),
	DEFINE_FLAG(IgnoreSuppressErrors),

	DEFINE_FLAG(VanishEntOnTimeout),
	DEFINE_FLAG(DoNotTintCostume),
	DEFINE_FLAG(DoNotDisplayShift),
	DEFINE_FLAG(NoTokenTime),
	DEFINE_FLAG(RevokeAll),
	DEFINE_FLAG(CopyBoosts),
	DEFINE_FLAG(CopyCreatorMods),
	DEFINE_FLAG(NoCreatorModFX),
	DEFINE_FLAG(PseudoPet),
	DEFINE_FLAG(PetVisible),
	DEFINE_FLAG(PetCommandable),
	DEFINE_END
};
#undef DEFINE_FLAG

TokenizerParseInfo ParseAttribModMessages[] =
{
	{ "{",                   TOK_START,  0 },
	{ "DisplayAttackerHit",  TOK_STRING(AttribModMessages, pchDisplayAttackerHit, 0)   },
	{ "DisplayVictimHit",    TOK_STRING(AttribModMessages, pchDisplayVictimHit, 0)     },
	{ "DisplayFloat",        TOK_STRING(AttribModMessages, pchDisplayFloat, 0)         },
	{ "DisplayAttribDefenseFloat", TOK_STRING(AttribModMessages, pchDisplayDefenseFloat, 0)  },
	{ "}",                   TOK_END,      0 },
	{ "", 0, 0 }
};

TokenizerParseInfo ParseAttribModFX[] =
{
	{ "{",                   TOK_START,  0 },
	{ "ContinuingBits",      TOK_INTARRAY(AttribModFX, piContinuingBits), ParsePowerDefines },
	{ "ContinuingFX",        TOK_FILENAME(AttribModFX, pchContinuingFX, 0)    },
	{ "ConditionalBits",     TOK_INTARRAY(AttribModFX, piConditionalBits), ParsePowerDefines },
	{ "ConditionalFX",       TOK_FILENAME(AttribModFX, pchConditionalFX, 0)   },
	{ "}",                   TOK_END,      0 },
	{ "", 0, 0 }
};

TokenizerParseInfo ParseAttribModTargetInfo[] =
{
	{ "{",                   TOK_START,  0 },
	{ "Marker",              TOK_STRINGARRAY(AttribModTargetInfo, ppchMarkerNames) },
	{ "Count",               TOK_INTARRAY(AttribModTargetInfo, piMarkerCount)      },
	{ "}",                   TOK_END,      0 },
	{ "", 0, 0 }
};

TokenizerParseInfo ParseAttribModParam_Costume[] =
{
	{ "{",                   TOK_START,  0 },
	{ "Costume",             TOK_LOAD_ONLY|TOK_STRING(AttribModParam_Costume_loadonly, pchCostumeName, 0) },
	{ "Priority",            TOK_INT(AttribModParam_Costume, iPriority, 5) },
	{ "",                    TOK_NO_BIN|TOK_AUTOINT(AttribModParam_Costume, npcCostume, 0) },
	{ "}",                   TOK_END,      0 },
	{ "", 0, 0 }
};

TokenizerParseInfo ParseAttribModParam_Reward[] =
{
	{ "{",                   TOK_START,  0 },
	{ "Reward",              TOK_STRINGARRAY(AttribModParam_Reward, ppchRewards) },
	{ "}",                   TOK_END,      0 },
	{ "", 0, 0 }
};

TokenizerParseInfo ParseAttribModParam_EntCreate[] =
{
	{ "{",                   TOK_START,  0 },
	{ "EntityDef",           TOK_LOAD_ONLY|TOK_STRING(AttribModParam_EntCreate_loadonly, pchEntityDef, 0) },
	{ "Class",			     TOK_LOAD_ONLY|TOK_STRING(AttribModParam_EntCreate_loadonly, pchClass, 0) },
	{ "Costume",             TOK_LOAD_ONLY|TOK_STRING(AttribModParam_EntCreate_loadonly, pchCostumeName, 0) },
	{ "DisplayName",         TOK_STRING(AttribModParam_EntCreate, pchDisplayName, 0) },
	{ "PriorityList",        TOK_STRING(AttribModParam_EntCreate, pchPriorityList, 0) },
	{ "AIConfig",            TOK_STRING(AttribModParam_EntCreate, pchAIConfig, 0) },
	ParsePowerSpecEmbed(AttribModParam_EntCreate, ps)
	{ "",                    TOK_NO_BIN|TOK_AUTOINT(AttribModParam_EntCreate, pVillain, 0) },
	{ "",                    TOK_NO_BIN|TOK_AUTOINT(AttribModParam_EntCreate, pClass, 0) },
	{ "",                    TOK_NO_BIN|TOK_AUTOINT(AttribModParam_EntCreate, npcCostume, 0) },
	{ "",                    TOK_NO_BIN|TOK_STRING(AttribModParam_EntCreate, pchDoppelganger, 0) },
	{ "}",                   TOK_END,      0 },
	{ "", 0, 0 }
};

TokenizerParseInfo ParseAttribModParam_Power[] =
{
	{ "{",                   TOK_START,  0 },
	ParsePowerSpecEmbed(AttribModParam_Power, ps)
	{ "Count",               TOK_INT(AttribModParam_Power, iCount, 1) },
	{ "}",                   TOK_END,      0 },
	{ "", 0, 0 }
};

TokenizerParseInfo ParseAttribModParam_Phase[] =
{
	{ "{",                   TOK_START,  0 },
	{ "",                    TOK_INTARRAY(AttribModParam_Phase, piCombatPhases) },
	{ "",                    TOK_INTARRAY(AttribModParam_Phase, piVisionPhases) },
	{ "",                    TOK_INT(AttribModParam_Phase, iExclusiveVisionPhase, -1) },
	{ "CombatPhase",         TOK_NO_BIN|TOK_LOAD_ONLY|TOK_STRINGARRAY(AttribModParam_Phase_loadonly, ppchCombatPhaseNames) },
	{ "VisionPhase",         TOK_NO_BIN|TOK_LOAD_ONLY|TOK_STRINGARRAY(AttribModParam_Phase_loadonly, ppchVisionPhaseNames) },
	{ "ExclusiveVisionPhase",TOK_NO_BIN|TOK_LOAD_ONLY|TOK_STRING(AttribModParam_Phase_loadonly, pchExclusiveVisionPhaseName, 0) },
	{ "}",                   TOK_END,      0 },
	{ "", 0, 0 }
};

TokenizerParseInfo ParseAttribModParam_Teleport[] =
{
	{ "{",                   TOK_START,  0 },
	{ "Destination",         TOK_STRING(AttribModParam_Teleport, pchDestination, 0) },
	{ "}",                   TOK_END,      0 },
	{ "", 0, 0 }
};

TokenizerParseInfo ParseAttribModParam_Behavior[] =
{
	{ "{",                   TOK_START,  0 },
	{ "Behavior",            TOK_STRINGARRAY(AttribModParam_Behavior, ppchBehaviors) },
	{ "}",                   TOK_END,      0 },
	{ "", 0, 0 }
};

TokenizerParseInfo ParseAttribModParam_SZEValue[] =
{
	{ "{",                   TOK_START,  0 },
	{ "ScriptID",            TOK_STRINGARRAY(AttribModParam_SZEValue, ppchScriptID) },
	{ "ScriptValue",         TOK_STRINGARRAY(AttribModParam_SZEValue, ppchScriptValue) },
	{ "}",                   TOK_END,      0 },
	{ "", 0, 0 }
};

TokenizerParseInfo ParseAttribModParam_Token[] =
{
	{ "{",                   TOK_START,  0 },
	{ "Token",               TOK_STRINGARRAY(AttribModParam_Token, ppchTokens) },
	{ "}",                   TOK_END,      0 },
	{ "", 0, 0 }
};

TokenizerParseInfo ParseAttribModParam_EffectFilter[] =
{
	{ "{",                   TOK_START,  0 },
	ParsePowerSpecEmbed(AttribModParam_EffectFilter, ps)
	{ "Tag",                 TOK_STRINGARRAY(AttribModParam_EffectFilter, ppchTags) },
	{ "}",                   TOK_END,      0 },
	{ "", 0, 0 }
};

#define PARSE_PARAM_LIST(n) { #n, ParamType_##n, sizeof(AttribModParam_##n), ParseAttribModParam_##n }
ParseDynamicSubtable ParseAttribModParam[] =
{
	PARSE_PARAM_LIST(Costume),
	PARSE_PARAM_LIST(Reward),
	PARSE_PARAM_LIST(EntCreate),
	PARSE_PARAM_LIST(Power),
	PARSE_PARAM_LIST(Phase),
	PARSE_PARAM_LIST(Teleport),
	PARSE_PARAM_LIST(Behavior),
	PARSE_PARAM_LIST(SZEValue),
	PARSE_PARAM_LIST(Token),
	PARSE_PARAM_LIST(EffectFilter),
	{ 0 }
};
#undef PARSE_PARAM_LIST

TokenizerParseInfo ParseAttribModTemplate[] =
{
	{ "{",                   TOK_START,  0 },

	{ "Attrib",              TOK_INTARRAY(AttribModTemplate, piAttrib), ParsePowerDefines },
	{ "Aspect",              TOK_AUTOINT(AttribModTemplate, offAspect, offsetof(CharacterAttribSet, pattrMod)), AspectEnum   },

	{ "ApplicationType",     TOK_INT(AttribModTemplate, eApplicationType, kModApplicationType_OnTick), ModApplicationEnum },
	{ "Type",                TOK_INT(AttribModTemplate, eType, kModType_Magnitude), ModTypeEnum },
	{ "Target",              TOK_INT(AttribModTemplate, eTarget, kModTarget_Affected), ModTargetEnum },
	{ "TargetInfo",          TOK_OPTIONALSTRUCT(AttribModTemplate, pTargetInfo, ParseAttribModTargetInfo) },

	{ "Table" ,              TOK_STRING(AttribModTemplate, pchTable, 0) },
	{ "Scale",               TOK_F32(AttribModTemplate, fScale, 0) },
	{ "Duration",            TOK_F32(AttribModTemplate, fDuration, 0), DurationEnum  },
	{ "Magnitude",           TOK_F32(AttribModTemplate, fMagnitude, 0), ParsePowerDefines },
	{ "DurationExpr",        TOK_STRINGARRAY(AttribModTemplate, ppchDuration)       },
	{ "MagnitudeExpr",       TOK_STRINGARRAY(AttribModTemplate, ppchMagnitude)      },

	{ "Delay",               TOK_F32(AttribModTemplate, fDelay, 0)             },
	{ "Period",              TOK_F32(AttribModTemplate, fPeriod, 0)            },
	{ "TickChance",          TOK_F32(AttribModTemplate, fTickChance, 1)        },
	{ "DelayedRequires",     TOK_STRINGARRAY(AttribModTemplate, ppchDelayedRequires)			},

	{ "CasterStackType",     TOK_INT(AttribModTemplate, eCasterStack, kCasterStackType_Individual), CasterStackTypeEnum },
	{ "StackType",           TOK_INT(AttribModTemplate, eStack, kStackType_Replace), StackTypeEnum },
	{ "StackLimit",          TOK_INT(AttribModTemplate, iStackLimit, 2)		},
	{ "StackKey",            TOK_INT(AttribModTemplate, iStackKey, -1), ParsePowerDefines		},

	{ "CancelEvents",        TOK_INTARRAY(AttribModTemplate, piCancelEvents), PowerEventEnum },
	{ "Suppress",            TOK_STRUCT(AttribModTemplate, ppSuppress, ParseSuppressPair) },

	{ "BoostModAllowed",     TOK_INT(AttribModTemplate, boostModAllowed, 0), ParsePowerDefines },
	{ "Flags",               TOK_FLAGARRAY(AttribModTemplate, iFlags, ATTRIBMOD_FLAGS_SIZE), ParseAttribModFlag },

	{ "Messages",            TOK_OPTIONALSTRUCT(AttribModTemplate, pMessages, ParseAttribModMessages) },
	{ "FX",                  TOK_OPTIONALSTRUCT(AttribModTemplate, pFX, ParseAttribModFX) },

	{ "Params",              TOK_DYNAMICSTRUCT(AttribModTemplate, pParams, ParseAttribModParam) },

	{ "}",                   TOK_END,      0 },
	{ "", 0, 0 }
};

TokenizerParseInfo ParseEffectGroup[] =
{
	{ "{",                   TOK_START,  0 },
	{ "Tag",                 TOK_STRINGARRAY(EffectGroup, ppchTags)                       },
	{ "Chance",				 TOK_F32(EffectGroup, fChance, 1)                             },
	{ "ProcsPerMinute",		 TOK_F32(EffectGroup, fProcsPerMinute, 0)                     },
	{ "Delay",				 TOK_F32(EffectGroup, fDelay, 0)                              },
	{ "RadiusInner",		 TOK_F32(EffectGroup, fRadiusInner, -1)                       },
	{ "RadiusOuter",		 TOK_F32(EffectGroup, fRadiusOuter, -1)                       },
	{ "Requires",			 TOK_STRINGARRAY(EffectGroup, ppchRequires)                   },
	{ "Flags",				 TOK_FLAGS(EffectGroup, iFlags, 0), ParseEffectGroupFlag      },
	{ "EvalFlags",			 TOK_INT(EffectGroup, evalFlags, 0)                           }, // created automatically at bin time from Requires statements
	{ "AttribMod",			 TOK_STRUCT(EffectGroup, ppTemplates, ParseAttribModTemplate) },
	{ "Effect",				 TOK_STRUCT(EffectGroup, ppEffects, ParseEffectGroup)         },
	{ "}",                   TOK_END,      0 },
	{ "", 0, 0 }
};

TokenizerParseInfo ParseLegacyAttribModTemplate[] =
{
	{ "{",                   TOK_START,  0 },
	{ "Name",                TOK_STRINGARRAY(LegacyAttribModTemplate, eg.ppchTags)                },
	{ "DisplayAttackerHit",  TOK_STRING(LegacyAttribModTemplate, msgs.pchDisplayAttackerHit, 0)   },
	{ "DisplayVictimHit",    TOK_STRING(LegacyAttribModTemplate, msgs.pchDisplayVictimHit, 0)     },
	{ "DisplayFloat",        TOK_STRING(LegacyAttribModTemplate, msgs.pchDisplayFloat, 0)         },
	{ "DisplayAttribDefenseFloat", TOK_STRING(LegacyAttribModTemplate, msgs.pchDisplayDefenseFloat, 0)  },
	{ "ShowFloaters",        TOK_BOOL(LegacyAttribModTemplate, bShowFloaters, 1), ModBoolEnum},

	{ "Attrib",              TOK_AUTOINT(LegacyAttribModTemplate, offAttrib, 0), ParsePowerDefines },
	{ "Aspect",              TOK_AUTOINT(LegacyAttribModTemplate, am.offAspect, 0), AspectEnum   },
	{ "BoostIgnoreDiminishing",	TOK_BOOL(LegacyAttribModTemplate, bIgnoresBoostDiminishing, 0), ModBoolEnum},

	{ "Target",              TOK_INT(LegacyAttribModTemplate, am.eTarget, kModTarget_Affected), ModTargetEnum },
	{ "Table" ,              TOK_STRING(LegacyAttribModTemplate, am.pchTable, 0) },
	{ "Scale",               TOK_F32(LegacyAttribModTemplate, am.fScale, 0) },

	{ "ApplicationType",		TOK_INT(LegacyAttribModTemplate, am.eApplicationType, kModApplicationType_OnTick), ModApplicationEnum },
	{ "Type",					TOK_INT(LegacyAttribModTemplate, am.eType, kModType_Magnitude), ModTypeEnum },
	{ "Delay",					TOK_F32(LegacyAttribModTemplate, am.fDelay, 0)             },
	{ "Period",					TOK_F32(LegacyAttribModTemplate, am.fPeriod, 0)            },
	{ "Chance",					TOK_F32(LegacyAttribModTemplate, fChance, 0)            },
	{ "CancelOnMiss",			TOK_BOOL(LegacyAttribModTemplate, bCancelOnMiss, 0), ModBoolEnum },
	{ "CancelEvents",			TOK_INTARRAY(LegacyAttribModTemplate, am.piCancelEvents), PowerEventEnum },
	{ "NearGround",				TOK_BOOL(LegacyAttribModTemplate, bNearGround, 0), ModBoolEnum },
	{ "AllowStrength",			TOK_BOOL(LegacyAttribModTemplate, bAllowStrength, 1), ModBoolEnum },
	{ "AllowResistance",		TOK_BOOL(LegacyAttribModTemplate, bAllowResistance, 1), ModBoolEnum },
	{ "UseMagnitudeResistance",	TOK_BOOL(LegacyAttribModTemplate, bUseMagnitudeResistance, -1), ModBoolEnum },
	{ "UseDurationResistance",	TOK_BOOL(LegacyAttribModTemplate, bUseDurationResistance, -1), ModBoolEnum },
	{ "AllowCombatMods",		TOK_BOOL(LegacyAttribModTemplate, bAllowCombatMods, 1), ModBoolEnum },
	{ "UseMagnitudeCombatMods",	TOK_BOOL(LegacyAttribModTemplate, bUseMagnitudeCombatMods, -1), ModBoolEnum },
	{ "UseDurationCombatMods",	TOK_BOOL(LegacyAttribModTemplate, bUseDurationCombatMods, -1), ModBoolEnum },
	{ "BoostTemplate",			TOK_BOOL(LegacyAttribModTemplate, bBoostTemplate, 0), ModBoolEnum },
	{ "Requires",				TOK_STRINGARRAY(LegacyAttribModTemplate, eg.ppchRequires)       },
	{ "PrimaryStringList",		TOK_STRINGARRAY(LegacyAttribModTemplate, ppchPrimaryStringList)       },
	{ "SecondaryStringList",	TOK_STRINGARRAY(LegacyAttribModTemplate, ppchSecondaryStringList)       },
	{ "CasterStackType",		TOK_INT(LegacyAttribModTemplate, am.eCasterStack, kCasterStackType_Individual), CasterStackTypeEnum },
	{ "StackType",				TOK_INT(LegacyAttribModTemplate, am.eStack, kStackType_Replace), StackTypeEnum },
	{ "StackLimit",				TOK_INT(LegacyAttribModTemplate, am.iStackLimit, 2)		},
	{ "StackKey",				TOK_INT(LegacyAttribModTemplate, am.iStackKey, -1), ParsePowerDefines		},
	{ "Duration",				TOK_F32(LegacyAttribModTemplate, am.fDuration, -1), DurationEnum  },
	{ "DurationExpr",			TOK_STRINGARRAY(LegacyAttribModTemplate, am.ppchDuration)       },
	{ "Magnitude",				TOK_F32(LegacyAttribModTemplate, am.fMagnitude, 0), ParsePowerDefines },
	{ "MagnitudeExpr",			TOK_STRINGARRAY(LegacyAttribModTemplate, am.ppchMagnitude)      },
	{ "RadiusInner",			TOK_F32(LegacyAttribModTemplate, eg.fRadiusInner, -1) },
	{ "RadiusOuter",			TOK_F32(LegacyAttribModTemplate, eg.fRadiusOuter, -1) },

	{ "Suppress",            TOK_STRUCT(LegacyAttribModTemplate, am.ppSuppress, ParseSuppressPair) },
	{ "IgnoreSuppressErrors",TOK_STRING(LegacyAttribModTemplate, pchIgnoreSuppressErrors, 0)},

	{ "ContinuingBits",      TOK_INTARRAY(LegacyAttribModTemplate, fx.piContinuingBits), ParsePowerDefines },
	{ "ContinuingFX",        TOK_FILENAME(LegacyAttribModTemplate, fx.pchContinuingFX, 0)    },
	{ "ConditionalBits",     TOK_INTARRAY(LegacyAttribModTemplate, fx.piConditionalBits), ParsePowerDefines },
	{ "ConditionalFX",       TOK_FILENAME(LegacyAttribModTemplate, fx.pchConditionalFX, 0)   },

	{ "CostumeName",         TOK_FILENAME(LegacyAttribModTemplate, pchCostumeName, 0)     },

	{ "Power",               TOK_REDUNDANTNAME|TOK_STRING(LegacyAttribModTemplate, pchReward, 0)               },
	{ "Reward",              TOK_STRING(LegacyAttribModTemplate, pchReward, 0)               },
	{ "Params",				 TOK_STRING(LegacyAttribModTemplate, pchParams, 0)				},
	{ "EntityDef",           TOK_STRING(LegacyAttribModTemplate, pchEntityDef, 0)            },
	{ "PriorityList",        TOK_STRING(LegacyAttribModTemplate, pchPriorityList, 0)  },
	{ "PriorityListPassive", TOK_REDUNDANTNAME|TOK_STRING(LegacyAttribModTemplate, pchPriorityList, 0)  },
	{ "PriorityListOffense", TOK_IGNORE, 0 },
	{ "PriorityListDefense", TOK_IGNORE, 0 },
	{ "DisplayOnlyIfNotZero",TOK_BOOL(LegacyAttribModTemplate, bDisplayTextOnlyIfNotZero, 0), ModBoolEnum},
	{ "MatchExactPower",	 TOK_BOOL(LegacyAttribModTemplate, bMatchExactPower, 0), ModBoolEnum},
	{ "VanishEntOnTimeout",  TOK_BOOL(LegacyAttribModTemplate, bVanishEntOnTimeout, 0), ModBoolEnum},
	{ "DoNotTint",			 TOK_BOOL(LegacyAttribModTemplate, bDoNotTintCostume, 0), ModBoolEnum},
	{ "KeepThroughDeath",	 TOK_BOOL(LegacyAttribModTemplate, bKeepThroughDeath, 0), ModBoolEnum},
	{ "DelayEval",			 TOK_BOOL(LegacyAttribModTemplate, bDelayEval, 0), ModBoolEnum},
	{ "LinkedChance",		 TOK_BOOL(LegacyAttribModTemplate, bLinkedChance, 0), ModBoolEnum},
	{ "StackByAttrib",		 TOK_BOOL(LegacyAttribModTemplate, bStackByAttrib, 0), ModBoolEnum},
	{ "BoostModAllowed",	 TOK_INT(LegacyAttribModTemplate, am.boostModAllowed, 0), ParsePowerDefines },
//	{ "EvalFlags",			 TOK_INT(LegacyAttribModTemplate, eg.evalFlags, 0)		},						// created automatically at bin time from Requires statements
	{ "ProcsPerMinute",		 TOK_F32(LegacyAttribModTemplate, eg.fProcsPerMinute, 0)         },

	{ "}",                   TOK_END,      0 },
	{ "", 0, 0 }
};
#endif

// combat eval relevant information stored for delayed Eval usage
typedef struct AttribModEvalInfo
{
	float fRand;
	float fToHit;
	bool bAlwaysHit;
	bool bForceHit;
	int targetsHit;

	float fFinal;
	float fVal;
	float fEffectiveness;
	float fStrength;

	float fChanceMods;
	float fChanceFinal;

	int iChainJump;
	float fChainEff;

	int refCount;

} AttribModEvalInfo;

/***************************************************************************/
/***************************************************************************/

typedef struct AttribMod
{
	// After a power hits a character, it is factored down into simple
	//   AttribMods which the target is tagged with. These AttribMods
	//   have a lifetime which eventually expires.

	struct AttribMod *next;
		// This must stay first so this works with genericlist.
	struct AttribMod *prev;
		// This must stay second so this works with genericlist.

	EntityRef erSource;
		// The entity which caused this modifier.

	EntityRef erPrevTarget;
		// The entity which was the previous target. Used for chain target FX.

	EntityRef erCreated;
		// The entity which was created by this modifier, if any.

	Vec3 vecLocation;
		// The base location where this modifier was created, if any.

	unsigned int uiInvokeID;
		// An ID which increments every power invocation. Allows you
		//   to know which attribmods are part of the same power
		//   invocation.

	const AttribModTemplate *ptemplate;
		// A reference to the base template this AttribMod came from.

	size_t offAttrib;
		// Which attribute from the template this AttribMod was created from.

	Power *parentPowerInstance;
		// A reference to the power instance that applied this attrib mod.
		// DO NOT set this value outside of attribmod_setParentPowerInstance()!
		// This is ONLY accurate (and only non-NULL) if this attrib mod was made in mod_Fill()!
		// When attrib mods are saved to the database, they lose this connection to a Power.

	const PowerFX *fx;
		// PowerFX information for this particular attribmod, changeable
		// with power customization.

	float fDuration;
		// How long the effect will last.

	float fTimer;
		// A timer used to count the time between applications of the power
		//    and delay the first application of the power.

	float fMagnitude;
		// How much to change the attribute.

	float fTickChance;
		// The chance that this attrib modifier will be applied to the
		//   target. (1.0 == 100%)

	float fCheckTimer;
		// If a character is defeated before the CheckTimer runs out, then
		//    the attribmod is removed. (Because it won't happen.)
	bool bSourceChecked;
		// If true, then the source entity has been checked to see if it is
		//    alive or not.

	bool bDelayEvalChecked;
		// If true, either this mod doesn't use DelayEval or it has
		//	  processed the DelayEval.

	bool bReportedOnLastTick;
		// true if the attribmod was reported on last tick. Used to minimize
		//   the number of spurious messages. Thi is set up to go off each
		//   period of the atribmod.

	bool bContinuingFXOn;
		// True if the ContinuingFX and ContinuingBits are on for this
		//   attribmod.

	bool bIgnoreSuppress;
		// True if this AttribMod should no longer pay attention to suppression.

	bool bUseLocForReport;
		// True if this AttribMod should use the location for reporting floaters.

	ColorPair uiTint;
		// The tint color of the power which created the attribmod

	const char *customFXToken;
		// Token used to find/fire the appropriate fx

	int petFlags;
		// field use to help pets zone
	
	AttribModEvalInfo *evalInfo;

	bool bSuppressedByStacking;
		// True if this AttribMod is currently being suppressed due to kStackType_Suppress.

	bool bNoFx;
		// Suppress ContinuingFX and ContinuingBits for this attribmod

} AttribMod;


#define PETFLAG_TRANSFER	(1<<0)
#define PETFLAG_UPGRADE1	(1<<1)
#define PETFLAG_UPGRADE2	(1<<2)

/***************************************************************************/
/***************************************************************************/

typedef struct AttribModList
{
	// Implements a list of AttribMods.

	AttribMod *phead;
		// Points to the top of the list.

	AttribMod *ptail;
		// Points to the end of the list.
		// NOTE:  GenericList doesn't support tail pointers!
		// We have to do the tracking ourselves.

} AttribModList;

typedef struct AttribModListIter
{
	AttribModList *plist;

	AttribMod *pposNext;
	AttribMod *pposCur;
		// Variables for tracking where the iteration is. The iterator is
		//   designed so that the "current" AttribMod can safely be
		//   removed and the iteration can continue.
} AttribModListIter;

/***************************************************************************/
/***************************************************************************/

AttribMod *attribmod_Create(void);
void attribmod_Destroy(AttribMod *pmod);

AttribModEvalInfo *attribModEvalInfo_Create(void);
void attribModEvalInfo_Destroy(AttribModEvalInfo *evalInfo);

AttribMod *modlist_GetNextMod(AttribModListIter *piter);
AttribMod *modlist_GetFirstMod(AttribModListIter *piter, AttribModList *plist);
void modlist_RemoveAndDestroyCurrentMod(AttribModListIter *piter);

AttribMod *modlist_FindOldestUnsuppressed(AttribModList *plist, const AttribModTemplate *ptemplate, size_t offAttrib, Power *ppower, Character *pSrc);
AttribMod *modlist_FindLargestMagnitudeSuppressed(AttribModList *plist, const AttribModTemplate *ptemplate, size_t offAttrib, Power *ppower, Character *pSrc);
int modlist_Count(AttribModList *plist, const AttribModTemplate *ptemplate, size_t offAttrib, Power *ppower, Character *pSrc);
void modlist_AddMod(AttribModList *plist, AttribMod *pmod);
void modlist_AddModToTail(AttribModList *plist, AttribMod *pmod);
void modlist_RemoveAndDestroyMod(AttribModList *plist, AttribMod *pmod);
AttribMod* modlist_FindEarlierMod(AttribModList *plist, AttribMod *modA, AttribMod *modB);
void modlist_MoveModToTail(AttribModListIter *piter, AttribMod *mod);

void modlist_RemoveAll(AttribModList *plist);
void modlist_CancelAll(AttribModList *plist, Character *pchar);

void mod_Fill(AttribMod *pmod, const AttribModTemplate *ptemplate, int iAttribIdx, Character *pSrc, Character *pTarget, ModFillContext *ctx);

void modlist_RefreshAllMatching(AttribModList *plist, const AttribModTemplate *ptemplate, size_t offAttrib, Power *ppower, Character *pSrc, float fDuration);

bool mod_Process(AttribMod *pmod, Character *pchar, CharacterAttribSet *pattribset,
	CharacterAttributes *pattrResBuffs, CharacterAttributes *pattrResDebuffs,
	CharacterAttributes *pResistance, bool *pbRemoveSleepMods, bool *pbDisallowDefeat, float fRate);

void mod_Cancel(AttribMod *pmod, Character *pchar);
void mod_CancelFX(AttribMod *pmod, Character *pchar);
void attribmod_SetParentPowerInstance(AttribMod *pmod, Power *ppow);

void HandleSpecialAttrib(AttribMod *pmod, Character *pchar, float f, bool *pbDisallowDefeat);

bool mod_IsChancePerTick(const AttribModTemplate *ptemplate);
bool mod_ChanceRoll(const EffectGroup *peffect, float fFinalChance, unsigned int uiInvokeID);

AttribMod *modlist_BlameDefenseMod(AttribModList *plist, float fPercentile, int iType);

typedef enum CombatMessageType
{
	CombatMessageType_DamagingMod,
	CombatMessageType_PowerActivation,
	CombatMessageType_PowerHitRoll,
} CombatMessageType;

/***************************************************************************/
/***************************************************************************/

#ifdef ATTRIBMOD_PARSE_INFO_DEFINITIONS
#define ATTRIBMOD_PARSE_INFO_DEFINED
#endif

#endif /* #ifndef ATTRIBMOD_H__ */

// These next two comments will not make sense unless you're here because you were 
// changing stuff in seqstate.h.  Just don't delete them, they do serve a purpose.

// This is a different line at the bottom of attribmod.h
// This is a line at the bottom of attribmod.h


/* End of File */

