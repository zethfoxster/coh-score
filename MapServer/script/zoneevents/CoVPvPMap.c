// ZONE SCRIPT
//

// Used for testing new features of the script system

#include "scriptutil.h"

enum {
	PvPSeesawTypeNormalDamageDef = 0,
	PvPSeesawTypeNormalDamage,
	PvPSeesawTypeNormalNum
};

enum {
	PvPSeesawTypeFreeForAllDamage = 0,
	PvPSeesawTypeFreeForAllTowers,
	PvPSeesawTypeFreeForAllNum
};

enum {
	PvPSeesawSideHero = 0,
	PvPSeesawSideVillain,
	PvPSeesawSideNum
};


enum {
	PvPSeesawDirDeBuff = 0,
	PvPSeesawDirBuff,
	PvPSeesawDirNum
};

char *PvPSeesawMessages[PvPSeesawTypeNormalNum][PvPSeesawSideNum][PvPSeesawDirNum] =
{
	{ { "HeroesDamageDefDeBuff", "HeroesDamageDefBuff" }, { "VillainsDamageDefDeBuff", "VillainsDamageDefBuff" } },
	{ { "HeroesDamageDeBuff", "HeroesDamageBuff" }, { "VillainsDamageDeBuff", "VillainsDamageBuff" } }
};

char *PvPSeesawTypeNormalStrings[PvPSeesawTypeNormalNum] = { "Def", "Dam"  };
char *PvPSeesawTypeFreeForAllStrings[PvPSeesawTypeFreeForAllNum] = { "Tow", "Dam"  };
char *PvPSeesawSideStrings[PvPSeesawSideNum] = { "H", "V" };
char *PvPSeesawDirStrings[PvPSeesawDirNum] = { "D", "B" };

// removes all powers from player
void CoVPvPMapRemoveSeesawPowers(ENTITY player, NUMBER type)
{
	int i;
	
	for ( i = 1; i <= VarGetNumber("Max"); i++)
	{
		STRING DamageBuf = StringAdd(VarGet("DamageBuff"), NumberToString(i));
		STRING DamageDeBuf = StringAdd(VarGet("DamageDeBuff"), NumberToString(i));
		STRING DamageDefBuf = StringAdd(VarGet("DamageDefBuff"), NumberToString(i));
		STRING DamageDefDeBuf = StringAdd(VarGet("DamageDefDeBuff"), NumberToString(i));

		if (type == PvPSeesawTypeNormalDamageDef) {
			EntityRemoveTempPower(player, "SilentPowers", DamageDefBuf);
			EntityRemoveTempPower(player, "SilentPowers", DamageDefDeBuf);
		} else {
			EntityRemoveTempPower(player, "SilentPowers", DamageBuf);
			EntityRemoveTempPower(player, "SilentPowers", DamageDeBuf);
		}
	}

}



// called when the player leaves the map
int CoVPvPMapOnLeaveMap(ENTITY player)
{
	//Remove gang affiliation
	SetAIGang(player, 0);

	// cleaning up seesaw powers
	CoVPvPMapRemoveSeesawPowers(player, PvPSeesawTypeNormalDamageDef);
	CoVPvPMapRemoveSeesawPowers(player, PvPSeesawTypeNormalDamage);

	return 0;
}

// called when the player enters the map
int CoVPvPMapOnEnterMap(ENTITY player)
{
	//Set gang affiliation
	//if you are on a team, use the teamup ID 
	if (VarGetNumber("IsFreeForAll") == 1)
		SetBloodyBayTeam( player );

	// sending dialog message
	SendPlayerDialog(player, StringLocalize(VarGet("PopUpMessage")));

	// setting up any seesaw powers
	{ 
		int i, value;
		STRING countName;
		STRING power;


		for (i = 0; i < PvPSeesawTypeNormalNum; i++) {
			countName = StringAdd("C", VarGet("MapName"));
			countName = StringAdd(countName, PvPSeesawTypeNormalStrings[i]);
			if (IsEntityHero(player)) {
				countName = StringAdd(countName, PvPSeesawSideStrings[PvPSeesawSideHero]);
			} else {
				countName = StringAdd(countName, PvPSeesawSideStrings[PvPSeesawSideVillain]);
			}
			value = VarGetNumber(countName);

			power = NULL;
			if (i == PvPSeesawTypeNormalDamageDef) 
			{
				if (value > 0) {
					power = StringAdd(VarGet("DamageDefBuff"), NumberToString(value));
				} else if (value < 0) {
					power = StringAdd(VarGet("DamageDefDeBuff"), NumberToString(-value));
				}
			} else if (i == PvPSeesawTypeNormalDamage) {
				if (value > 0) {
					power = StringAdd(VarGet("DamageBuff"), NumberToString(value));
				} else if (value < 0) {
					power = StringAdd(VarGet("DamageDeBuff"), NumberToString(-value));
				}
			}
			if (power != NULL && !StringEqual(power, "")) {
				EntityGrantTempPower(player, "SilentPowers", power);
			}
		}
	}

	return 0;
}

