#include "failtext.h"
#include "stdtypes.h"
#include "font.h"
#include "mathutil.h"
#include "superassert.h"
#include "file.h" // for isDevelopmentOrQAMode
#include <time.h>
#include <windows.h>

#define FAILTEXT_SIZE 128
#define FAILTEXT_COUNT 10

typedef struct failTextColor {
	float elapsed;
	Vec3 color;
} failTextColor;

static failTextColor colors[] = {
	{-1.0f, {1, 1, 1}},
	{ 0.0f, {1, 1, 0}},
	{ 1.0f, {1, 0, 0}},
	{ 2.0f, {0, 0, 1}},
	{ 8.0f, {0, 0, 0}},
};

typedef struct failTextEntry {
	clock_t ticks;
	char buf[FAILTEXT_SIZE];
} failTextEntry;

typedef struct failTextGlobals {
	int lock_inited;
	CRITICAL_SECTION lock;
	int count;
	struct failTextEntry entries[FAILTEXT_COUNT];
} failTextGlobals;

static failTextGlobals globals = {0};

static int failTextEntryCmp(const void * a, const void * b)
{
	const failTextEntry * ea = (const failTextEntry *)a;
	const failTextEntry * eb = (const failTextEntry *)b;

	// sort by most recent time first
	if (ea->ticks > eb->ticks) return -1;
	if (ea->ticks < eb->ticks) return 1;

	// sort by address second to enforce a stable sort
	if (ea < eb) return -1;
	if (ea > eb) return 1;

	return 0;
}

void failText(const char * msg, ...)
{
	va_list va;
	int i;
	int oldest = -1;
	clock_t oldestTicks = 0;
	char buf[FAILTEXT_SIZE];

	if(!isDevelopmentOrQAMode()) {
		return;
	}

	if(!globals.lock_inited) {
		InitializeCriticalSection(&globals.lock);
		globals.lock_inited = 1;
	}

	// Build and mark string as truncated if it is too long
	va_start(va, msg);
	if (vsnprintf(buf, FAILTEXT_SIZE, msg, va) <= 0) {
		strcpy(buf + FAILTEXT_SIZE - 5, "...");
	}
	va_end(va);

	EnterCriticalSection(&globals.lock);

	// Find an existing match and best replacement in case we are full
	for (i = 0; i < globals.count; i++) {
		if(strcmp(buf, globals.entries[i].buf) == 0) {
			globals.entries[i].ticks = 0;
			LeaveCriticalSection(&globals.lock);
			return;
		} else if(globals.entries[i].ticks) {
			if(globals.entries[i].ticks < oldestTicks || !oldestTicks) {
				oldest = i;
				oldestTicks = globals.entries[i].ticks;
			}
		}
	}

	// Append
	if (globals.count < FAILTEXT_COUNT) {
		failTextEntry * entry = globals.entries + globals.count;
		strcpy(entry->buf, buf);
		entry->ticks = 0;
		globals.count++;
	} else if(oldest != -1) { // Replace
		failTextEntry * entry = globals.entries + oldest;
		strcpy(entry->buf, buf);
		entry->ticks = 0;
	}

	LeaveCriticalSection(&globals.lock);
}

int failTextGetDisplayHt()
{
	// return height of display (for others who want to draw under us)
	return( globals.count * 8); 
}

void failTextDisplay()
{
	int x = 0;
	int y = 0;
	clock_t curTicks = clock();
	int i, j;

	if(!isDevelopmentOrQAMode()) {
		return;
	}

	if(!globals.lock_inited) {
		InitializeCriticalSection(&globals.lock);
		globals.lock_inited = 1;
	}

	EnterCriticalSection(&globals.lock);

	// Render in sorted order
	for (i = 0; i < globals.count; i++) {
		Vec3 color = {0,0,0};
		failTextEntry * entry = globals.entries + i;
		float elapsed;

		if(!entry->ticks) entry->ticks = curTicks;
		elapsed = (float)(curTicks - entry->ticks) / (float)CLOCKS_PER_SEC;

		// Find the color for the tick count of this object
		for (j = sizeof(colors)/sizeof(colors[0])-1; j >= 0; j--) {			
			if(elapsed > colors[j].elapsed) {
				copyVec3(colors[j].color, color);
				break;
			}
		}
		if(vec3IsZero(color)) {
			entry->ticks = 0xFFFFFFFF;
		}

		xyprintfcolor(x, y, color[0] * 255.9f, color[1] * 255.9f, color[2] * 255.9f, entry->buf);

		y++;
	}

	// Collapse expired entries
	for (i = globals.count-1; i >= 0; i--) {
		if(globals.entries[i].ticks == 0xFFFFFFFF) {
			memmove(globals.entries + i, globals.entries + i + 1, globals.count - i - 1);
			globals.count--;
		}		
	}

	LeaveCriticalSection(&globals.lock);
}
