/***************************************************************************
 *     Copyright (c) 2000-2005, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 *
 * $Workfile: uiLoadCostume.c $
 * 
 ***************************************************************************/

#include "stringutil.h"
#include "utils.h"
#include "earray.h"
#include "entity.h"
#include "player.h"
#include "seq.h"
#include "seqstate.h"

#include "cmdgame.h"
#include "costume.h"
#include "entPlayer.h"
#include "entclient.h"
#include "costume_client.h"
#include "character_base.h"
#include "gameData/costume_data.h"
#include "language/langClientUtil.h"

#include "uiGame.h"
#include "uiUtil.h"
#include "uiInput.h"
#include "uiAvatar.h"
#include "uiTailor.h"
#include "uiCostume.h"
#include "uiGender.h"
#include "uiUtilGame.h"
#include "uiUtilMenu.h"  
#include "uiWindows.h"
#include "uiSupercostume.h"
#include "uiSuperRegistration.h"
#include "uiEditText.h"
#include "uiNet.h"

#include "ttFontUtil.h"
#include "sprite_base.h" 
#include "sprite_text.h"
#include "sprite_font.h"
#include "tex.h"

#include "uiListView.h"
#include "uiClipper.h"
#include "uiTailor.h"
#include "uiEdit.h"
#include "uiFocus.h"
#include "validate_name.h"
#include "EString.h"
#include "uiDialog.h"
#include "sound.h"
#include "file.h"
#include "ConvertUtf.h"
#include "font.h"

#include <direct.h> //for getcwd()



static int prevMenu = MENU_COSTUME;
static Costume *prevCostume;
extern int gE3CostumeCreator;
extern int gKoreanCostumeCreator;
extern int gLoadRandomPresetCostume;
extern int gLoadingCostume;

static void doScaling()
{
	Entity *e = playerPtr();

	switch( e->costume->appearance.bodytype )
	{
	case kBodyType_Huge:
		scaleHero( e, -10.f );
		break;
	case kBodyType_Female:
		scaleHero( e, 0.f );
		break;

	case kBodyType_Male:
	default:
		scaleHero( e, -10.f );
		break;
	}
}

void loadCostume_start(int previous_menu)
{
	Entity *e = playerPtr();

	texDisableThreadedLoading();
	entSetAlpha( e, 255, SET_BY_CAMERA );	

	prevMenu = previous_menu;
	prevCostume = costume_clone(e->costume);

	start_menu(MENU_LOADCOSTUME);
}

void loadCostume_cancel()
{
	Entity *e = playerPtr();

	texEnableThreadedLoading();

	costume_destroy(e->costume);
	e->pl->costume[0] = prevCostume;
	entSetMutableCostumeUnowned(e, e->pl->costume[0]);	// e->costume = e->pl->costume[0]
	prevCostume = 0;

	costume_Apply( e );

	gSelectedGeoSet = NULL;

	doScaling();
	start_menu( prevMenu );
}

void tailor_RevertAll(Entity *e);

void costume_setCurrentBodyType( Entity *e );

void loadCostume_next()
{
	Entity *e = playerPtr();

	texEnableThreadedLoading();

	costume_destroy(prevCostume);
	prevCostume = 0;

	gSelectedGeoSet = NULL;

	e->pl->costume[0] = e->costume;

	gCurrentBodyType = e->costume->appearance.bodytype;

	if (gCurrentBodyType == kBodyType_Female)
		gCurrentGender = AVATAR_GS_FEMALE;
	else if (gCurrentBodyType == kBodyType_Huge)
		gCurrentGender = AVATAR_GS_HUGE;
	else
		gCurrentGender = AVATAR_GS_MALE;

	costume_SetStartColors();
	costume_setCurrentCostume( e, 0 );
	tailor_matchSkin(e);
	genderSetScalesFromApperance(e);
	updateCostume();
	

	costume_Apply( e );

	doScaling();
	
	gLoadRandomPresetCostume = 0;

	start_menu( MENU_COSTUME );
}

