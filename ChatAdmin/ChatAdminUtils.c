#include "ChatAdminUtils.h"
#include "RegistryReader.h"


RegReader rr = 0;
int name_count=0;
char namelist[256][256];

// maximum # of entries in a combo box (read from registry)
#define MAX_COMBO_BOX_ENTRIES 15

void initRR() {
	if (!rr) {
		rr = createRegReader();
		initRegReader(rr, "HKEY_CURRENT_USER\\SOFTWARE\\Cryptic\\ChatAdmin");
	}
}
const char *regGetString(const char *key, const char *deflt) {
	static char buf[512];
	initRR();
	strcpy(buf, deflt);
	rrReadString(rr, key, buf, 512);
	return buf;
}

void regPutString(const char *key, const char *value) {
	initRR();
	rrWriteString(rr, key, value);
}

void regDeleteString(const char *key) {
	initRR();
	rrDelete(rr, key);
}

int regGetInt(const char *key, int deflt) {
	static int value;
	initRR();
	value = deflt;
	rrReadInt(rr, key, &value);
	return value;
}

void regPutInt(const char *key, int value) {
	initRR();
	rrWriteInt(rr, key, value);
}


void getNameList(const char * category) {
	char key[256];
	char *badvalue="BadValue";
	const char *name;
	int i;
	name_count = 0;
	for(i=0; i < MAX_COMBO_BOX_ENTRIES; i++) {
		sprintf(key, "%s%d", category, i);
		name = regGetString(key, badvalue);
		if (strcmp(name, badvalue) != 0)
			strcpy(namelist[name_count++], name);
	}
}


void addNameToList(const char * category, char *name) {
	int i;
	char key[256];

	// Reg. is ready every time, since this is now shared by "relay" and "server monitor" dialogs
	getNameList(category);

	if(name_count >= MAX_COMBO_BOX_ENTRIES)
		return;

	for (i=0; i<name_count; i++) {
		if (stricmp(namelist[i], name)==0)
			return;
	}

	sprintf(key, "%s%d", category, name_count);
	regPutString(key, name);

	// might not be needed now, since registry is ready every time
	// this function is called
	strcpy(namelist[name_count++], name);
}