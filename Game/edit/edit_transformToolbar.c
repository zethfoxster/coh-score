#include "edit_transformToolbar.h"

#include "edit_cmd.h"
#include "edit_select.h"
#include "gfxtree.h"
#include "input.h"
#include "smf_main.h"
#include "sprite_base.h"
#include "sprite_font.h"
#include "sprite_text.h"
#include "textureatlas.h"
#include "ttFontUtil.h"
#include "uiGame.h"
#include "uiInput.h"
#include "uiScrollBar.h"
#include "uiUtil.h"
#include "win_init.h"

#define FIELD_WIDTH 100
#define FIELD_HEIGHT 20
#define SMF_MARGIN 4
#define CAPTION_WIDTH 40
#define HEADER_WIDTH 60

#define SMF_WIDTH ((F32)FIELD_WIDTH - CAPTION_WIDTH - SMF_MARGIN)
#define SMF_HEIGHT ((F32)FIELD_HEIGHT - 2 * SMF_MARGIN)

static bool s_isTransformAbsolute = false;

typedef struct {
	char* label;
	SMFBlock* smfBlock;
} TransformField;

static TransformField moveFields[] = {
	{
		"X:",
	},
	{
		"Y:",
	},
	{
		"Z:",
	},
};

static TransformField rotateFields[] = {
	{
		"Pitch:",
	},
	{
		"Yaw:",
	},
	{
		"Roll:",
	},
};

static bool drawSMFBlock(SMFBlock* block, int smfX, int smfY, bool changedSinceLastFrame,
						 int captionColor, F32* value, int* lost_focus) 
{
	// By design, SMF blocks take keyboard focus when mouse-dragged over, even if the drag
	//  started outside the block. This is a Bad Thing for the editor. To combat this, we
	//  track which SMF block, if any, was the starting point for a mouse drag. If it's not
	//  the current one, we spoof the SMF dragging logic by fooling it into thinking that
	//  a scrollbar is being dragged (which evidently presented a similar problem).
	static SMFBlock* dragOrigination;
	int scrollBarDraggingSave = gScrollBarDraggingLastFrame;

	int fieldColor = 0x000000ff;
	F32 originalValue = *value;

	smf_SetScissorsBox(block, smfX, smfY, SMF_WIDTH, SMF_HEIGHT);

	if (!isDown(MS_LEFT))
		dragOrigination = NULL;
	else if (mouseDownHit(block->scissorsBox, MS_LEFT))
		dragOrigination = block;

	if (sel.parent && mouseCollision(block->scissorsBox))
		fieldColor = 0x333333ff;

	display_sprite(white_tex_atlas, smfX - 1, smfY - 1, 510, 
		(SMF_WIDTH + 2) / white_tex_atlas->width, 
		(SMF_HEIGHT + 2) / white_tex_atlas->height, 
		captionColor);
	display_sprite(white_tex_atlas, smfX, smfY, 510, SMF_WIDTH / white_tex_atlas->width, 
		SMF_HEIGHT / white_tex_atlas->height, fieldColor);
	block->editMode = sel.parent ? SMFEditMode_ReadWrite : SMFEditMode_Unselectable;

	if (!sel.parent)
		smf_SetRawText(block, "", false);
	else if (changedSinceLastFrame) {
		char buf[16];

		// Without this check, PYR often displays "-0.00"
		if (fabs(*value) < 0.00001)
			*value = 0.0;

		sprintf(buf, "%1.2f", *value);
		smf_SetRawText(block, buf, false);
	}

	if (block != dragOrigination && !smfBlock_HasFocus(block) && mouseCollision(block->scissorsBox))
		gScrollBarDraggingLastFrame = 1;

	smf_Display(block, smfX, smfY, 520, SMF_WIDTH, SMF_HEIGHT, 0, 0,
		&gTextAttr_White9, 0);

	gScrollBarDraggingLastFrame = scrollBarDraggingSave;

	if (changedSinceLastFrame && smfBlock_HasFocus(block))
		smf_SelectAllText(block);

	if (sel.parent && block->clickedOnThisFrame)
		smf_SelectAllText(block);

	if (!sel.parent) {
		display_sprite(white_tex_atlas, smfX - 1, smfY - 1, 510, 
			(SMF_WIDTH + 2) / white_tex_atlas->width,
			(SMF_HEIGHT + 2) / white_tex_atlas->height, 0x7f7f7fd0);
	}

	if (sscanf(block->outputString, "%f", value) != 1)
		*value = originalValue;

	if (smfBlock_HasFocus(block)) {
		KeyInput* input = inpGetKeyBuf();
		
		while (input) {
			if (input->type == KIT_EditKey && 
					(input->key == INP_RETURN || input->key == INP_NUMPADENTER)) {
				return true;
			}

			inpGetNextKey(&input);
		}
	}

	return false;
}

