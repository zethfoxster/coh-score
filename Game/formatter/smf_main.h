#ifndef SMF_MAIN_H__
#define SMF_MAIN_H__

#ifndef _STDTYPES_H
#include "stdtypes.h" // for bool
#endif

#include "smf_util.h"

extern TextAttribs gTextAttr_Gray12;
extern TextAttribs gTextAttr_White9;
extern TextAttribs gTextAttr_White12;
extern TextAttribs gTextAttr_WhiteNoOutline12;
extern TextAttribs gTextAttr_Chat;
extern TextAttribs gTextAttr_Arena;
extern TextAttribs gTextAttr_Green;
extern TextAttribs gTextAttr_LightHybridBlueHybrid12;
extern TextAttribs gTextAttr_WhiteHybridBold12;
extern TextAttribs gTextAttr_WhiteHybridBold18;
extern TextAttribs gTextAttr_WhiteTitle18Italics;
extern TextAttribs gTextAttr_WhiteTitle24Italics;
// TODO: Go through code and put all the other static TextAttribs here

void smf_HandlePerFrame();

TextAttribs *smf_CreateTextAttribs();
TextAttribs *smf_CreateDefaultTextAttribs();
void smf_DestroyTextAttribs(TextAttribs *pattrs);
void smf_DestroyDefaultTextAttribs(TextAttribs *pattrs);
void smf_InitTextAttribs(TextAttribs *pattrs, TextAttribs *pdefaults);
void smf_InitDefaultTextAttribs(TextAttribs *pattrs, TextAttribs *pdefaults);

void smf_NotifyOfExternalScrollDiff(int yDiff);
int smf_GetInternalScrollDiff(SMFBlock *pSMFBlock);
void smf_SetFlags(SMFBlock *pSMFBlock, SMFEditMode editMode, SMFLineBreakMode lineBreakMode, 
				  SMFInputMode inputMode, int inputLimit, SMFScrollMode scrollMode, 
				  SMFOutputMode outputMode, SMFDisplayMode displayMode, SMFContextMenuMode contextMenuMode,
				  SMAlignment defaultHorizAlign, char *coselectionSetName,
				  char *tabSelectionSetName, int tabSelectionSetIndex);
void smf_SetScissorsBox(SMFBlock *pSMFBlock, float xPos, float yPos, float width, float height);
void smf_ClearScissorsBox(SMFBlock *pSMFBlock);
void smf_SetMasterScale(SMFBlock *pSMFBlock, float scale);
void smf_SetRawText(SMFBlock *pSMFBlock, const char *pch, bool encodeCharacters);
void smf_InsertIntoRawText(SMFBlock *pSMFBlock, char *pch, int insertPos, bool encodeCharacters);
void smf_SetCharacterInsertSounds(SMFBlock *pSMFBlock, char *successfulSound, char *failedSound);
int smf_ParseAndFormat(SMFBlock *pSMFBlock, char *pch, int x, int y, int z, int w, int h, 
					   bool isAllowedToInteract, bool bReparse, bool bReformat, 
					   TextAttribs *ptaDefaults, int (*callback)(char *));
int smf_Display(SMFBlock *pSMFBlock, int x, int y, int z, int w, int h, 
				bool bReparse, bool bReformat, TextAttribs *ptaDefaults, 
				int (*callback)(char *pch));
int smf_DisplayEx(SMFBlock *pSMFBlock, int x, int y, int z, int w, int h, 
				bool bCanInteract, bool bReparse, bool bReformat, TextAttribs *ptaDefaults, 
				int (*callback)(char *pch));

void smf_CopyStringToClipboard(void);

// Deprecated
int smf_ParseAndDisplay(SMFBlock *pSMFBlock, const char *pch, int x, int y, int z, int w, int h,
						bool bReparse, bool bReformat, TextAttribs *ptaDefaults, 
						int (*callback)(char *), char *coselectionSetName, bool selectable);

// Extra functions, not used as a part of standard SMF usage... needs to be integrated better with
// the rest of the character code handling functionality...
void smf_EncodeAllCharactersInPlace(char **eString);
void smf_DecodeAllCharactersInPlace(char **eString);



#endif // SMF_MAIN_H__
