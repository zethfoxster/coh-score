#define dbContainerUnpack FAKE_dbContainerUnpack
#define dbLog FAKE_dbLog
#define dbSyncContainerUpdate FAKE_dbSyncContainerUpdate
#define dbContainerPackage FAKE_dbContainerPackage
#define sendInfoBox FAKE_sendInfoBox
#define serverParseClientEx FAKE_serverParseClientEx
#define dbSecondsSince2000 FAKE_dbSecondsSince2000

#include "autoCommands.c"

#include "UnitTest++.h"

static void conPrintf(ClientLink* client, const char *s, ...) {}
static void dbContainerUnpack(StructDesc *descs,char *buff,void *mem) {}
static void dbLog(char const *cmdname,Entity *e,char const *fmt, ...) {}
static int dbSyncContainerUpdate(int list_id,int container_id,int cmd,char *data) { return 0; }
static char *dbContainerPackage(StructDesc *descs,void *mem) { return NULL; }
static void sendInfoBox( Entity *e, int type, const char *s, ... ) {}
static void serverParseClientEx(const char *str, ClientLink *client, int access_level) {}
static U32 dbSecondsSince2000() { return 0; }

TEST(AutoCommandCreateCreates) {
	AutoCommand *command = AutoCommandCreate();
	CHECK(command);
	MP_FREE(AutoCommand, command);
}

TEST(AutoCommandDestroyDestroys) {
	AutoCommand *command = AutoCommandCreate();
	size_t numAllocated = mpGetAllocatedCount(MP_NAME(AutoCommand));
	MP_FREE(AutoCommand, command);
	CHECK_EQUAL(numAllocated - 1, mpGetAllocatedCount(MP_NAME(AutoCommand)));
}
