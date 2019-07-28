/***************************************************************************
 *     Copyright (c) 2005-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#include "DoorStrings.h"
#include "utils.h"
#include "assert.h"
#include "error.h"
#include "mathutil.h"
#include "earray.h"
#include "MemoryPool.h"
#include "StashTable.h"

const char* mapSelectFormatTop1 = 
"<font face=computer outline=0 color=#32ff5d><b>"
"<span align=center>"
"<scale 1.8>"
;

const char* mapSelectFormatTop2 = "PTATitle";

const char* mapSelectFormatTop3 = 
"</scale><br><br>"
"</span>"
"<table><tr>"
"<td>"
"<font face=computer outline=0 color=#22ff4d><b>"
"<linkbg #88888844><linkhoverbg #578836><linkhover #a7ff6c><link #52ff7d><b>"
"<table>"
;

const char* mapSelectFormatBottom = 
"</table>"
"</linkhover>"
"</linkhoverbg>"
"</linkbg>"
"</font>"
"</td>"
"</tr></table>"
"</b></font>"
;
// These strings need to be skinned
const char* teleFormatTop2 = "TeleporterTitle";
const char* teleEmptyFormatTop3 = 
"</scale><br><br>"
"</span>"
"<table><tr>"
"<td>"
"<font face=computer outline=0 color=#22ff4d><b>"
"<linkbg #88888844><linkhoverbg #578836><linkhover #a7ff6c><link #52ff7d><b>";

const char* teleEmptyFormatTop4 = 
"<p><table>";

const char* plainFormatTop2 = "MapSelectTitle";

const char *monorailSelectFormatTitleHeader =
"<font face=heading outline=1 color=#ffffff>"
"<span align=center>"
"<scale 1.6>";

const char *monorailSelectFormatTitleBodyDivider =
"</scale><br></span>"
"<table><tr>"
"<td>"
"<linkbg #ffffff33><linkhoverbg #ffffff66><linkhover #ffffff><link #9d9d9d>"
"<font face=small outline=1 color=#9d9d9d>"
"<table>";

const char *monorailSelectFormatBodyFooter =
"</table>"
"</font>"
"</linkhover>"
"</linkhoverbg>"
"</linkbg>"
"</td></tr></table>";
