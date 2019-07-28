#include "uiAuctionHouseIncludeInternal.h"
#include "uiAuctionHouse.h"
#include "uiAmountSlider.h"

UIEdit *sellItemEdit = NULL;
TrayObj *auctionInvTray = NULL;

static TextAttribs gAuctionTextAttr =
{
	/* piBold        */  (int *)0,
	/* piItalic      */  (int *)0,
	/* piColor       */  (int *)0x48ff48ff,
	/* piColor2      */  (int *)0,
	/* piColorHover  */  (int *)0x48ff48ff,
	/* piColorSelect */  (int *)0,
	/* piColorSelectBG*/ (int *)0x333333ff,
	/* piScale       */  (int *)(int)(1.0f*SMF_FONT_SCALE),
	/* piFace        */  (int *)&game_12,
	/* piFont        */  (int *)0,
	/* piAnchor      */  (int *)0,
	/* piLink        */  (int *)0,
	/* piLinkBG      */  (int *)0,
	/* piLinkHover   */  (int *)0,
	/* piLinkHoverBG */  (int *)0,
	/* piLinkSelect  */  (int *)0,
	/* piLinkSelectBG*/  (int *)0x666666ff,
	/* piOutline     */  (int *)0,
	/* piShadow      */  (int *)0,
};

int getItemColor(AuctionItem *itm, int selected)
{
	int alpha = selected ? 0xcc : 0x44;
	int color = 0;

	switch ( itm->auction_status )
	{
	case AuctionInvItemStatus_ForSale:
		color = 0x33338800 | alpha;
	xcase AuctionInvItemStatus_Bidding:
		color = 0x22882200 | alpha;
	xcase AuctionInvItemStatus_Sold:
		color = 0x5555cc00 | alpha;
	xcase AuctionInvItemStatus_Bought:
		color = 0x55cc5500 | alpha;
	xcase AuctionInvItemStatus_Stored:
		color = 0x88888800 | alpha;
		break;
	}

	return color;
}

void auctionAmountSliderCallback(int amount, void *userData)
{
	AuctionItem *itm = (AuctionItem*)userData;
	if ( itm )
	{
		//auctionHouseAddItem(itm->type, itm->name, itm->lvl, itm->auction_id, amount, itm->infPrice,
		//	itm->auction_status, itm->id);
	}
}