UIBox costumesListViewDisplayItem(UIListView* list, UIPointFloatXYZ rowOrigin, void* userSettings, char* item, int itemIndex)
{
	UIBox box;
	UIPointFloatXYZ pen = rowOrigin;
	UIColumnHeaderIterator columnIterator; 
	int color;

	box.origin.x = rowOrigin.x;
	box.origin.y = rowOrigin.y;

	uiCHIGetIterator(list->header, &columnIterator);

	rowOrigin.z+=1;

	if(item == list->selectedItem)
	{
		color = CLR_WHITE;
	}
	else
	{
		color = CLR_GREY;
	}

	font_color(color, color);

	// Iterate through each of the columns.
	for(;uiCHIGetNextColumn(&columnIterator, list->scale);)
	{
		UIColumnHeader* column = columnIterator.column;
		UIBox clipBox;
		int rgba[4] = {color, color, color, color };

		if(item)
		{
			pen.x = rowOrigin.x + columnIterator.columnStartOffset*list->scale;

			clipBox.origin.x = pen.x;
			clipBox.origin.y = pen.y;
			clipBox.height = 20*list->scale;

			if( columnIterator.columnIndex >= EArrayGetSize(&columnIterator.header->columns )-1 )
				clipBox.width = 1000;
			else
				clipBox.width = columnIterator.currentWidth;

			clipperPushRestrict(&clipBox);

			if(str_wd_notranslate(&game_12, list->scale , list->scale, item) <= clipBox.width)
				font(&game_12);
			else
				font(&game_9);

			printBasic(font_grp, pen.x, pen.y+18*list->scale, rowOrigin.z, list->scale, list->scale, 0, item, strlen(item), rgba );

			clipperPop();
		}
	}

	box.height = 0;
	box.width = list->header->width;

	// Returning the bounding box of the item does not do anything right now.
	// See uiListView.c for more details.
	return box;
}

static UIListView *lvCostumes = 0;

char hexCharToChar(char* c)
{
	char newChar = 0;

	switch(*c)
	{
	case 'a':
		newChar |= 0xa0;
		break;
	case 'b':
		newChar |= 0xb0;
		break;
	case 'c':
		newChar |= 0xc0;
		break;
	case 'd':
		newChar |= 0xd0;
		break;
	case 'e':
		newChar |= 0xe0;
		break;
	case 'f':
		newChar |= 0xf0;
		break;
	default:
		{
			char tmp[2] = {*c,0};
			newChar |= (atoi(tmp) << 4) & 0xf0;
		}
		break;
	}

	switch(*(c+1))
	{
	case 'a':
		newChar |= 0x0a;
		break;
	case 'b':
		newChar |= 0x0b;
		break;
	case 'c':
		newChar |= 0x0c;
		break;
	case 'd':
		newChar |= 0x0d;
		break;
	case 'e':
		newChar |= 0x0e;
		break;
	case 'f':
		newChar |= 0x0f;
		break;
	default:
		{
			char tmp[2] = {*(c+1),0};
			newChar |= atoi(tmp) & 0x0f;
		}
		break;
	}

	return newChar;
}

void hexStrToStr(char *str, char *hexStr )
{
	unsigned int i;
	for ( i = 0; i < strlen(hexStr); i += 2 )
	{
		str[i>>1] = hexCharToChar(&hexStr[i]);
	}
}

void strToHexStr(char *hexStr, char *str )
{
	unsigned int i;
	hexStr[0] = 0;
	for ( i = 0; i < strlen(str); ++i )
	{
		strcatf(hexStr, "%0.2x", (unsigned char)str[i]);
	}
}

