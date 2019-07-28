#include "textparser.h"
#include "earray.h"
#include "error.h"
#include "taskpvp.h"

typedef struct PvPModifierRule
{
	int min;
	int max;
	int heroModifier;
	int villainModifier;
} PvPModifierRule;

typedef struct PvPTaskModifier
{
	int bonusCap;
	int penaltyCap;
	PvPModifierRule** modifierRules;
	// Actual modifier lists to be looked up during runtime
	int* heroModifiers;
	int* villainModifiers;
} PvPTaskModifier;

typedef struct PvPTaskModifierList
{
	PvPTaskModifier heroDamageModifier;
	PvPTaskModifier heroResistanceModifier;
	PvPTaskModifier villainDamageModifier;
	PvPTaskModifier villainResistanceModifier;
	PvPTaskModifier towerDamageModifier;
	PvPTaskModifier towerResistanceModifier;
} PvPTaskModifierList;

PvPTaskModifierList g_pvptaskmodifierlist = {0};

TokenizerParseInfo ParsePvPModifierRule[] =
{
	{"{",				TOK_START,	0},
	{"RangeMin",		TOK_INT(PvPModifierRule, min, 0)},
	{"RangeMax",		TOK_INT(PvPModifierRule, max, 0)},
	{"HeroModifier",	TOK_INT(PvPModifierRule, heroModifier, 0)},
	{"VillainModifier",	TOK_INT(PvPModifierRule, villainModifier, 0)},
	{"}",				TOK_END,		0},
	{"",0,0}
};

TokenizerParseInfo ParsePvPTaskModifier[] =
{
	{"{",				TOK_START,	0},
	{"BonusCap",		TOK_INT(PvPTaskModifier,bonusCap, 0)},
	{"PenaltyCap",		TOK_INT(PvPTaskModifier,penaltyCap, 0)},
	{"ModifierRule",	TOK_STRUCT(PvPTaskModifier,modifierRules,ParsePvPModifierRule) },
	{"}",				TOK_END,		0},
	{"",0,0}
};

TokenizerParseInfo ParsePvPTaskModifierList[] =
{
	{"HeroDamageModifier",			TOK_EMBEDDEDSTRUCT(PvPTaskModifierList,heroDamageModifier,ParsePvPTaskModifier) },
	{"HeroResistanceModifier",		TOK_EMBEDDEDSTRUCT(PvPTaskModifierList,heroResistanceModifier,ParsePvPTaskModifier) },
	{"VillainDamageModifier",		TOK_EMBEDDEDSTRUCT(PvPTaskModifierList,villainDamageModifier,ParsePvPTaskModifier) },
	{"VillainResistanceModifier",	TOK_EMBEDDEDSTRUCT(PvPTaskModifierList,villainResistanceModifier,ParsePvPTaskModifier) },
	{"",0,0}
};

// Only call when modifier is defined
#define PMR_INDEX(i)	i - modifier->penaltyCap