void displayInventory(Entity *e, float x, float y, float z, float wd, float ht, float sc, int clr, int bkclr)
{
	//int i;
	//CBox framebox = {x, y, x+wd, y+ht},	sellbox = {0};
	//UIBox uibox;
	//static ScrollBar sb = { WDW_AUCTIONHOUSE };
	//int invSize = e->pchar->auctionInv ? e->pchar->auctionInv->invSize : 0;
	//static int draggingFromInv = 0;
	//float sprite_y = y + dividerOffset*sc, sprite_sc = ((ht - (dividerOffset + INVENTORY_H + 11)*sc))/(white_tex_atlas->height);
	//float item_y = sprite_y + (white_tex_atlas->height*sprite_sc);
	//int unclaimed_inf = 0;
	//enum
	//{
	//	kNone,
	//	kAmtStored,
	//	kInfStored,
	//} currentInvItemGet = kNone;

	//updateAuctionInventory(e);
	//uiBoxFromCBox(&uibox, &framebox);
	//uibox.y += (R12/2 - 2)*sc;
	//uibox.height -= (R12 - 4)*sc;

	//if(e->pchar->auctionInv && !invSize)
	//	invSize = eaSize(&localInventory) + 1;

	//if ( !sellItemEdit )
	//{
	//	INIT_EDIT_BOX(sellItemEdit, 13);
	//	sellItemEdit->rightAlign = true;
	//	sellItemEdit->noClip = true;
	//	sellItemEdit->inputHandler = uiEditCommaNumericInputHandler;
	//}

	//if ( currentInvItem >= eaSize(&localInventory) )
	//{
	//	currentInvItem = eaSize(&localInventory)-1;
	//}

	//if ( currentInvItem != -1 )
	//{
	//	display_sprite( white_tex_atlas, x, sprite_y, z, wd/white_tex_atlas->width, sprite_sc,
	//		getItemColor(localInventory[currentInvItem], 1));
	//}

	//for ( i = 0; i < eaSize(&localInventory) && i < invSize; ++i )
	//{
	//	int yPos = item_y;//- (DISPLAYITEM_H/2 + 12)*sc;
	//	int color = CLR_FRAME_GREY;
	//	AtlasTex *icon = NULL;
	//	CBox itembox = { x + (INVENTORY_X+i*INVENTORY_W)*sc - sb.offset,
	//		yPos,// + INVENTORYITEM_Y*sc,
	//		x + (INVENTORY_X + i*INVENTORY_W + INVENTORY_W)*sc - sb.offset,
	//		yPos + (/*INVENTORYITEM_Y + */INVENTORY_H)*sc  };
	//	CBox itembox2 = { x + (INVENTORY_X+i*INVENTORY_W-1)*sc - sb.offset,
	//		yPos,// + INVENTORYITEM_Y*sc,
	//		x + (INVENTORY_X + i*INVENTORY_W + INVENTORY_W+1)*sc - sb.offset,
	//		yPos + (/*INVENTORYITEM_Y + */INVENTORY_H+4)*sc  };
	//	char *infoStr = NULL;
	//	char *itemStatusString = NULL;


	//	// set the name for the current item
	//	if ( localInventory[i]->pItem && !localInventory[i]->displayName )
	//	{
	//		switch (localInventory[i]->type)
	//		{
	//		case kTrayItemType_Salvage:
	//			localInventory[i]->displayName = localInventory[i]->salvage->ui.pchDisplayName;
	//		xcase kTrayItemType_Recipe:
	//			localInventory[i]->displayName = localInventory[i]->recipe->ui.pchDisplayName;
	//		xcase kTrayItemType_Inspiration:
	//			localInventory[i]->displayName = localInventory[i]->inspiration->pchDisplayName;
	//		xcase kTrayItemType_SpecializationInventory:
	//			localInventory[i]->displayName = localInventory[i]->enhancement->pchDisplayName;
	//			break;
	//		}
	//	}

	//	color = getItemColor(localInventory[i], i == currentInvItem);

	//	switch ( localInventory[i]->auction_status )
	//	{
	//	case AuctionInvItemStatus_ForSale:
	//		strdup_alloca(infoStr, textStd("AuctionSelling",localInventory[i]->amtStored, localInventory[i]->infPrice));
	//		itemStatusString = "SellingString";
	//	xcase AuctionInvItemStatus_Bidding:
	//		strdup_alloca(infoStr, textStd("AuctionLookingToBuy", localInventory[i]->amtOther, localInventory[i]->infPrice));
	//		itemStatusString = "BiddingString";
	//	xcase AuctionInvItemStatus_Sold:
	//		{
	//			int fee = calcAuctionSoldFee(localInventory[i]->infStored, localInventory[i]->infPrice, localInventory[i]->amtOther);
	//			strdup_alloca(infoStr, textStd("AuctionSold", localInventory[i]->infStored - fee, localInventory[i]->infStored, fee));
	//			itemStatusString = "SoldString";
	//			unclaimed_inf = 1;
	//		}
	//	xcase AuctionInvItemStatus_Bought:
	//		strdup_alloca(infoStr, textStd("AuctionBought", localInventory[i]->infPrice));
	//		itemStatusString = "BoughtString";
	//	xcase AuctionInvItemStatus_Stored:
	//		itemStatusString = "StoredString";
	//		break;
	//	}


	//	if ( localInventory[i]->auction_status == AuctionInvItemStatus_Stored )
	//	{
	//		// this item is in limbo, it can either be put up for sale or removed from the inventory
	//		if ( i == currentInvItem )
	//			drawTabbedFlatFrameBox(PIX3, R10, &itembox2, AUCTION_Z, sc, color, color, kTabDir_Top);
	//		else
	//			drawFlatFrameBox(PIX3, R10, &itembox, AUCTION_Z, color, color);

	//		if ( i == currentInvItem )
	//		{
	//			char *priceStr;
	//			int price;
	//			int fee = 0;
	//			UIBox sellboxbounds;
	//			char *sellingForString = textStd("SellingAmountFor", localInventory[i]->amtStored);

	//			font(&game_14);
	//			font_color( CLR_WHITE, CLR_WHITE );
	//			prnt(x + INVENTORYINFO_X*sc, y + (INVENTORYBUTTONS_Y+19)*sc, z, sc, sc, sellingForString);

	//			sellbox.left = x + INVENTORYINFO_X*sc + str_wd(font_get(), sc, sc, sellingForString) + 5*sc;
	//			//sellbox.left = x + INVENTORYINFO_X*sc + DETAILITEM_SELLBOX_X*sc;
	//			sellbox.top = y + (INVENTORYBUTTONS_Y)*sc;
	//			sellbox.right = sellbox.left + DETAILITEM_SELLBOX_W*sc;
	//			sellbox.bottom = sellbox.top + STD_EDIT_H*sc;

	//			uiBoxFromCBox(&sellboxbounds, &sellbox);
	//			uiBoxAlter(&sellboxbounds, UIBAT_SHRINK, UIBAD_LEFT, R10*sc);
	//			uiBoxAlter(&sellboxbounds, UIBAT_SHRINK, UIBAD_RIGHT, R10*sc);
	//			uiBoxAlter(&sellboxbounds, UIBAT_SHRINK, UIBAD_TOP, PIX3*sc);
	//			sellItemEdit->z = AUCTION_Z;
	//			uiEditSetBounds(sellItemEdit, sellboxbounds);
	//			uiEditSetFontScale(sellItemEdit, sc);
	//			uiEditSetFont(searchResultEdit, &game_12);

	//			if( mouseCollision( &sellbox ) )
	//			{
	//				clr = CLR_WHITE;
	//			}

	//			if( mouseDownHit( &sellbox, MS_LEFT ) )
	//			{
	//				uiEditTakeFocus(sellItemEdit);
	//			}

	//			drawFrameBox( PIX3, R10, &sellbox, AUCTION_Z, clr, bkclr );
	//			uiEditProcess(sellItemEdit);

	//			font(&game_14);
	//			font_color( CLR_WHITE, CLR_WHITE );
	//			prnt(sellbox.right+4*sc, y + (INVENTORYBUTTONS_Y+19)*sc, AUCTION_Z, sc, sc, "%s", textStd("EachString"));

	//			priceStr = uiEditGetUTF8Text(sellItemEdit);
	//			if ( priceStr )
	//			{
	//				price = atoi_ignore_nonnumeric(priceStr);
	//				if ( price > MAX_INFLUENCE )
	//				{
	//					char infstr[MAX_INF_LENGTH_WITH_COMMAS];

	//					price = MAX_INFLUENCE;
	//					uiEditSetUTF8Text(sellItemEdit, itoa_with_commas(price, infstr));
	//				}

	//				fee = calcAuctionSellFee(price) * localInventory[currentInvItem]->amtStored;
	//			}
	//			else
	//			{
	//				font(&game_12);
	//				font_color( 0xffffff77, 0xffffff77 );
	//				prnt(sellboxbounds.x, sellboxbounds.y + ((DISPLAYITEM_EDIT_H/2+PIX3)*sc), z, sc, sc, "EnterPrice");
	//			}

	//			font(&game_9);
	//			if ( fee > e->pchar->iInfluencePoints )
	//				font_color( CLR_RED, CLR_RED );
	//			else
	//				font_color( CLR_GREEN, CLR_GREEN );
	//			prnt(sellbox.right+50*sc, y + (INVENTORYBUTTONS_Y+19)*sc, AUCTION_Z, sc, sc,
	//				"%s", textStd("TotalFeeColonValue", priceStr ? fee : 0));

	//			if ( drawStdButtonTopLeft( x + DETAILITEM_POSTBUTTON_OFFSET*sc, sellbox.top, AUCTION_Z, DETAILITEM_POSTBUTTON_W*sc, DETAILITEM_SELLBOX_H*sc, CLR_BLUE, "PostItem", sc,
	//				!priceStr || price <= 0 || fee > e->pchar->iInfluencePoints || g_auc.uiDisabled ? BUTTON_LOCKED : 0) == D_MOUSEDOWN ||
	//				(uiEditHasFocus(sellItemEdit) && inpEdge(INP_RETURN)) )
	//			{
	//				if ( priceStr )
	//				{
	//					auction_changeItemStatus(localInventory[currentInvItem]->auction_id, price, AuctionInvItemStatus_ForSale);
	//					uiEditClear(sellItemEdit);
	//					estrDestroy(&priceStr);
	//				}
	//			}
	//			if ( drawStdButtonTopLeft( x + (DETAILITEM_POSTBUTTON_OFFSET+DETAILITEM_POSTBUTTON_W+10)*sc, sellbox.top, AUCTION_Z,
	//				DETAILITEM_POSTBUTTON_W*sc, DETAILITEM_SELLBOX_H*sc, CLR_BLUE, "GetFromInv", sc, 0)
	//				== D_MOUSEHIT )
	//			{
	//				currentInvItemGet = kAmtStored;
	//			}
	//		}
	//	}
	//	else
	//	{
	//		if ( i == currentInvItem && localInventory[i] )
	//		{
	//			char *infoStr1 = NULL;
	//			char *infoStr2 = NULL;

	//			switch ( localInventory[i]->auction_status )
	//			{
	//				case AuctionInvItemStatus_ForSale:
	//					if ( localInventory[i]->infStored )
	//					{
	//						int fee = calcAuctionSoldFee(localInventory[i]->infStored, localInventory[i]->infPrice, localInventory[i]->amtOther);
	//						strdup_alloca(infoStr2, textStd("AuctionSold", localInventory[i]->amtOther, localInventory[i]->infStored - fee, localInventory[i]->infStored, fee));
	//					}
	//					if ( localInventory[i]->amtStored )
	//						strdup_alloca(infoStr1, textStd("AuctionSelling",localInventory[i]->amtStored, localInventory[i]->infPrice));

	//				xcase AuctionInvItemStatus_Bidding:
	//					if ( localInventory[i]->amtStored )
	//						strdup_alloca(infoStr2, textStd("AuctionBought", localInventory[i]->amtStored, localInventory[i]->infPrice));
	//					if ( localInventory[i]->amtOther )
	//						strdup_alloca(infoStr1, textStd("AuctionLookingToBuy", localInventory[i]->amtOther, localInventory[i]->infPrice));

	//				xcase AuctionInvItemStatus_Sold:
	//					{
	//						int fee = calcAuctionSoldFee(localInventory[i]->infStored, localInventory[i]->infPrice, localInventory[i]->amtOther);
	//						strdup_alloca(infoStr1, textStd("AuctionSold", localInventory[i]->amtOther, localInventory[i]->infStored - fee, localInventory[i]->infStored, fee));
	//					}

	//				xcase AuctionInvItemStatus_Bought:
	//					strdup_alloca(infoStr1, textStd("AuctionBought", localInventory[i]->amtStored, localInventory[i]->infPrice));

	//				xcase AuctionInvItemStatus_Stored:
	//					break;
	//			}

	//			drawTabbedFlatFrameBox(PIX3, R10, &itembox2, AUCTION_Z, sc, color, color, kTabDir_Top);
	//			if ( infoStr1 )
	//			{
	//				int strWd;

	//				font(&game_14);
	//				font_color( CLR_WHITE, CLR_WHITE );
	//				prnt(x + INVENTORYINFO_X*sc, y + (INVENTORYBUTTONS_Y+10)*sc, AUCTION_Z, sc, sc, "%s", infoStr1);
	//				strWd = str_wd(font_get(), sc, sc, infoStr1);

	//				// this button will either cancel a bid or get influence from a sold item, or get an item if its for sale and not bought
	//				if ( drawStdButtonTopLeft( x + (INVENTORYINFO_X + 5)*sc + strWd, y + (DETAILITEM_GETBUTTON_Y)*sc, AUCTION_Z,
	//					DETAILITEM_GETBUTTON_W*sc, DETAILITEM_GETBUTTON_H*sc, CLR_BLUE,
	//					localInventory[currentInvItem]->auction_status == AuctionInvItemStatus_Bidding ? "CancelBid" :
	//					localInventory[currentInvItem]->auction_status == AuctionInvItemStatus_Sold ? "GetInf" : "GetFromInv", sc, g_auc.uiDisabled ? BUTTON_LOCKED : 0)
	//					== D_MOUSEHIT )
	//				{
	//					if ( localInventory[currentInvItem]->auction_status == AuctionInvItemStatus_Bidding ||
	//						localInventory[currentInvItem]->auction_status == AuctionInvItemStatus_Sold )
	//					{
	//						currentInvItemGet = kInfStored;
	//					}
	//					else
	//					{
	//						currentInvItemGet = kAmtStored;
	//					}
	//				}
	//			}
	//			if ( infoStr2 && currentInvItem >= 0 )
	//			{
	//				int strWd;

	//				font(&game_14);
	//				font_color( CLR_WHITE, CLR_WHITE );
	//				prnt(x + INVENTORYINFO_X*sc, y + (INVENTORYBUTTONS_Y+30)*sc, AUCTION_Z, sc, sc, "%s", infoStr2);
	//				strWd = str_wd(font_get(), sc, sc, infoStr2);

	//				// this button will either get a bought item, or get influence from a sold item
	//				if ( drawStdButtonTopLeft( x + (INVENTORYINFO_X + 5)*sc + strWd, y + (DETAILITEM_GETBUTTON_Y+DETAILITEM_GETBUTTON_H)*sc, AUCTION_Z,
	//					DETAILITEM_GETBUTTON_W*sc, DETAILITEM_GETBUTTON_H*sc, CLR_BLUE,
	//					localInventory[currentInvItem]->auction_status == AuctionInvItemStatus_Bidding ? "GetFromInv" :
	//					localInventory[currentInvItem]->auction_status == AuctionInvItemStatus_ForSale ? "GetInf" : "GetFromInv", sc, g_auc.uiDisabled ? BUTTON_LOCKED : 0)
	//					== D_MOUSEHIT )
	//				{
	//					if ( localInventory[currentInvItem]->auction_status == AuctionInvItemStatus_ForSale )
	//					{
	//						currentInvItemGet = kInfStored;
	//					}
	//					else
	//					{
	//						currentInvItemGet = kAmtStored;
	//					}
	//				}
	//			}
	//		}
	//		else
	//			drawFlatFrameBox(PIX3, R10, &itembox, AUCTION_Z, color, color);
	//	}

 //		if ( i == currentInvItem )
 //		{
 //			font(&game_14);
 //			font_color( CLR_WHITE, CLR_WHITE );
 //			prnt(x + INVENTORYINFO_X*sc, y + (dividerOffset + INVENTORYNAME_Y)*sc, AUCTION_Z, sc, sc, textStd(localInventory[i]->displayName) );
	//	}

	//	if ( localInventory[i]->auction_status != AuctionInvItemStatus_Bought && 
	//		localInventory[i]->auction_status != AuctionInvItemStatus_Sold )
	//	{
	//		float tooltipOffsetY = displayingExtraInfo ? 0 : (TOOLTIPOFFSETY*sc);

	//		if ( (!(localInventory[i]->auction_status == AuctionInvItemStatus_ForSale && localInventory[i]->infStored)) &&
	//			 (!(localInventory[i]->auction_status == AuctionInvItemStatus_Bidding && localInventory[i]->amtStored)))
	//		{
	//			if (localInventory[i]->type == kTrayItemType_Recipe)
	//				icon = drawAuctionItemIcon(localInventory[i], itembox.left + 21*sc, itembox.top + 15*sc, AUCTION_Z, sc*0.7f, sc*0.9f, tooltipOffsetY);
	//			else
	//				icon = drawAuctionItemIcon(localInventory[i], itembox.left + 29*sc, itembox.top + 15*sc, AUCTION_Z, sc*0.7f, sc*0.9f, tooltipOffsetY);
	//		}

	//		font(&game_14);
	//		font_color( CLR_WHITE, CLR_WHITE );
	//		if ( (localInventory[i]->auction_status == AuctionInvItemStatus_Bidding && localInventory[i]->amtOther > 1) ||
	//			localInventory[i]->amtStored > 1 )
	//			cprnt(itembox.left + 51*sc,itembox.top+14*sc, AUCTION_Z, sc*0.6f, sc*0.6f, "%s %d", textStd(itemStatusString), localInventory[i]->auction_status == AuctionInvItemStatus_Bidding ? localInventory[i]->amtOther : localInventory[i]->amtStored);
	//		else
	//			cprnt(itembox.left + 51*sc, itembox.top+14*sc, AUCTION_Z, sc*0.6f, sc*0.6f, "%s", textStd(itemStatusString));


	//		if ( localInventory[i]->auction_status != AuctionInvItemStatus_Stored )
	//		{
	//			int price = localInventory[i]->infPrice;
	//			int status = localInventory[i]->auction_status;
	//			char infstr[MAX_INF_LENGTH_WITH_COMMAS];

	//			if(((status==AuctionInvItemStatus_Bought||status==AuctionInvItemStatus_ForSale)
	//					&& localInventory[i]->amtStored>1)
	//				|| (status==AuctionInvItemStatus_Bidding
	//					&& localInventory[i]->amtOther>1))
	//			{
	//				cprnt(itembox.left + 51*sc, itembox.bottom - 3*sc, AUCTION_Z, sc*0.6f, sc*0.6f, "%s %s", itoa_with_commas(price, infstr), textStd("EachString"));
	//			}
	//			else
	//			{
	//				cprnt(itembox.left + 51*sc, itembox.bottom - 3*sc, AUCTION_Z, sc*0.6f, sc*0.6f, "%s", itoa_with_commas(price, infstr));
	//			}
	//		}
	//	}

	//	if ( localInventory[i]->auction_status == AuctionInvItemStatus_Bought ||
	//			localInventory[i]->auction_status == AuctionInvItemStatus_Sold ||
	//			(localInventory[i]->auction_status == AuctionInvItemStatus_ForSale && localInventory[i]->infStored) ||
 //				(localInventory[i]->auction_status == AuctionInvItemStatus_Bidding && localInventory[i]->amtStored))
	//	{
	//		static int colorTimer = 0;
	//		int color;
	//		float tooltipOffsetY = displayingExtraInfo ? 0 : (TOOLTIPOFFSETY*sc);

	//		if ( !colorTimer )
	//			colorTimer = timerAlloc();

	//		interp_rgba(MAX(0.0f, MIN((sin(timerElapsed(colorTimer)*3.0f)+1.0f)/2,1.0f)),
	//			CLR_PARAGON, CLR_WHITE, &color);
	//			
	//		if (localInventory[i]->type == kTrayItemType_Recipe)
	//			icon = drawAuctionItemIconEx(localInventory[i], itembox.left + 17*sc, itembox.top + 15*sc, AUCTION_Z, sc*0.7f, sc, 0xffffff88, tooltipOffsetY);
	//		else
	//			icon = drawAuctionItemIconEx(localInventory[i], itembox.left + 29*sc, itembox.top + 15*sc, AUCTION_Z, sc*0.7f, sc, 0xffffff88, tooltipOffsetY);

	//		font(&title_14);
	//		font_color( CLR_WHITE, color );

	//		if ( localInventory[i]->auction_status == AuctionInvItemStatus_Bought )
	//		{
	//			if ( localInventory[i]->amtStored > 1 )
	//			{
	//				cprnt(itembox.left + 49*sc, itembox.top + 35*sc, AUCTION_Z+5, sc*1.1, sc*1.1, "%d", localInventory[i]->amtStored);
	//				cprnt(itembox.left + 49*sc, itembox.top + 55*sc, AUCTION_Z+5, sc*1.1, sc*1.1, "BoughtItem");
	//			}
	//			else
	//				cprnt(itembox.left + 49*sc, itembox.top + 45*sc, AUCTION_Z+5, sc*1.1, sc*1.1, "BoughtItem");
	//		}
	//		else if ( localInventory[i]->auction_status == AuctionInvItemStatus_Sold )
	//		{
	//			if ( localInventory[i]->amtOther > 1 )
	//			{
	//				cprnt(itembox.left + 49*sc, itembox.top + 35*sc, AUCTION_Z+5, sc*1.1, sc*1.1, "%d", localInventory[i]->amtOther);
	//				cprnt(itembox.left + 49*sc, itembox.top + 55*sc, AUCTION_Z+5, sc*1.1, sc*1.1, "SoldItem");
	//			}
	//			else
	//				cprnt(itembox.left + 49*sc, itembox.top + 45*sc, AUCTION_Z+5, sc*1.1, sc*1.1, "SoldItem");
	//		}
	//		else if ( localInventory[i]->auction_status == AuctionInvItemStatus_ForSale && localInventory[i]->infStored )
	//		{
	//			cprnt(itembox.left + 49*sc, itembox.top + 35*sc, AUCTION_Z+5, sc*1.1, sc*1.1, "%d", localInventory[i]->amtOther);
	//			cprnt(itembox.left + 49*sc, itembox.top + 55*sc, AUCTION_Z+5, sc*1.1, sc*1.1, "SoldItem");
	//		}
	//		else if ( localInventory[i]->auction_status == AuctionInvItemStatus_Bidding && localInventory[i]->amtStored )
	//		{
	//			cprnt(itembox.left + 49*sc, itembox.top + 35*sc, AUCTION_Z+5, sc*1.1, sc*1.1, "%d", localInventory[i]->amtStored);
	//			cprnt(itembox.left + 49*sc, itembox.top + 55*sc, AUCTION_Z+5, sc*1.1, sc*1.1, "BoughtItem");
	//		}
	//	}

	//	if ( mouseDownHit(&itembox, MS_LEFT) )
	//	{
	//		currentInvItem = i;
	//		if(displayingExtraInfo)
	//			auction_updateHistory(localInventory[i]->type, localInventory[i]->name, localInventory[i]->lvl);
	//		if ( localInventory[i]->auction_status == AuctionInvItemStatus_Stored )
	//			uiEditTakeFocus(sellItemEdit);
	//	}
	//	else if ( mouseClickHit( &itembox, MS_RIGHT ) )
	//	{
	//		AuctionDoContextMenu( localInventory[i] );
	//	}

	//	if ( mouseLeftDrag(&itembox) )
	//	{
	//		currentInvItem = i;
	//		if(displayingExtraInfo)
	//			auction_updateHistory(localInventory[i]->type, localInventory[i]->name, localInventory[i]->lvl);

	//		draggingFromInv = 1;
	//		if ( !auctionInvTray )
	//			auctionInvTray = (TrayObj*)calloc(1, sizeof(TrayObj));
	//		auctionInvTray->scale = 1.0f;
	//		auctionInvTray->type = localInventory[i]->type;
	//		switch ( auctionInvTray->type )
	//		{
	//			case kTrayItemType_Salvage:
	//				auctionInvTray->icon = atlasLoadTexture(localInventory[i]->salvage->ui.pchIcon);
	//			xcase kTrayItemType_Recipe:
	//				auctionInvTray->icon = atlasLoadTexture(localInventory[i]->recipe->ui.pchIcon);
	//			xcase kTrayItemType_Inspiration:
	//				auctionInvTray->icon = atlasLoadTexture(localInventory[i]->inspiration->pchIconName);
	//			xcase kTrayItemType_SpecializationInventory:
	//				auctionInvTray->icon = atlasLoadTexture(localInventory[i]->enhancement->pchIconName);
	//				auctionInvTray->icon = atlasLoadTexture(localInventory[i]->enhancement->pchIconName);
	//				break;
	//		}
	//		auctionInvTray->invIdx = i;
	//		trayobj_startDragging(auctionInvTray, auctionInvTray->icon, icon);
	//		cursor.drag_obj.ispec = -1;
	//		cursor.drag_obj.itray = -1;
	//	}
	//}

	//// Draw empty inventory slots
	//for ( i = eaSize(&localInventory); i < invSize; ++i )
	//{
	//	static SMFBlock *smf;
	//	static char *emptyInvStr = NULL;
	//	int yPos = y + ht - (INVENTORY_H + 11)*sc;//- 25*sc;//- (DISPLAYITEM_H*sc/2 + 12*sc);
	//	int color = CLR_FRAME_GREY;
	//	CBox itembox = { x + (INVENTORY_X+i*INVENTORY_W)*sc - sb.offset,
	//		yPos,// + INVENTORYITEM_Y,
	//		x + (INVENTORY_X + i*INVENTORY_W + INVENTORY_W)*sc - sb.offset,
	//		yPos + (/*INVENTORYITEM_Y +*/ INVENTORY_H)*sc  };

	//	if (!smf)
	//	{
	//		smf = smfBlock_Create();
	//	}

	//	//if ( itembox.top + INVENTORY_H*sc < framebox.top || itembox.bottom - INVENTORY_H*sc > framebox.bottom)
	//	//	continue;

	//	font(&game_14);
	//	font_color( 0xffffffff, 0xcccc00ff );

	//	estrClear(&emptyInvStr);
	//	estrConcatf(&emptyInvStr, "<span align=center>%s</span>", textStd("AuctionEmptyInventory"));
	//	gAuctionTextAttr.piColor = (int*)0x707070ff;
	//	gAuctionTextAttr.piScale = (int*)((int)(1.0f * SMF_FONT_SCALE * sc));
	//	drawFlatFrameBox(PIX3, R10, &itembox, AUCTION_Z, color, color);
	//	smf_ParseAndDisplay(smf, emptyInvStr, itembox.left+10*sc, itembox.top+15*sc, AUCTION_Z,
	//		itembox.right-itembox.left-20*sc, itembox.bottom-itembox.top-20*sc, 1, 1, 
	//		&gAuctionTextAttr, NULL, 0, true);
	//}

	//// This is deferred to the end since it may remove and item from localInventory.
	//if(currentInvItemGet == kInfStored)
	//{
	//	auctionHouseGetInfStored(currentInvItem);
	//}
	//else if(currentInvItemGet == kAmtStored)
	//{
	//	auctionHouseGetAmtStored(currentInvItem);
	//}

	//if ( cursor.dragging && !draggingFromInv )
	//{
	//	static AuctionItem addInfo;

	//	if( !isDown( MS_LEFT ) && mouseCollision( &framebox ) )
	//	{
	//		switch(cursor.drag_obj.type)
	//		{
	//		case kTrayItemType_Recipe:
	//			{
	//				DetailRecipe *recipe = detailrecipedict_RecipeFromId(cursor.drag_obj.invIdx);
	//				if ( recipe )
	//				{
	//					if (recipe->flags & RECIPE_NO_TRADE)
	//					{
	//						addSystemChatMsg( textStd("CannotTradeRecipe"), INFO_USER_ERROR, 0 );
	//					}
	//					else
	//					{
	//						int stackSize = MIN(MAX_AUCTION_STACK_SIZE,character_RecipeCount(e->pchar, recipe));
	//						addInfo.type = kTrayItemType_Recipe;
	//						addInfo.name = recipe->pchName;
	//						addInfo.lvl = recipe->level;
	//						addInfo.auction_id = cursor.drag_obj.invIdx;
	//						addInfo.id = auctionData_GetID(kTrayItemType_Recipe, recipe->pchName, recipe->level);
	//						addInfo.infPrice = 0;
	//						addInfo.auction_status = AuctionInvItemStatus_Stored;
	//						addInfo.recipe = recipe;
	//						amountSliderWindowSetupAndDisplay(1, stackSize, NULL, amountSliderDrawCallback, auctionAmountSliderCallback, &addInfo, 0);
	//					}
	//				}
	//			}
	//			break;
	//		case kTrayItemType_Salvage:
	//			if ( e->pchar->salvageInv[cursor.drag_obj.invIdx] )
	//			{
	//				const SalvageItem *salvage = e->pchar->salvageInv[cursor.drag_obj.invIdx]->salvage;
	//				if ( salvage )
	//				{
	//					if (salvage->flags & SALVAGE_NOTRADE)
	//					{
	//						addSystemChatMsg( textStd("CannotTradeSalvage"), INFO_USER_ERROR, 0 );
	//					} else {
	//						int stackSize = MIN(MAX_AUCTION_STACK_SIZE,character_SalvageCount(e->pchar, salvage));
	//						addInfo.type = kTrayItemType_Salvage;
	//						addInfo.name = salvage->pchName;
	//						addInfo.lvl = 0;
	//						addInfo.auction_id = cursor.drag_obj.invIdx;
	//						addInfo.id = auctionData_GetID(kTrayItemType_Salvage, salvage->pchName, 0);
	//						addInfo.infPrice = 0;
	//						addInfo.auction_status = AuctionInvItemStatus_Stored;
	//						amountSliderWindowSetupAndDisplay(1, stackSize, NULL, amountSliderDrawCallback, auctionAmountSliderCallback, &addInfo, 0);
	//					}
	//				}
	//			}
	//			break;
	//		case kTrayItemType_SpecializationInventory:
	//			if ( e->pchar->aBoosts[cursor.drag_obj.ispec] )
	//			{
	//				BasePower *ppow = e->pchar->aBoosts[cursor.drag_obj.ispec]->ppowBase;
	//				char *name = basepower_ToPath(ppow);
	//				int level = e->pchar->aBoosts[cursor.drag_obj.ispec]->iLevel;
	//				DetailRecipe *recipe = detailrecipedict_RecipeFromEnhancementAndLevel(name, level+1);
	//				if (	(ppow && !ppow->bBoostTradeable) ||
	//						(recipe && (recipe->flags & RECIPE_NO_TRADE)) ){
	//					addSystemChatMsg( textStd("CannotTradeEnhancement"), INFO_USER_ERROR, 0 );
	//				} else if (e->pchar->aBoosts[cursor.drag_obj.ispec]->iNumCombines > 0) {
	//					addSystemChatMsg( textStd("CantAuctionCombined"), INFO_USER_ERROR, 0 );
	//				} else {
	//					auctionHouseAddItem(kTrayItemType_SpecializationInventory, name,
	//						level, cursor.drag_obj.ispec, 1, 0,
	//						AuctionInvItemStatus_Stored,
	//						auctionData_GetID(kTrayItemType_SpecializationInventory, name, level));
	//				}
	//			}
	//			break;
	//		case kTrayItemType_Inspiration:
	//			if ( e->pchar->aInspirations[cursor.drag_obj.iset][cursor.drag_obj.ipow] )
	//			{
	//				char *name = basepower_ToPath(e->pchar->aInspirations[cursor.drag_obj.iset][cursor.drag_obj.ipow]);
	//				auctionHouseAddItem(kTrayItemType_Inspiration, name,
	//					0, (cursor.drag_obj.iset << 16 | (cursor.drag_obj.ipow & 0x0000FFFF)), 1, 0,
	//					AuctionInvItemStatus_Stored,
	//					auctionData_GetID(kTrayItemType_Inspiration, name, 0));
	//			}
	//			break;
	//		}

	//		trayobj_stopDragging();
	//	}
	//}
	//else if ( draggingFromInv )
	//{
	//	CBox auctionWindowBox = { x, y, x + wd, y + ht };
	//	// they are dragging an object out of the auction window
	//	if ( cursor.dragging && !isDown(MS_LEFT) && !mouseCollision(&auctionWindowBox) && localInventory[cursor.drag_obj.invIdx])
	//	{
	//		//auction_placeItemInInventory(localInventory[cursor.drag_obj.invIdx]->auction_id);
	//		auctionHouseRemoveItem(cursor.drag_obj.invIdx);
	//		eaRemove(&e->pchar->auctionInv->items, auctionInvTray->invIdx);
	//		trayobj_stopDragging();
	//		draggingFromInv = 0;
	//	}
	//	else if ( !isDown(MS_LEFT) && !mouseCollision(&auctionWindowBox) )
	//		draggingFromInv = 0;
	//}

	//font(&game_14);
	//if( eaSize(&localInventory)== invSize )
	//	font_color(CLR_RED, CLR_RED);
	//else
	//	font_color(CLR_PARAGON, CLR_PARAGON);

	//cprntEx( x+wd/2, y + ht - INVENTORY_H*sc - 20*sc, z, sc, sc, CENTER_X|CENTER_Y, "%i/%i", eaSize(&localInventory), invSize );

	//if(drawStdButtonTopLeft( x + (DETAILITEM_MOREBUTTON_X - DETAILITEM_MOREBUTTON_W)*sc, y + (dividerOffset+DETAILITEM_MOREBUTTON_Y)*sc, AUCTION_Z, DETAILITEM_MOREBUTTON_W*sc, 25*sc, CLR_BLUE, "GetAllInf", sc, !unclaimed_inf) == D_MOUSEHIT )
	//{
	//	for (i = eaSize(&localInventory)-1; i>=0; i-- )
	//	{
	//		if(localInventory[i]->auction_status == AuctionInvItemStatus_Sold )
	//			auctionHouseGetInfStored(i);
	//	}
	//}

	//if ( drawStdButtonTopLeft( x + (DETAILITEM_MOREBUTTON_X)*sc, y + (dividerOffset+DETAILITEM_MOREBUTTON_Y)*sc, AUCTION_Z, DETAILITEM_MOREBUTTON_W*sc, 25*sc, CLR_BLUE,
	//	displayingExtraInfo ?"AuctionLess":"AuctionMore", sc, 0) == D_MOUSEHIT )
	//{
	//	if ( dividerOffset == DEFAULT_DIVIDER_OFFSET_EXPANDED )
	//	{
	//		displayingExtraInfo = 1;
	//		dividerOffset = DEFAULT_DIVIDER_OFFSET_CONTRACTED;
	//		if ( currentInvItem != -1 )
	//			auction_updateHistory(localInventory[currentInvItem]->type, localInventory[currentInvItem]->name, localInventory[currentInvItem]->lvl);
	//	}
	//	else
	//	{
	//		displayingExtraInfo = 0;
	//		dividerOffset = DEFAULT_DIVIDER_OFFSET_EXPANDED;
	//	}
	//}

	//font_color( CLR_WHITE, CLR_WHITE );
	//prnt(x + (610)*sc, y + (INVENTORYBUTTONS_Y+20)*sc, AUCTION_Z, sc, sc, "%s", textStd("InfColonValue", e->pchar->iInfluencePoints));

	//sb.horizontal = true;
	//doScrollBar(&sb, wd-(R12+10)*sc, (invSize * INVENTORY_W)*sc, 10, (ht), AUCTION_Z, &framebox, 0);
}

