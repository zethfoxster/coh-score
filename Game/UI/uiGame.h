#ifndef UIGAME_H
#define	UIGAME_H

//switch to turn off all menu collisions.
extern int collisions_off_for_rest_of_frame; 

typedef enum menuFunctions
{
	MENU_LOGIN,			// login screen

	// hybrid menus
	MENU_ORIGIN,
	MENU_PLAYSTYLE,
	MENU_ARCHETYPE,
	MENU_POWER,
	MENU_BODY,
	MENU_REGISTER,

	// original 
	MENU_GENDER,		// gender selection menu
	MENU_COSTUME,		// costume creation
	MENU_LEVEL_POWER,	// power leveling screen
	MENU_LEVEL_POOL_EPIC, // pool and epic power leveling screen
	MENU_LEVEL_SPEC,	// specialization leveling screen
	MENU_COMBINE_SPEC,	// specialization combining screen
	MENU_SUPER_REG,		// supergroup registration page
	MENU_SUPERCOSTUME,	// supergroup costume color changing
	MENU_TAILOR,		// costume changing
	MENU_RESPEC,		// character respecialization
	MENU_LOADCOSTUME,	// load costume for CostumeCreator
	MENU_PCC_POWERS,	// power selection screen for Player Created Critter
	MENU_PCC_PROFILE,	// profile menu for Player Created Critter
	MENU_NPC_COSTUME,	// NPC Costume Edit screen
	MENU_NPC_PROFILE,	// NPC Profile Edit screen
	MENU_PCC_RANK,		//	Costume creator rank screen
	MENU_SAVECOSTUME,	// save costume for CostumeCreator
	MENU_POWER_CUST,	// Powers Customization
	MENU_POWER_CUST_POWERPOOL,	// Powers Customization - Power Pools
	MENU_POWER_CUST_INHERENT,	// Powers Customization - Inherent
	MENU_POWER_CUST_INCARNATE,	// Powers Customization - Incarnate
	MENU_LOAD_POWER_CUST,	//	Power Customization load
	MENU_REDIRECT,		// Error screen with instructions for what to fix
	MENU_GAME,			// in gamewindows - DO NOT ADD SHELL MENUS AFTER THIS
	MENU_TOTAL,

} menuFunctions;

// returns true if the world is not visible for a particular menu
//
int shell_menu(); 

// returns true if the current menu is the given menu
//
int isMenu(  int menu );

// Get menu name
//
const char* getMenuName(int menu);

// returns the current menu index
//
int current_menu();

// returns true if the current menu is a menu that needs to be letterboxed
int isLetterBoxMenu();

// since there is more than one powers customization menu, we will lump them all here
int isPowerCustomizationMenu(int menu);
int isCurrentMenu_PowerCustomization();

void forceLoginMenu();

// changes the current menu to the given menu index
//
void start_menu( int menu );

// go to game menu and close extra windows/dialogs
//
void returnToGame(void);


// Main UI loop
//
void serve_menus();


#endif