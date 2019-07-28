#ifndef _RT_CURSOR_H
#define _RT_CURSOR_H

#include "rt_pbuffer.h"
#include "rt_sprite.h"

extern PBuffer	cursor_pbuf;

#ifdef RT_PRIVATE
void hwcursorClearDirect();
void hwcursorBlitDirect( RdrSpritesCmd *cmd);
#endif

#endif
