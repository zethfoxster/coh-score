#include "randomName.h"
#include "stdtypes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memcheck.h"

// Generates a random name
// note: uses name list from http://home.hiwaay.net/~lkseitz/comics/herogen/
//  (used with permission)

#if defined(_DEBUG) || defined(TEST_CLIENT)

char *first[] = {
#	include "list1a.h"
};
char *last[] = {
#	include "list2a.h"
};

extern int randInt2(int max);

char *randomName(void) {
	int good=0;
	int i, j, len;
	int dashed;
	static char ret[256];
	while (!good) {
		dashed=0;
		i = randInt2(ARRAY_SIZE(first));
		j = randInt2(ARRAY_SIZE(last));
		len = strlen(first[i]);
		if (first[i][len-1]!='-')
			good=1;
		else
			dashed=1;
		if (last[j][0]!='-')
			good=1;
		else
			dashed=1;
	}
	sprintf(ret, "%s%s%s", first[i], dashed?"":" ", last[j]);
	return ret;
}

#else

char *randomName(void) {
	static char ret[11] = "RANDOMNAME";
	return ret;
}

#endif