// called when the player leaves the map
int CoVPvPMapOnJoinTeam(ENTITY player)
{
	//Set gang affiliation
	//if you are on a team, use the teamup ID 
	if (VarGetNumber("IsFreeForAll") == 1)
		SetBloodyBayTeam( player );

	return 0;
}

// called when the player leaves the map
int CoVPvPMapOnLeaveTeam(ENTITY player)
{
	//Set gang affiliation
	//if you are on a team, use the teamup ID 
	if (VarGetNumber("IsFreeForAll") == 1)
		SetBloodyBayTeam( player );
 
	return 0;
}


void CoVPvPMap(void)
{
	INITIAL_STATE

		ON_ENTRY 
			// put stuff that should run once on initialization here

			// let map know its a PvP map
			SetPvPMap(true);

			SetScriptCombatLevel(VarGetNumber("PvPCombatLevel"));

			//////////////////////////////////////////////////////////////////////////
			///Script Hooks
			OnPlayerExitingMap( CoVPvPMapOnLeaveMap );

			OnPlayerEnteringMap( CoVPvPMapOnEnterMap );
				
			OnPlayerJoinsTeam( CoVPvPMapOnJoinTeam );

			OnPlayerLeavesTeam(	CoVPvPMapOnLeaveTeam );

		END_ENTRY

		if (VarGetNumber("IsFreeForAll") == 1)
			RunBloodyBayTick();

		// check for seesaw action!
		{
			int i, j, k, l, count;
			STRING varname;
			char **type;
			int changed = false;

			// all the same for now
//			if (VarGetNumber("IsFreeForAll")) {
//				count = PvPSeesawTypeFreeForAllNum;
//				type = PvPSeesawTypeFreeForAllStrings;
//			} else {
				count = PvPSeesawTypeNormalNum;
				type = PvPSeesawTypeNormalStrings;
//			}

			for (i = 0; i < count; i++)
			{
				for (j = 0; j < PvPSeesawSideNum; j++)
				{
					for (k = 0; k < PvPSeesawDirNum; k++)
					{
						varname = StringAdd(VarGet("MapName"), type[i]);
						varname = StringAdd(varname, PvPSeesawSideStrings[j]);
						varname = StringAdd(varname, PvPSeesawDirStrings[k]);
 
						if (MapHasDataToken(varname))
						{
							int value;
							STRING countName = StringAdd("C", VarGet("MapName"));
							int times = MapGetDataToken(varname);
							 
							if (times > 0) {
								countName = StringAdd(countName, type[i]);
								countName = StringAdd(countName, PvPSeesawSideStrings[j]);

								value = VarGetNumber(countName);
 
								for (l = 0; l < times; l++)
								{
									value += PvPTaskGetModifier(j, k, i, value);

									changed = true;

									// checking for bounds
									if (value > VarGetNumber("Max")) {
										value = VarGetNumber("Max");
									} else if (value < -VarGetNumber("Max")) {
										value = -VarGetNumber("Max");
									}																

									// checking for message to send
									// map / type / side / direction / "Name" / #
									if (l < 5) {
										STRING playerDBIDVar = StringAdd(VarGet("MapName"), type[i]);
										playerDBIDVar = StringAdd(playerDBIDVar, PvPSeesawSideStrings[j]);
										playerDBIDVar = StringAdd(playerDBIDVar, PvPSeesawDirStrings[k]);
										playerDBIDVar = StringAdd(playerDBIDVar, "Name");
										playerDBIDVar = StringAdd(playerDBIDVar, NumberToString(l));

										if (MapHasDataToken(playerDBIDVar))
										{
											int DBID = MapGetDataToken(playerDBIDVar);
											int diff = value;
											char format[512], msg[512];
											int num = NumEntitiesInTeam(ALL_PLAYERS);
											int idx;

											format[0] = 0;
											MapRemoveDataToken(playerDBIDVar);

											strcat(format, StringLocalizeWithDBID(VarGet("Victory"), DBID));
											strcat(format, StringLocalize(VarGet(PvPSeesawMessages[i][j][value >= 0])));

											if (value < 0)
												diff = -value;

											sprintf(msg, format, diff);
											for (idx = 1; (idx <= num); idx++)
											{
												ENTITY player = GetEntityFromTeam(ALL_PLAYERS, idx);

												SendPlayerSystemMessage(player, msg);
											}
										}
									}
								}

								// writing back to var
								VarSetNumber(countName, value);
							}
							MapRemoveDataToken(varname);

							// update powers
							if (changed) {
								int count;
								STRING power = NULL;
								int type = i;
								int m;

								count = NumEntitiesInTeam(ALL_PLAYERS);
								for (m = 1; (m <= count); m++)
								{
									ENTITY player = GetEntityFromTeam(ALL_PLAYERS, m);

									// checking for correct side
									if ((j == PvPSeesawSideHero && IsEntityHero(player)) ||
										(j == PvPSeesawSideVillain && IsEntityVillain(player)))
									{
										// finding type and direction
										CoVPvPMapRemoveSeesawPowers(player, type);
										if (type == PvPSeesawTypeNormalDamageDef) 
										{
											if (value > 0) {
												power = StringAdd(VarGet("DamageDefBuff"), NumberToString(value));
											} else if (value < 0) {
												power = StringAdd(VarGet("DamageDefDeBuff"), NumberToString(-value));
											}
										} else if (type == PvPSeesawTypeNormalDamage) {
											if (value > 0) {
												power = StringAdd(VarGet("DamageBuff"), NumberToString(value));
											} else if (value < 0) {
												power = StringAdd(VarGet("DamageDeBuff"), NumberToString(-value));
											}
										}

										if (power != NULL && !StringEqual(power, "")) {
											EntityGrantTempPower(player, "SilentPowers", power);
										}
									}
								}
								changed = false;
							} 
						}
					}
				}
			}
		}
	END_STATES

	ON_SIGNAL("End")
		EndScript();
	END_SIGNAL

	ON_STOPSIGNAL
		EndScript();
	END_SIGNAL
}