static bool drawTransformRow(char* header, TransformField* fields, F32 y, 
							 bool changedSinceLastFrame, F32* offset, int* lost_focus) {
	int coordinate;
	bool result = false;
	F32 x;
	F32 textScale;
	int smfY = y + SMF_MARGIN;
	int screenWidth, screenHeight;
	windowClientSize(&screenWidth, &screenHeight);

	x = screenWidth - 250 - 3 * FIELD_WIDTH - HEADER_WIDTH;
	textScale = MIN(screenWidth / 1024.f, screenHeight / 768.f);
	font_color(0xffffffff, 0xffffffff);
	cprntEx(x + 5, y + FIELD_HEIGHT / 2, 510, textScale, textScale, CENTER_Y, header);

	for (coordinate = 0; coordinate < 3; ++coordinate) {
		TransformField* field = &fields[coordinate];
		F32 x = screenWidth - 250 - (3 - coordinate) * FIELD_WIDTH;
		F32 z = 500.0f;
		int smfX = x + CAPTION_WIDTH;
		int captionColor = 0x999999ff;

		if (!field->smfBlock) {
			field->smfBlock = smfBlock_Create();
			smf_SetFlags(field->smfBlock, SMFEditMode_ReadWrite, SMFLineBreakMode_SingleLine, 
				SMFInputMode_AnyTextNoTagsOrCodes, 14, SMFScrollMode_ExternalOnly,
				SMFOutputMode_StripAllTagsAndCodes, SMFDisplayMode_AllCharacters, SMFContextMenuMode_None,
				SMAlignment_Left, 0, field->label, -1);
		}

		if (smfBlock_HasFocus(field->smfBlock))
			captionColor = 0x00ff00ff;
		font_color(captionColor, captionColor);
		cprntEx(x + 5, y + FIELD_HEIGHT / 2, 510, textScale, textScale, CENTER_Y, field->label);
		result |= drawSMFBlock(field->smfBlock, smfX, smfY, changedSinceLastFrame, captionColor,
					   offset + coordinate, lost_focus);
	}

	return result;
}

static bool drawTransformModeButton(int barX, int barY) {
	AtlasTex* buttonTex = s_isTransformAbsolute ?
		atlasLoadTexture("TransformTool_Button_Mode_Absolute") :
		atlasLoadTexture("TransformTool_Button_Mode_Offset");
	int buttonX = barX + FIELD_HEIGHT - buttonTex->width / 2;
	int buttonY = barY + FIELD_HEIGHT - buttonTex->height / 2;
	CBox buttonBox;
	int buttonTint = 0x999999ff;
	bool result = false;

	BuildCBox(&buttonBox, buttonX, buttonY, buttonTex->width, buttonTex->height);

	if (mouseCollision(&buttonBox)) {
		buttonTint = 0xffffffff;
		
		if (mouseDown(MS_LEFT)) {
			s_isTransformAbsolute = !s_isTransformAbsolute;
			result = true;
		}
	}
			
	display_sprite(buttonTex, buttonX, buttonY, 510, 1, 1, buttonTint);

	return result;
}

void editDrawTransformToolbar(int* lost_focus) {
	static GfxNode* lastSelection;
	static Vec3 lastPYR;
	static Vec3 lastPosition;

	F32 textScale;
	Vec3 currentPYR;
	Vec3 currentPosition;
	Vec3 userXYZ = {0.0f, 0.0f, 0.0f};
	Vec3 userPYR = {0.0f, 0.0f, 0.0f};
	int screenWidth, screenHeight;
	F32 y;
	bool changedSinceLastFrame;
	bool changedHere = false;
	int i;
	int barX, barY, barWidth, barHeight;
	CBox barBox;
	bool transformModeChanged;

	windowClientSize(&screenWidth, &screenHeight);

	barWidth = 3 * FIELD_WIDTH + HEADER_WIDTH + 2 * FIELD_HEIGHT;
	barHeight = 2 * FIELD_HEIGHT;
	barX = screenWidth - 250 - barWidth;
	barY = screenHeight - 2 * FIELD_HEIGHT;
	BuildCBox(&barBox, barX, barY, barWidth, barHeight);

	if (mouseCollision(&barBox)) {
		edit_state.sel = 0;
		*lost_focus = 1;
	}

	display_sprite(white_tex_atlas, barX, barY, 500, (F32)barWidth / white_tex_atlas->width, 
		(F32)barHeight / white_tex_atlas->height, 0x00000088);
	textScale = MIN(screenWidth / 1024.f, screenHeight / 768.f);
	y = screenHeight - FIELD_HEIGHT * 2;
	font(&game_9);

	if (sel.parent) {
		copyVec3(sel.parent->mat[3], currentPosition);
		getMat3PYR(sel.parent->mat, currentPYR);
	}

	transformModeChanged = drawTransformModeButton(barX, barY);

	if (s_isTransformAbsolute) {
		copyVec3(currentPosition, userXYZ);
		copyVec3(currentPYR, userPYR);

		for (i = 0; i < 3; ++i)
			userPYR[i] = DEG(userPYR[i]);
	}

	changedSinceLastFrame = transformModeChanged ||
		sel.parent != lastSelection ||
		!sameVec3(lastPosition, currentPosition) ||
		!sameVec3(lastPYR, currentPYR);
	changedHere = drawTransformRow(s_isTransformAbsolute ? "Position" : "Move", moveFields,
		y, changedSinceLastFrame, userXYZ, lost_focus);
	y += FIELD_HEIGHT;
	changedHere |= drawTransformRow(s_isTransformAbsolute ? "Rotation" : "Rotate",
		rotateFields, y, changedSinceLastFrame, userPYR, lost_focus);

	for (i = 0; i < 3; ++i)
		userPYR[i] = RAD(userPYR[i]);

	lastSelection = sel.parent;
	copyVec3(currentPosition, lastPosition);
	copyVec3(currentPYR, lastPYR);

	if (changedHere) {
		extern void edit_relock(void);

		edit_relock();
		
		if (s_isTransformAbsolute) {
			createMat3PYR(sel.parent->mat, userPYR);
			copyVec3(userXYZ, sel.parent->mat[3]);
		} else {
			copyVec3(sel.parent->mat[3], currentPosition);
			getMat3PYR(sel.parent->mat, currentPYR);
			addVec3(currentPYR, userPYR, currentPYR);
			createMat3PYR(sel.parent->mat, currentPYR);
			addVec3(sel.parent->mat[3], userXYZ, sel.parent->mat[3]);
		}
	}
}