void VerifyPvPModifier(PvPTaskModifier* modifier, char* which)
{
	int size = 0;
	int i, j, n;
	if (!modifier)
	{
		ErrorFilenamef("pvptaskmodifiers.def", "Could not find modifier: %s.", which);
		return;
	}
	
	if (modifier->bonusCap > 100 || modifier->bonusCap < 0)
	{
		ErrorFilenamef("pvptaskmodifiers.def", "%s.BonusCap: %i, must be between 0 and 100.", which, modifier->bonusCap);
		return;
	}

	if (modifier->penaltyCap < -100 || modifier->penaltyCap > 0)
	{
		ErrorFilenamef("pvptaskmodifiers.def", "%s.PenaltyCap: %i, must be between 0 and -100.", which, modifier->penaltyCap);
		return;
	}

	size = modifier->bonusCap - modifier->penaltyCap + 1;

	eaiCreate(&modifier->heroModifiers);
	eaiSetSize(&modifier->heroModifiers, size);
	eaiCreate(&modifier->villainModifiers);
	eaiSetSize(&modifier->villainModifiers, size);

	n = eaSize(&modifier->modifierRules);

	// Fill the modifiers and make sure rules don't overlap
	for (i=0; i<n; i++)
	{
		PvPModifierRule* rule = modifier->modifierRules[i];
		if (rule->min < modifier->penaltyCap)
		{
			ErrorFilenamef("pvptaskmodifiers.def", "%s.rule[%i].min: %i cannot be less than PenaltyCap: %i.", which, i, rule->min, modifier->penaltyCap);
			continue;
		}
		if (rule->max > modifier->bonusCap)
		{
			ErrorFilenamef("pvptaskmodifiers.def", "%s.rule[%i].max: %i cannot be greater than BonusCap: %i.", which, i, rule->max, modifier->penaltyCap);
			continue;
		}
		for (j=rule->min; j<=rule->max; j++)
		{
			if (modifier->heroModifiers[PMR_INDEX(j)] || modifier->villainModifiers[PMR_INDEX(j)])
			{
				ErrorFilenamef("pvptaskmodifiers.def", "%s.rule[%i]: %i overlaps an existing rule.", which, i, j);
				break;
			}
			modifier->heroModifiers[PMR_INDEX(j)] = rule->heroModifier;
			modifier->villainModifiers[PMR_INDEX(j)] = rule->villainModifier;
		}
	}

	// Make sure we covered the whole range
	for (i=modifier->penaltyCap; i<=modifier->bonusCap; i++)
	{
		if (!modifier->heroModifiers[PMR_INDEX(i)] || !modifier->villainModifiers[PMR_INDEX(i)])
		{
			ErrorFilenamef("pvptaskmodifiers.def", "%s.modifiers[%i]: The entire range was not covered.", which, i);
			break;
		}
	}
}

void VerifyPvPTaskModifiers()
{
	// Make sure it read in all the modifiers 
	VerifyPvPModifier(&g_pvptaskmodifierlist.heroDamageModifier, "HeroDamageModifier");
	VerifyPvPModifier(&g_pvptaskmodifierlist.heroResistanceModifier, "HeroResistanceModifier");
	VerifyPvPModifier(&g_pvptaskmodifierlist.villainDamageModifier, "VillainDamageModifier");
	VerifyPvPModifier(&g_pvptaskmodifierlist.villainResistanceModifier, "VillainResistanceModifier");
}

void PvPTaskLoadModifiers()
{
	ParserLoadFiles(0, "defs/pvptaskmodifiers.def", "pvptaskmodifiers.bin", PARSER_SERVERONLY, ParsePvPTaskModifierList, &g_pvptaskmodifierlist, NULL, NULL, NULL, NULL);
	VerifyPvPTaskModifiers();
}

// villain: 0=hero, 1=villain
// buff: 0=debuff, 1=buff
// damage: 0=damage resist, 1 = damage increase
// curr = the current value of this type
int PvPTaskGetModifier(int villain, int buff, int damage, int curr)
{
	PvPTaskModifier* modifier;
	
	// Find the correct modifier
	if (damage)
	{
		if (villain)
			modifier = &g_pvptaskmodifierlist.villainDamageModifier;
		else
			modifier = &g_pvptaskmodifierlist.heroDamageModifier;
	}
	else
	{
		if (villain)
			modifier = &g_pvptaskmodifierlist.villainResistanceModifier;
		else
			modifier = &g_pvptaskmodifierlist.heroResistanceModifier;
	}

	// Shouldn't happen unless def wasn't read in properly, would have been flagged as a bug already
	if (!modifier) return 0;

	// Now get the appropriate value;
	if (villain)
	{
		if (buff)
		{
			if (curr >= modifier->bonusCap)
				return 0;
			else
				return modifier->villainModifiers[PMR_INDEX(curr)];
		}
		else // penalty
		{
			if (curr <= modifier->penaltyCap)
				return 0;
			else
				return modifier->heroModifiers[PMR_INDEX(curr)];
		}
	}
	else // hero
	{
		if (buff)
		{
			if (curr >= modifier->bonusCap)
				return 0;
			else
				return modifier->heroModifiers[PMR_INDEX(curr)];
		}
		else // penalty
		{
			if (curr <= modifier->penaltyCap)
				return 0;
			else
				return modifier->villainModifiers[PMR_INDEX(curr)];
		}
	}

	return 0;
}