static FileScanAction fillCostumeListProcessor(char *dir, struct _finddata_t* data)
{
	if (simpleMatch("*.costume", data->name))
	{
		char *p;
		char *name = strdup(data->name);
		char newName[256] ={0};

		p = strrchr(name, '.');
		assert(p);
		*p = 0;

		hexStrToStr(newName, name);
		
		uiLVAddItem(lvCostumes, strdup(newName));
		free(name);
	}
	return FSA_NO_EXPLORE_DIRECTORY;
}

#include <wchar.h>
#include <mbstring.h>

void RecurseAndFillCostumeList(char *dir)
{
	char dir2[MAX_PATH];
	wchar_t newDir[MAX_PATH];
	struct wfinddata_t findData;
	int test, handle;

	strcpy(dir2, dir);
	strcat(dir2, "/*");

	strcpy( (char*)newDir, dir2 );
	utf8ToUtf16((char*)newDir, MAX_PATH);

	for ( test = handle = _wfindfirst(newDir, &findData); test >= 0; test = _wfindnext(handle, &findData) )
	{
		wchar_t *name, *p;

		if(findData.name[0] == _mbbtombc('.') )
			continue;

		if ( !(findData.attrib & _A_SUBDIR) )
		{
			char *tmp;
			name = _wcsdup(findData.name);
			p = wcschr(name, _mbbtombc('.'));
			assert(p);
			*p = 0;
			//utf16ToUtf8((char*)name, wcslen(name));
			tmp = strdup(WideToUTF8StrTempConvert(name, NULL));

			uiLVAddItem(lvCostumes, tmp);
		}
	}
}

void freeCostume(char *costumeStr)
{
	if (costumeStr)
		free(costumeStr);
}

static int refreshList = 0;

static void deleteCurrentCostume()
{
	if (lvCostumes->selectedItem)
	{
		char fname[MAX_PATH];
		char fixedName[MAX_PATH];

		strToHexStr(fixedName, lvCostumes->selectedItem);
		//sprintf(fname, "./costumes/%s.costume",lvCostumes->selectedItem);
		sprintf(fname, "./costumes/%s.costume",fixedName);
		remove(fname);
		refreshList = 1;
	}
}

