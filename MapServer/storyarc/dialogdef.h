/*\
 *
 *	dialogdef.h/c - Copyright 2007 NCSoft
 *		All Rights Reserved
 *		Confidential property of NCSoft
 *
 * 	functions to handle static dialog defs, and
 *	verify them on load
 *
 */

#ifndef __DIALOGDEF_H
#define __DIALOGDEF_H

// for DialogPageDef flags
#define DIALOGPAGE_COMPLETEDELIVERYTASK				(1 << 0)
#define DIALOGPAGE_COMPLETEANYTASK					(1 << 1)
#define DIALOGPAGE_AUTOCLOSE						(1 << 2) // Effects are processed but page is never shown.
#define DIALOGPAGE_NOHEADSHOT						(1 << 3) // Don't display the default headshot.
#define DIALOGPAGE_NOEMPTYLINE						(1 << 4) // Don't add an empty line before the "Close" link.

// for DialogPageListDef flags
#define DIALOGPAGELIST_RANDOM						(1 << 0)
#define DIALOGPAGELIST_INORDER						(1 << 1)



typedef struct DialogAnswerDef {
	const char				*text;
	const char				*page;
	const char				**pRequires;
} DialogAnswerDef;

typedef struct DialogPageDef {
	const char				*name;
	const char				*missionName; // only for MissionPages
	const char				*contactFilename; // only for ContactPages
	const char				*teleportDest; // only for Teleport
	const char				*text;
	const char				*sayOutLoudText;
	const char				*workshop;
	const char				**pRequires;
	const char				**pObjectives;
	const char				**pRewards;
	const char				**pAddClues;
	const char				**pRemoveClues;
	const char				**pAddTokens;
	const char				**pRemoveTokens;
	const char				**pAddTokensToAll;
	const char				**pRemoveTokensFromAll;
	const char				**pAbandonContacts;
	const DialogAnswerDef	**pAnswers;
	int					flags;
} DialogPageDef;

typedef struct DialogPageListDef {
	const char*				name;
	const char				**pPages;
	const char				*fallbackPage;
	const char				**pRequires;
	int					flags;
} DialogPageListDef;

typedef struct DialogDef {
	const char				*name;
	const char				*filename;
	const char				**pQualifyRequires;
	const DialogPageDef		**pPages;
	const DialogPageListDef	**pPageLists;
} DialogDef;

typedef struct DialogContext {
	const DialogDef		*pDialog;
	int					ownerDBID;
	StoryTaskHandle		sahandle;
	ScriptVarsTable		vars;
} DialogContext;

extern TokenizerParseInfo ParseDialogDef[];

// *********************************************************************************
//  Load and parse dialog defs
// *********************************************************************************

const DialogDef *DialogDef_FindDefFromStoryTask(const StoryTask *task, const char *name);
const DialogDef *DialogDef_FindDefFromSpawnDef(const SpawnDef *pSpawnDef, const char *name);
const DialogDef *DialogDef_FindDefFromContact(const ContactDef *pContactDef, const char *name);
const DialogPageDef *DialogDef_FindPageFromDef(int hash, const DialogDef *pDialog);
const DialogPageListDef *DialogDef_FindPageListFromDef(int hash, const DialogDef *pDialog);
const DialogPageDef *DialogDef_FindPageFromPageList(Entity *pPlayer, const DialogPageListDef *pPageList, const DialogDef *pDialog, StoryTaskInfo *pTask);
int DialogDef_FindAndHandlePage(Entity *pPlayer, Entity *pTarget, const DialogDef *pDialog, int hash, StoryTaskInfo *pTask, ContactResponse *response, ContactHandle contactHandle, ScriptVarsTable *vars);

void DialogDefs_Verify(const char *filename, const DialogDef **pDialogs);
void DialogDef_Verify(const char *filename, const DialogDef *pDialog);

void DialogDefList_Load(void);
const DialogDef* DialogDef_FindDefFromGlobalDefs(const char* filename, const char* dialogName);

#endif //__DIALOGDEF_H
