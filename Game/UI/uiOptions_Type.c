/***************************************************************************
 *     Copyright (c) 2008-2008, NCSoft
 *     All Rights Reserved
 *     Confidential Property of NCSoft
 *
 * Module Description:
 * special file just to be included by another file
 *
 ***************************************************************************/

UserOption game_options[] = 
{	// ID                              // DisplayName               //Val, fVal, dontwrite, bitsize, data ptr
	{ kUO_MouseSpeed,                  "MouseSpeed",                  0, 1, 0, -1 },
	{ kUO_SpeedTurn,                   "SpeedTurn",                   0, 3, 0, -1 },
	{ kUO_MouseInvert,                 "MouseInvert",                 0, 0, 0, 1 },
	{ kUO_ChatFade,                    "ChatFade",                    0, 0, 0, 1, },
	{ kUO_Chat1Fade,                   "Chat1Fade",                   0, 0, 0, 1, },
	{ kUO_Chat2Fade,                   "Chat2Fade",                   0, 0, 0, 1, },
	{ kUO_Chat3Fade,                   "Chat3Fade",                   0, 0, 0, 1, },
	{ kUO_Chat4Fade,                   "Chat4Fade",                   0, 0, 0, 1, },
	{ kUO_CompassFade,                 "CompassFade",                 0, 0, 0, 1, },
	// 
	{ kUO_UseToolTips,                 "UseToolTips",                 1, 0, 0, 1 },
	{ kUO_AllowProfanity,              "AllowProfanity",              0, 0, 0, 1 },
	{ kUO_ShowBallons,                 "ShowBallons",                 1, 0, 0, 1 },
	{ kUO_DeclineGifts,                "DeclineGifts",                0, 0, 0, 1 },
	{ kUO_DeclineTeamGifts,            "DeclineGiftsFromTeammates",   0, 0, 0, 1 },
	{ kUO_PromptTeamTeleport,          "PromptTeleportFromTeammates", 0, 0, 0, 1 },
	{ kUO_ShowPets,                    "ShowPets",                    0, 0, 0, 1 },
	{ kUO_ShowSalvage,                 "ShowSalvage",                 0, 0, 1, 1 },
	{ kUO_WebHideBasics,               "WebHideBasics",               0, 0, 1, 1 },
	{ kUO_HideContactIcons,            "HideContactIcons",            0, 0, 0, 1 },
	{ kUO_ShowAllStoreBoosts,          "ShowAllStoreBoosts",          0, 0, 0, 1 },
	//
	{ kUO_WebHideBadges,               "WebHideBadges",               0, 0, 0, 1 },
	{ kUO_WebHideFriends,              "WebHideFriends",              0, 0, 0, 1 },
	{ kUO_WindowFade,                  "WindowFade",                  0, 0, 0, 1 },
	{ kUO_LogChat,                     "EnableChatLog",               0, 0, 0, 1 },
	{ kUO_HideHeader,                  "HideHeader",                  0, 0, 0, 1 },
	{ kUO_HideButtons,                 "HideButtons",                 0, 0, 0, 1 },
	{ kUO_ClickToMove,                 "EnableClickToMove",           0, 0, 0, 1 },
	{ kUO_DisableDrag,                 "DisableDrag",                 0, 0, 0, 1 },
	{ kUO_ShowPetBuffs,                "gShowPetBuffs",               0, 0, 0, 1 },
	{ kUO_PreventPetIconDrag,          "PreventPetIconDrag",          0, 0, 0, 1 },
	//
	{ kUO_ShowPetControls,             "ShowPetControls",             0, 0, 0, 1 },
	{ kUO_AdvancedPetControls,         "AdvancedPetControls",         0, 0, 0, 1 },
	{ kUO_ChatDisablePetSay,           "ChatDisablePetSay",           0, 0, 0, 1 },
	{ kUO_ChatEnablePetTeamSay,        "ChatEnablePetTeamSay",        0, 0, 0, 1 },
	{ kUO_TeamComplete,                "TeamComplete",                0, 0, 0, 2 },
	{ kUO_DisablePetNames,             "HidePetNames",                0, 0, 0, 1 },
	{ kUO_HidePlacePrompt,             "HidePromptPlaceEnhancement",  0, 0, 0, 1 },
	{ kUO_HideDeletePrompt,            "HidePromptDeleteEnhancement", 0, 0, 0, 1 },
	{ kUO_HideDeleteSalvagePrompt,     "HidePromptDeleteSalvage",     0, 0, 0, 1 },
	{ kUO_HideDeleteRecipePrompt,      "HidePromptDeleteRecipe",      0, 0, 0, 1 },
	//
	{ kUO_HideInspirationFull,         "HideInspirationFullMsg",      0, 0, 0, 1 },
	{ kUO_HideSalvageFull,             "HideSalvageFullMsg",          0, 0, 0, 1 },
	{ kUO_HideRecipeFull,              "HideRecipeFullMsg",           0, 0, 0, 1 },
	{ kUO_HideEnhancementFull,         "HideEnhancementFullMsg",      0, 0, 0, 1 },
	{ kUO_ShowEnemyTells,              "ShowEnemyTells",              1, 0, 0, 1 },
	{ kUO_ShowEnemyBroadcast,          "SeeEnemyBroadcast",           1, 0, 0, 1 },
	{ kUO_HideEnemyLocal,              "DoNotSeeEnemyLocal",          0, 0, 0, 1 },
	{ kUO_HideCoopPrompt,              "HidePromptCoop",              0, 0, 0, 1 },
	{ kUO_StaticColorsPerName,         "StaticColorsPerName",         0, 0, 0, 1 },
	//
	{ kUO_ContactSort,                 "ContactSort",                 0, 0, 0, 4 },
	{ kUO_RecipeHideMissingParts,      "RecipeHideMissingParts",      0, 0, 0, 1 },
	{ kUO_RecipeHideUnowned,           "RecipeHideUnowned",           0, 0, 0, 1 },
	{ kUO_RecipeHideMissingPartsBench, "RecipeHideMissingPartsBench", 0, 0, 0, 1 },
	{ kUO_RecipeHideUnownedBench,      "RecipeHideUnownedBench",      0, 0, 0, 1 },
	{ kUO_DeclineSuperGroupInvite,     "AutoDeclineSuperGroupInvite", 0, 0, 0, 1 },
	{ kUO_DeclineTradeInvite,		   "AutoDeclineTradeInvite",      0, 0, 0, 1 },
	{ kUO_CamFree,					   "CamFree",                     0, 0, 0, 1 },
	{ kUO_ToolTipDelay,                "ToolTipDelaySec",             0, .6f, 0, -1 },
	{ kUO_ShowArchetype,               "ShowArchetype",               kShow_OnMouseOver, 0, 0, 3 },
	//
	{ kUO_ShowSupergroup,              "ShowSupergroup",              kShow_OnMouseOver, 0, 0, 3 },
	{ kUO_ShowPlayerName,              "ShowPlayerName",              kShow_Always, 0, 0, 3 },
	{ kUO_ShowPlayerBars,              "ShowPlayerBars",              kShow_Selected|kShow_OnMouseOver, 0, 0, 3 },
	{ kUO_ShowVillainName,             "ShowVillainName",             kShow_Selected|kShow_OnMouseOver, 0, 0, 3 },
	{ kUO_ShowVillainBars,             "ShowVillainBars",             kShow_Selected|kShow_OnMouseOver, 0, 0, 3 },
	{ kUO_ShowPlayerReticles,          "ShowPlayerReticles",          kShow_Selected|kShow_OnMouseOver, 0, 0, 3 },
	{ kUO_ShowVillainReticles,         "ShowVillainReticles",         kShow_Selected|kShow_OnMouseOver, 0, 0, 3 },
	{ kUO_ShowAssistReticles,          "ShowAssistReticles",          kShow_Selected|kShow_OnMouseOver, 0, 0, 3 },
	{ kUO_ShowOwnerName,               "ShowOwnerName",               kShow_HideAlways, 0, 0, 3 },
	//
	{ kUO_MousePitchOption,            "MousePitchSetting",           0, 0, 0, 3 },
	{ kUO_MapOptions,                  "MapOptions",                  0, 0, 0, 32 },
	{ kUO_BuffSettings,                "BuffSettings",                0, 0, 0, 32 },
	{ kUO_ChatBubbleColor1,            "ChatBubbleColor1",            0, 0, 0, 32 },
	{ kUO_ChatBubbleColor2,            "ChatBubbleColor2",           -1, 0, 0, 32 },
	{ kUO_ChatFontSize,                "DefaultChatFontSize",        12, 0, 0, 8 },
	{ kUO_ReverseMouseButtons,         "MouseButtonReverse",          0, 0, 0, 1 },
	{ kUO_DisableCameraShake,          "DisableCameraShake",          0, 0, 0, 1 },
	{ kUO_DisableMouseScroll,          "DisableMouseScroll",          0, 0, 0, 1 },	
	{ kUO_LogPrivateMessages,          "LogPrivateMessages",          0, 0, 0, 1 },	
	//
	{ kUO_ShowPlayerRating,            "ShowPlayerRating",			  kShow_Selected|kShow_OnMouseOver, 0, 0, 3 },	
	{ kUO_DisableLoadingTips,          "DisableLoadingTips",		  0, 0, 0, 1 },
	{ kUO_MouseScrollSpeed,			   "MouseScrollSpeed",			  0, 20, 0, -1 },
	{ kUO_EnableJoystick,			   "EnableJoystick",			  1, 0, 0, 1 },
	{ kUO_FadeExtraTrays,			   "FadeExtraTrays",			  0, 0, 0, 1, },	
	{ kUO_OptionArchitectNav,	       "ArchitectNav",				  1, 0, 0, 1, },
	{ kUO_ArchitectTips,			   "ArchitectToolTips",			  1, 0, 0, 1, },
	{ kUO_ArchitectAutoSave,	       "ArchitectAutoSave",			  1, 0, 0, 1, },
	{ kUO_NoXP,						   "NoXP",						  0, 0, 0, 1, },
	{ kUO_BlockArchitectComments,	   "ArchitectBlockComment",		  0, 0, 0, 1, },
	//
	{ kUO_TeamLevelAbove,				"AutoAcceptTeamLevelAbove",	  0, 0, 0, 6, },
	{ kUO_TeamLevelBelow,				"AutoAcceptTeamLevelBelow",	  0, 0, 0, 6, },
	{ kUO_DisableEmail,					"DisableEmail",				  0, 0, 0, 1, },
	{ kUO_FriendSGEmailOnly,			"FriendSGEmailOnly",		  0, 0, 0, 1, },
	{ kUO_NoXPExemplar,					"NoXPExemplar",		          0, 0, 0, 1, },
	{ kUO_HideFeePrompt,				"HideFee",					  0, 0, 0, 1, },
	{ kUO_HideUsefulSalvagePrompt,		"HideSalvageWarning",		  0, 0, 0, 1, },
	{ kUO_GMailFriendOnly,				"GmailFriendOnly",			  0, 0, 0, 1, },
	//
	{ kUO_UseOldTeamUI,					"UseOldTeamUI",				  0, 0, 0, 1, },
	{ kUO_HideUnclaimableCerts,			"HideUnclaimableCert",		  0, 0, 0, 1, },
	{ kUO_BlinkCertifications,			"BlinkCertifications",		  1, 0, 0, 1, },
	{ kUO_VoucherPrompt,				"VoucherPrompt",			  0, 0, 0, 1, },
	{ kUO_NewCertificationPrompt,		"NewCertPrompt",			  0, 0, 0, 1, },
	{ kUO_HideUnslotPrompt,            "HidePromptUnslotEnhancement", 0, 0, 0, 1 },
	{ kUO_MapOptionRevision,			"MapOptionRevision",			0, 0, 0, 16 },
	{ kUO_MapOptions2,					"MapOptions2",					0, 0, 0, 32 },
	{ kUO_HideOpenSalvagePrompt,		"OpenSalvageWarning",			0, 0, 0, 1, },
	{ kUO_HideLoyaltyTreeAccessButton,	"LoyaltyTreeAccessButton",		0, 0, 0, 1, },
	//
	{ kUO_HideStoreAccessButton,		"StoreAccessButton",			0, 0, 0, 1, },
	{ kUO_AutoFlipSuperPackCards,		"AutoFlipSuperPackCards",		0, 0, 0, 1, },
	{ kUO_HideConvertConfirmPrompt,		"HideConvertConfirmPrompt",		0, 0, 0, 1, },
	{ kUO_HideStorePiecesState,			"HideStorePiecesState",			0, 0, 0, 3, },
	{ kUO_CursorScale,					"CursorScale",					0, 1, 0, -1, },
};

static StashTable st_GameOptions;
void initGameOptionStash()
{
	int i, size = ARRAY_SIZE(game_options);
	if(st_GameOptions)
		return;

	assert( size == kUO_OptionTotal-1 );
	st_GameOptions = stashTableCreateInt(128);
	for( i = ARRAY_SIZE(game_options)-1; i>=0; i-- )
		stashIntAddPointer( st_GameOptions, game_options[i].id, &game_options[i], 0 );
}

UserOption *userOptionGet( int id )
{
	UserOption * pUO;
	initGameOptionStash();
	stashIntFindPointer( st_GameOptions, id, &pUO );
	if( pUO )
		return pUO;
	return 0;
}