void loadCostume_menu()
{
	static int init = 0;
	static int loadCost = 1;
	Entity *e = playerPtr();
	UIBox listViewDrawArea;
	UIPointFloatXYZ pen;
	int notReady = 0;

	listViewDrawArea.x = 100;
	listViewDrawArea.y = 76;
	listViewDrawArea.width = 500;
	listViewDrawArea.height = 596;
	pen.x = listViewDrawArea.x;
	pen.y = listViewDrawArea.y;
	pen.z = 5000;

	drawBackground( NULL );
	drawMenuTitle( 10, 30, 20, 450, textStd("LoadingCostume"), 1.f, !init );

	if( !init )
	{
		init = 1;

		// costume list
		if (!lvCostumes)
		{
			lvCostumes = uiLVCreate();
			uiLVHAddColumnEx(lvCostumes->header, "Name", "CostumeNameString", listViewDrawArea.width, listViewDrawArea.width, 0);
			uiLVEnableMouseOver(lvCostumes, 1);
			uiLVEnableSelection(lvCostumes, 1);
			lvCostumes->displayItem = costumesListViewDisplayItem;
		}

		refreshList = 1;
	}

	if (refreshList)
	{
		refreshList = 0;
		uiLVClearEx(lvCostumes, freeCostume);
		//if ( gE3CostumeCreator )
			fileScanDirRecurseEx("./costumes", fillCostumeListProcessor);
		//else if ( gKoreanCostumeCreator )
		//	RecurseAndFillCostumeList( "./costumes" );
	}

	// frame around guy
	drawMenuFrame( R12, 742, 64, 10, 240, 620, CLR_WHITE, 0x00000088, FALSE );

	drawMenuFrame( 12, listViewDrawArea.x-12, listViewDrawArea.y-12, 8, listViewDrawArea.width+24, listViewDrawArea.height+24, CLR_WHITE, 0x00000088, FALSE );
	uiLVSetDrawColor(lvCostumes, CLR_GREY);
	uiLVHSetDrawColor(lvCostumes->header, CLR_BLUE);
	uiLVSetHeight(lvCostumes, listViewDrawArea.height);
	uiLVHSetWidth(lvCostumes->header, listViewDrawArea.width);
	uiLVSetScale( lvCostumes, 1.f );
	clipperPushRestrict(&listViewDrawArea);
	uiLVDisplay(lvCostumes, pen);
	clipperPop();

	// Handle list view input.
	uiLVHandleInput(lvCostumes, pen);

	{
		int fullDrawHeight = uiLVGetFullDrawHeight(lvCostumes);
		int currentHeight = uiLVGetHeight(lvCostumes);
		if(fullDrawHeight > currentHeight)
		{
			doScrollBar( &lvCostumes->verticalScrollBar, currentHeight-12, fullDrawHeight, listViewDrawArea.x+listViewDrawArea.width-6, listViewDrawArea.y+5, pen.z+20 );
			lvCostumes->scrollOffset = lvCostumes->verticalScrollBar.offset;
		}
		else
		{
			lvCostumes->scrollOffset = 0;
		}
	}

	if (lvCostumes->selectedItem)
	{
		char fname[MAX_PATH];
		char fixedName[MAX_PATH];

		loadCost = 0;

		strToHexStr(fixedName, lvCostumes->selectedItem);
		//sprintf(fname, "./costumes/%s.costume",lvCostumes->selectedItem);
		sprintf(fname, "./costumes/%s.costume",fixedName);
		costume_load(fname);
	}
	else if (prevMenu == MENU_CCMAIN)
	{
		notReady = 1;
	}

	costume_Apply(e);

	drawSpinButtons(0,0);
	drawZoomButton();

	doScaling();

	if (notReady)
		toggle_3d_game_modes(SHOW_NONE);
	else
		toggle_3d_game_modes(SHOW_TEMPLATE);

	if (D_MOUSEHIT == drawStdButton(listViewDrawArea.x + listViewDrawArea.width * 0.5f, listViewDrawArea.y + listViewDrawArea.height + 12, 5000, 120, 30, CLR_RED, "DeleteCostumeString", 1.f, lvCostumes->selectedItem ? 0 : (1<<BUTTON_LOCKED)))
	{
		dialogStd( DIALOG_YES_NO, "DeleteCostumeConfirmString", NULL, NULL, deleteCurrentCostume, 0, 0 );
	}

	if ( drawNextButton( 40,  740, 10, 1.f, 0, 0, 0, "CancelString" ))
	{
		sndPlay( "back", SOUND_GAME );
		loadCostume_cancel();
		init = 0;
	}

	if ( drawNextButton( 980, 740, 5000, 1.f, 1, notReady, 0, "LoadString" ))
	{
		sndPlay( "next", SOUND_GAME );
		gLoadRandomPresetCostume = 0;
		gLoadingCostume = 1;
		loadCostume_next();
		init = 0;
	}
}


#define DEFAULT_CLR_BACK 0x00000088
#define DEFAULT_CLR_BORD CLR_WHITE
static Cbox box = {  100, 100, 350, 140 };
static Cbox out_path_box = {  100, 650, 930, 690 };


int isInvalidFilename(char *pch)
{
	// if the first character is a period, the file scan will not like it
	if (pch && *pch == '.')
		return 1;

	while(pch && *pch)
	{
		switch(*pch)
		{
			case '?':
			case '\\':
			case '//':
			case '*':
			case ':':
			case ',':
				return 1;
				break;
		}
		pch++;
	}
	return 0;
}


