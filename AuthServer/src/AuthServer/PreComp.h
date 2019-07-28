#pragma once

#define LINEAGE2_GAME_CODE 8
#include "WantedSocket.h"
#include "util.h"
#include "config.h"
#include "Account.h"
#include "dbconn.h"
#include "AccountDB.h"
#include "thread.h"
#include "IOServer.h"
#include "IPSessiondb.h"
#include "ServerList.h"
#include "LogSocket.h"

extern BOOL SendSocket(in_addr , const char *format, ...);