void createAuctionItemSMF(AuctionItem *item, float sc)
{
	//uiEnhancement *pEnh = NULL;

	//if ( item->smfStr )
	//	estrClear(&item->smfStr);

	//if ( item->type == kTrayItemType_Recipe && item->recipe->pchEnhancementReward != NULL )
	//{
	//	pEnh = uiEnhancementCreateFromRecipe(item->recipe, item->recipe->level, false);
	//}
	//else if ( item->type == kTrayItemType_SpecializationInventory )
	//{
	//	pEnh = uiEnhancementCreate(item->enhancement->pchFullName, item->lvl);
	//}

	////estrConcatf(&item->smfStr, "<span align=center>");
	////estrConcatf(&item->smfStr, "<b>%s<b><br>", textStd(item->displayName));
	////estrConcatf(&item->smfStr, "</span>");

	//estrConcatf(&item->smfStr, "<span align=left>");
	//switch ( item->type )
	//{
	//	case kTrayItemType_Salvage:
	//		estrConcatf(&item->smfStr, "<p>");
	//		estrConcatf(&item->smfStr, "<img src=\"%s\" align=left border=5>", item->salvage->ui.pchIcon);
	//		estrConcatf(&item->smfStr, "<color white>%s</color><br><color InfoBlue>%s</color><br><color paragon>%s</color></p>", textStd(item->salvage->ui.pchDisplayName), textStd(item->salvage->ui.pchDisplayShortHelp), textStd(item->salvage->pchDisplayTabName));
	//		estrConcatf(&item->smfStr, "<p>");
	//		estrConcatf(&item->smfStr, "<color white>%s</color></p>", textStd(item->salvage->ui.pchDisplayHelp));
	//	xcase kTrayItemType_Recipe:
	//		estrConcatf(&item->smfStr, "<p>");
 //			estrConcatf(&item->smfStr, "<img src=\"rec:%s\" align=left border=15>", item->recipe->pchName);
	//		//estrConcatf(&item->smfStr, "<b><color white>%s %d</color></b><br><br>", textStd("PowerLevel"), item->lvl);
	//		if ( item->recipe->pchEnhancementReward )
	//			estrConcatf(&item->smfStr, "<color white>%s</color></p>", uiEnhancementGetToolTipText(pEnh));//textStd(item->recipe->ui.pchDisplayHelp));
	//		else
	//			estrConcatf(&item->smfStr, "<color white>%s</color></p>", textStd(item->recipe->ui.pchDisplayHelp));
	//	xcase kTrayItemType_Inspiration:
	//		estrConcatf(&item->smfStr, "<p>");
	//		estrConcatf(&item->smfStr, "<img src=\"%s\" align=left border=10>", item->inspiration->pchIconName);
	//		estrConcatf(&item->smfStr, "<color white>%s</color><br><color InfoBlue>%s<color><br><color paragon>%s</color></p>", textStd(item->inspiration->pchDisplayName), textStd(item->inspiration->pchDisplayShortHelp), textStd(g_PowerDictionary.ppPowerCategories[item->inspiration->id.icat]->ppPowerSets[item->inspiration->id.iset]->pchDisplayShortHelp));
	//		estrConcatf(&item->smfStr, "<p>");
	//		estrConcatf(&item->smfStr, "%s</p>", textStd(item->inspiration->pchDisplayHelp));
	//	xcase kTrayItemType_SpecializationInventory:
	//		estrConcatf(&item->smfStr, "<p>");
	//		estrConcatf(&item->smfStr, "<img src=\"pow:%s.%d\" align=left border=25>", item->enhancement->pchFullName, item->lvl);
	//		//estrConcatf(&item->smfStr, "<b><color white>%s %d</color></b><br><br>", textStd("PowerLevel"), item->lvl+1);
	//		estrConcatf(&item->smfStr, "<color white>%s</color></p>", uiEnhancementGetToolTipText(pEnh));//textStd(item->inspiration->pchDisplayHelp));
	//		break;
	//}
	//estrConcatf(&item->smfStr, "</span>");

	//////create SMF block
	////if ( !item->smf )
	////	item->smf = smfBlock_Create();
	////smf_ParseAndFormat(&item->smf, item->smfStr, 0, 0, 0, DETAILITEM_SMFBLOCK_W*sc, DETAILITEM_SMFBLOCK_H*sc, false, false, &gAuctionTextAttr);

	//if ( pEnh )
	//	uiEnhancementFree(&pEnh);
}