//#undef FILE
//#undef fclose
//#undef fprintf
//#include <stdio.h>
//
//#define COSTUME_STRING_LEN 512
//#define COSTUME_PART_COUNT 30
//
//int save_costume_to_file( Costume * pCostume, char * csvFileName )
//{
//	int i,k;
//	int n;
//	FILE *fp;
//	wchar_t newStr[MAX_PATH];
//	UTF8ToWideStrConvert(csvFileName, newStr, MAX_PATH);
//
//	fp = _wfopen(newStr, L"w");
//	if(fp==NULL)
//	{
//		printf("*** Error: Unable to open file '%s'\n", csvFileName);
//		return 0;
//	}
//
//	// PARAMS: X=512,Y=512
//	// PARAMS: BGTEXTURE=hazard_06_03.tga,BGCOLOR=0x9f9f9f
//	// PARAMS: OUTPUTNAME=c:\is\work\test.jpg
//	// Tight,Pants,Stars,Pants,-3684409,-3372800,NULL,NULL,1,-16756099,None,NULL,NULL
//
//
//
//	n = EArrayGetSize(&pCostume->parts);
//	for(i=0; i<n; i++)
//	{
//		CostumePart *ppart= pCostume->parts[i];
//
//		if(ppart->pchGeom!=NULL && ppart->pchGeom)
//		{
//			char region[COSTUME_STRING_LEN];
//			char *tmp;
//			fprintf(fp, "\"%s\",\"%s\",\"%s\",\"%s\",%d,%d,%d,%d",
//				ppart->pchGeom,
//				ppart->pchTex1,
//				ppart->pchTex2,
//				ppart->pchName,
//				ppart->color[0],
//				ppart->color[1],
//				pCostume->appearance.bodytype,
//				pCostume->appearance.colorSkin);
//
//			for(k=0;k<MAX_BODY_SCALES;k++)
//				fprintf(fp,",%f", pCostume->appearance.fScales[k]);
//
//			if(ppart->pchFxName)
//			{
//				fprintf(fp, ",\"%s\",%d,%d",
//					ppart->pchFxName,
//					ppart->color[2],
//					ppart->color[3]);
//			}
//			else
//			{
//				fprintf(fp,",0,0,0");
//			}
//
//			if( ppart->regionName)
//			{
//				strcpy(region,ppart->regionName);
//				tmp = strchr(region,' ');
//				if( tmp )
//					*tmp = '_';
//				
//				fprintf(fp, ",\"%s\",\"%s\"", region, ppart->bodySetName);
//			}
//			else
//			{
//				fprintf(fp, ",0,0");
//			}
//
//			fprintf(fp, "\n");
//		}
//	}
//
//	fclose(fp);
//	return 1;
//}


char gCostumeSaveFileName[256];
static UIEdit *saveNameEdit = 0;
static int uninitSaveMenu = 0;

void OverWriteCharacterFile(void)
{
	costume_save(gCostumeSaveFileName);
	start_menu(MENU_CCMAIN);
	//init = 0;

	uiEditSetUTF8Text(saveNameEdit, "");
	uninitSaveMenu = 1;
}