void CoVPvPMapInit()
{
	SetScriptName( "CoVPvPMap" );
	SetScriptFunc( CoVPvPMap );
	SetScriptType( SCRIPT_ZONE );		

	SetScriptVar( "IsFreeForAll",								"0",					0 );
	SetScriptVar( "PvPCombatLevel",								"1",					0 );
	SetScriptVar( "MapName",									NULL,					0 );
	SetScriptVar( "PopUpMessage",								NULL,					0 );

	SetScriptVar( "Max",										"15",					SP_OPTIONAL );

//	SetScriptVar( "CBBDefV",									"15",					SP_OPTIONAL );

	SetScriptVar( "DamageDefBuff",								"Resist_Damage",		SP_OPTIONAL );
	SetScriptVar( "DamageDefDeBuff",							"Damage_Vulnerability",	SP_OPTIONAL );
	SetScriptVar( "DamageBuff",									"Damage_Buff",			SP_OPTIONAL );
	SetScriptVar( "DamageDeBuff",								"Damage_DeBuff",		SP_OPTIONAL );
	
	SetScriptVar( "Victory",									"",						0 );
	SetScriptVar( "HeroesDamageBuff",							"",						0 );
	SetScriptVar( "HeroesDamageDefBuff",						"",						0 );
	SetScriptVar( "HeroesDamageDeBuff",							"",						0 );
	SetScriptVar( "HeroesDamageDefDeBuff",						"",						0 );
	SetScriptVar( "VillainsDamageBuff",							"",						0 );
	SetScriptVar( "VillainsDamageDefBuff",						"",						0 );
	SetScriptVar( "VillainsDamageDeBuff",						"",						0 );
	SetScriptVar( "VillainsDamageDefDeBuff",					"",						0 );

}