void displayItemDetail(Entity *e, float x, float y, float z, float wd, float ht, float sc, int clr, int bkclr)
{
	//AuctionItem *item = NULL;
	////CBox sellbox = {x + DETAILITEM_SELLBOX_X*sc, y + DETAILITEM_SELLBOX_Y*sc,
	////	x + (DETAILITEM_SELLBOX_X + DETAILITEM_SELLBOX_W)*sc, y + (DETAILITEM_SELLBOX_Y + DETAILITEM_SELLBOX_H)*sc};
	////UIBox sellboxbounds;

	//if ( !eaSize(&localInventory) || currentInvItem < 0 || currentInvItem >= eaSize(&localInventory) )
	//{
	//	currentInvItem = -1;
	//	return;
	//}

	//item = localInventory[currentInvItem];

	//if ( !item )
	//	return;

	//curInventoryMenuItemName = item->name;

	//if ( !sellItemEdit )
	//{
	//	INIT_EDIT_BOX(sellItemEdit, 13);
	//	sellItemEdit->rightAlign = true;
	//	sellItemEdit->noClip = true;
	//	sellItemEdit->inputHandler = uiEditCommaNumericInputHandler;
	//}

	//createAuctionItemSMF(item, sc);

	//gAuctionTextAttr.piColor = (int*)0x48ff48ff;
	//gAuctionTextAttr.piFont = (int *)&game_12;
	//gAuctionTextAttr.piScale = (int *)(int)(1.0f*SMF_FONT_SCALE);
	////drawAuctionItemIcon(item, x + DETAILITEM_ICON_X*sc, y + DETAILITEM_ICON_Y*sc, AUCTION_Z, sc, sc);
	//{
	//	static ScrollBar sb = { -1, 0 };
	//	CBox infoFrameBox;
	//	UIBox infoFrameClipBox;
	//	int smf_ht=0;
	//	infoFrameBox.left = x + DETAILITEM_SMFBLOCK_X*sc;
	//	infoFrameBox.top = y + DISPLAYITEM_INFO_Y*sc;
	//	infoFrameBox.right = infoFrameBox.left + DETAILITEM_SMFBLOCK_W*sc;
	//	infoFrameBox.bottom = infoFrameBox.top + DETAILITEM_SMFBLOCK_H*sc;
	//	drawFrameBox(PIX3, R10, &infoFrameBox, AUCTION_Z, 0x00000000, 0x000000FF);
	//	uiBoxFromCBox(&infoFrameClipBox, &infoFrameBox);
	//	infoFrameClipBox.x += 5*sc;
	//	infoFrameClipBox.y += 5*sc;
	//	infoFrameClipBox.width -= 10*sc;
	//	infoFrameClipBox.height -= 10*sc;
	//	clipperPushRestrict(&infoFrameClipBox);
	//	smf_ht = smf_ParseAndDisplay(item->smf, item->smfStr, infoFrameClipBox.x, infoFrameClipBox.y-sb.offset*sc, AUCTION_Z, 
	//									infoFrameClipBox.width, infoFrameClipBox.height, false, false, 
	//									&gAuctionTextAttr, NULL, 0, true);
	//	clipperPop();
	//	sb.use_color = 1;
	//	sb.color = clr;
	//	doScrollBar(&sb, infoFrameClipBox.height, smf_ht/sc, infoFrameBox.right, infoFrameClipBox.y, AUCTION_Z, &infoFrameBox, 0);
	//}

	//font(&game_9);
	//font_color( 0xffffffff, 0xffffffff );
	//if (inventoryMenuHistory)
	//{
	//	int i;
	//	if ( stashFindInt(g_AuctionItemDict.itemToIDHash, inventoryMenuHistory->pchIdentifier, &i) )
	//	{
	//		
	//	//	if (item->numForSale != -1 )
	//		prnt(x + (405)*sc, y + (DISPLAYITEM_INFO_Y-6)*sc, AUCTION_Z, sc, sc, "%d %s", getNumSellingById(i), textStd("ForSale") );
	//	//	if (item->numForBuy != -1 )
	//		prnt(x + (505)*sc, y + (DISPLAYITEM_INFO_Y-6)*sc, AUCTION_Z, sc, sc, "%d %s", getNumBuyingById(i), textStd("BiddingString") );
	//	}
	//}

	//// this item needs a "put item up for sale" button/editbox
	////if ( item->status == AuctionInvItemStatus_Stored )
	//{
	//	int color = CLR_BLUE, colorBack = CLR_BLUE;

	//	// draw the item history
	//	{
	//		static ScrollBar sb = { -1, 0 };
	//		UIBox historyBox = {x + DETAILITEM_HISTORY_X*sc, y + DETAILITEM_HISTORY_Y*sc, DETAILITEM_HISTORY_W*sc, DETAILITEM_HISTORY_H*sc };
	//		int i = 0;
	//		int ht;
	//		TrayItemIdentifier *tii = inventoryMenuHistory ? TrayItemIdentifier_Struct(inventoryMenuHistory->pchIdentifier) : NULL;

	//		drawFrame(PIX3, R10, historyBox.x, historyBox.y-5*sc, AUCTION_Z, historyBox.width, historyBox.height+20*sc, 1.0f, 0x99, 0x99);

	//		if ( INRANGE0(item->id,eaSize(&g_AuctionItemDict.idToItemMap)) &&
	//			g_AuctionItemDict.items[item->id]->buyPrice > 0 )
	//		{
	//			cprnt( historyBox.x+5*sc, y + (STD_EDIT_H + DETAILITEM_HISTORY_Y)*sc, AUCTION_Z, sc, sc, "%s", textStd("ItemPriceFixed") );
	//		}
	//		else
	//		{
	//			clipperPushRestrict(&historyBox);

	//			if ( tii && stricmp(tii->name, item->name) == 0 )
	//			{
	//				font(&game_12);
	//				font_color( 0xffffffff, 0xffffffff );
	//				if ( eaSize(&inventoryMenuHistory->histories) <= 0 )
	//					prnt( historyBox.x+5*sc, y + (STD_EDIT_H + DETAILITEM_HISTORY_Y)*sc, AUCTION_Z, sc, sc, "%s", textStd("NoHistory") );
	//				else for ( i = 0; i < eaSize(&inventoryMenuHistory->histories); ++i )
	//				{
	//					char buffer[80];
	//					timerMakeDateStringFromSecondsSince2000(buffer,inventoryMenuHistory->histories[i]->date);
	//					//prnt( historyBox.x, y + (DETAILITEM_HISTORY_Y + 1 + i * STD_EDIT_H)*sc, AUCTION_Z, sc, sc,
	//					prnt( historyBox.x+5*sc, historyBox.y + (STD_EDIT_H + i * STD_EDIT_H)*sc - sb.offset, AUCTION_Z, sc, sc,
	//						"%s", textStd("HistoryItemString", buffer, inventoryMenuHistory->histories[i]->price) );
	//				}
	//			}
	//			else
	//				prnt( historyBox.x+5*sc, y + (STD_EDIT_H + DETAILITEM_HISTORY_Y)*sc, AUCTION_Z, sc, sc, "%s", textStd("WaitingForHistory") );

	//			clipperPop();

	//			ht = (i * STD_EDIT_H)*sc;
	//			doScrollBar(&sb, DETAILITEM_HISTORY_H*sc, ht, x + (DETAILITEM_HISTORY_X+DETAILITEM_HISTORY_W)*sc, y + (5+DETAILITEM_HISTORY_Y)*sc, AUCTION_Z, 0, &historyBox);
	//		}
	//	}
	//}
}

void auctionHouseInventoryMenu(Entity *e, float x, float y, float z, float wd, float ht, float sc, int clr, int bkclr)
{
	displayInventory(e, x, y, z, wd, ht, sc, clr, bkclr);
	if ( displayingExtraInfo )
		displayItemDetail(e, x, y, z, wd, ht, sc, clr, bkclr);
}