void saveCostume_menu()
{
	static int init = 0;
	Entity *e = playerPtr();

	drawBackground( NULL );
	drawMenuTitle( 10, 30, 20, 450, textStd("SavingCostume"), 1.f, !init );

	if( !init )
	{
		init = 1;

		moveAvatar( AVATAR_LOGIN);

		zoomstate = ZOOM_DEFAULT;
		gZoomed_in = 0;

	}

	switch( e->costume->appearance.bodytype )
	{
	case kBodyType_Huge:
		scaleHero( e, -10.f );
		break;
	case kBodyType_Female:
		scaleHero( e, 0.f );
		break;
	case kBodyType_Male:
	default:
		scaleHero( e, -10.f );  //mw changed when I changed male scale
		break;
	}

	seqSetState( playerPtrForShell(1)->seq->state, TRUE, STATE_PROFILE ); // freeze the player model
	seqSetState( playerPtrForShell(1)->seq->state, TRUE, STATE_HIGH ); // freeze the player model

	toggle_3d_game_modes(SHOW_TEMPLATE);

	drawMenuFrame( R12, 520, 60, 10, 410, 580, CLR_WHITE, DEFAULT_CLR_BACK, FALSE );


	// text entry
	{
		UIBox bounds;
		static float t = 0;
		int color = DEFAULT_CLR_BORD, colorBack = DEFAULT_CLR_BACK;

		t += TIMESTEP*.1;           // timer for the pulsing backgorund

		font(&game_18);

		if (!saveNameEdit)
		{
			uiBoxFromCBox(&bounds, &box);
			uiBoxAlter(&bounds, UIBAT_SHRINK, UIBAD_ALL, 5);
			uiBoxAlter(&bounds, UIBAT_SHRINK, UIBAD_LEFT, 5);
			saveNameEdit = uiEditCreate();
			saveNameEdit->textColor = CLR_BLACK;
			uiEditSetFont(saveNameEdit, font_grp);
	//		uiEditSetFontScale(saveNameEdit, 1.25);
			uiEditSetUTF8Text(saveNameEdit, e->name);
			saveNameEdit->z = 1010;
			saveNameEdit->limitType = UIECL_UTF8;
			saveNameEdit->limitCount = 15;
			saveNameEdit->disallowReturn = true;
			saveNameEdit->font = &game_18;
			saveNameEdit->textColor = CLR_WHITE;
			uiEditTakeFocus(saveNameEdit);
			uiEditSetBounds(saveNameEdit, bounds);
		}
		else
		{
			uiBoxFromCBox(&bounds, &box);
			uiBoxAlter(&bounds, UIBAT_SHRINK, UIBAD_ALL, 5);
			uiBoxAlter(&bounds, UIBAT_SHRINK, UIBAD_LEFT, 5);
			uiEditSetBounds(saveNameEdit, bounds);
		}

		//drawMenuFrame( R10, box.lx, box.ly, 20, box.hx-box.lx, box.hy-box.ly, CLR_WHITE, DEFAULT_CLR_BACK, FALSE );
		drawFlatFrame( PIX3, R10, box.lx, box.ly, 20, box.hx-box.lx, box.hy-box.ly, 1.f, CLR_BLUE, colorBack );
		drawFlatFrame( PIX3, R10, out_path_box.lx, out_path_box.ly, 20, out_path_box.hx-out_path_box.lx, 
			out_path_box.hy-out_path_box.ly, 1.f, CLR_BLUE, colorBack );

		{
			char *text = uiEditGetUTF8Text(saveNameEdit);
			if(text)
			{
				char space[2] = " ";
				char *pch;
				int len;
				
				//stripNonAsciiChars(text);

				len = strlen(text);

				// if the string is only whitespace, delete it
				if( strspn(text, space ) == len )
				{
					text[0] = '\0';
				}

				while((pch=strstr(text, "  "))!=NULL)
				{
					strcpy(pch, pch+1);
				}

				{
					int idx = saveNameEdit->cursor.characterIndex;
					uiEditSetUTF8Text(saveNameEdit, text);
					uiEditSetCursorPos(saveNameEdit, idx);
				}
				estrDestroy(&text);
			}
		}

		uiEditProcess(saveNameEdit);

		if(saveNameEdit->contentCharacterCount == 0)
		{
			font_color( CLR_CONSTRUCT(128, 128, 128, (int)(120+120*sinf(t))), CLR_CONSTRUCT(128, 128, 128, (int)(120+120*sinf(t))) );
			prnt( box.lx + 30, box.hy - 12, 30, 1.f, 1.f, "EnterNameHere" );
		}
		else
		{
			char buf[MAX_PATH] = {0,};
			char *text = uiEditGetUTF8Text(saveNameEdit);
			getcwd(buf, MAX_PATH);
			sprintf(buf, "%s\\costumes\\%s.costume", buf, text);
			font_color( CLR_YELLOW, CLR_ORANGE );

			while ( str_layoutWidth(font_get(), 1.0f, 1.0f, buf) > 670 )
			{
				strcpy( buf, &buf[1] );
			}
			//buf[64] = 0; // this is an ugly way to prevent it from running off the edge of the box
			prnt( out_path_box.lx + 10, out_path_box.hy - 12, 30, 1.0f, 1.f, buf );
		}
	}
	// END text entry


	if ( drawNextButton( 40,  740, 1001, 1.f, 0, 0, 0, "BackString" ))
	{
		dialogClearQueue( 0 );
		sndPlay( "back", SOUND_GAME );
		start_menu(MENU_CCANIMATION);
		init = 0;
	}

	if ( drawNextButton( 980, 740, 5000, 1.f, 1, saveNameEdit->contentCharacterCount == 0, 0, "NextString" ))
	{
		char *text = uiEditGetUTF8Text(saveNameEdit);

		sndPlay( "next", SOUND_GAME );

		if(text)
		{
			ValidateNameResult vnr = ValidateName(text, NULL, true, 20);

			if(vnr!=VNR_Valid)
			{
				switch(vnr)
				{
				case VNR_Malformed:
				case VNR_Profane:
				case VNR_Reserved:
				case VNR_WrongLang:
				case VNR_TooLong:
				default:
					dialogStd( DIALOG_OK, "InvalidName", NULL, NULL, NULL, NULL, 0 );
					break;

				case VNR_Titled:
					dialogStd( DIALOG_OK, "InvalidNameTitled", NULL, NULL, NULL, NULL, 0 );
					break;
				}
			}
			else
			{
				if ( isInvalidFilename(text) )
				{
					dialogStd( DIALOG_OK, "InvalidName", NULL, NULL, NULL, NULL, 0 );
				}
				//else if(multipleNonAciiCharacters(text) )
				//{
				//	dialogStd( DIALOG_OK, "MultipleNonAsciiChars", NULL, NULL, NULL, NULL,0 );
				//}
				else
				{
					char fname[MAX_PATH];
					char pname[MAX_PATH];
					FILE * open_test = NULL;
					
					//wchar_t fname[MAX_PATH];
					//wchar_t pname[MAX_PATH], pname2[MAX_PATH], newText[MAX_PATH];
					//FILE * open_test = NULL;
					//narrowToWideStrCopy( "./costumes", pname );
					//_wmkdir(pname);
					//narrowToWideStrCopy( ".costume", pname2 );
					//UTF8ToWideStrConvert(text, newText, MAX_PATH);
					//swprintf(fname, L"%s/%s%s",pname, newText, pname2);
					
					strcpy(pname, "./costumes");
					//change text to a hex string
					{
						char tempText[256] = {0};
						strToHexStr(tempText, text);
						sprintf(fname, "%s/%s.costume",pname, tempText);
					}
					makeDirectories(pname);
					strcpy( gCostumeSaveFileName, fname );
					
					open_test = fopen( fname, "rb" );
					//open_test = _wfopen( fname, L"rb" );
					if ( open_test )
					{
						dialogStd( DIALOG_YES_NO, "FileExists", NULL, NULL, OverWriteCharacterFile, NULL, 0 );
						fclose(open_test);
					}
					else
					{
						//Entity *e = playerPtr();
						costume_save(fname);
						//save_costume_to_file(e->costume, fname);
						start_menu(MENU_CCMAIN);
						init = 0;

						uiEditSetUTF8Text(saveNameEdit, "");
					}
				}
			}

			estrDestroy(&text);
		}
		else
		{
			dialogStd( DIALOG_OK, "InvalidName", NULL, NULL, NULL, NULL, 0 );
		}
	}

	if ( uninitSaveMenu == 1 )
	{
		init = 0;
		uninitSaveMenu = 0;
	}
}

/* End of File */









