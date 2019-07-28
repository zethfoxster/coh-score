#include "rt_tune.h"
#include "mathutil.h"
#include "font.h"
#include "superassert.h"
#include "failtext.h"

#define TUNE_MAX_DEPTH 5

#define STDMOD(a,b) ((a) - (b) * floor((float)(a) / (float)(b)))
#define CLAMP(val,min,max) MIN(MAX(val, min), max)
#define WRAP(val,min,max) ((STDMOD(val-min, max-min))+min)

enum tuneType {
	TUNE_INVALID=0,
	TUNE_MENU,
	TUNE_CALLBACK,
	TUNE_BIT,
	TUNE_ENUM,
	TUNE_INTEGER,
	TUNE_FLOAT
};

typedef struct _tuneVar {
	const char * name;
	enum tuneType type;
	void (*callback)();
	union {
		struct {
			struct _tuneMenu * menu;
		} m;
		struct {
			unsigned int * value;
			unsigned int bit;
		} b;
		struct {
			int * value;
			int count;
			const int * values;
			const char * const * names;
		} e;
		struct {
			int * value;
			int tick;
			int min;
			int max;
		} i;
		struct {
			float * value;
			float tick;
			float min;
			float max;
		} f;
	};
} tuneVar;

typedef struct _tuneMenu {
	const char * name;
	int top;
	int selected;
	int count;
	tuneVar * vars;
} tuneMenu;

typedef struct _tuneGlobals {
	int enabled;
	int drawRows;

	tuneMenu root;

	int addStackDepth;
	tuneMenu * addStack[TUNE_MAX_DEPTH];

	int drawStackDepth;
	tuneMenu * drawStack[TUNE_MAX_DEPTH];
} tuneGlobals;

static tuneGlobals g_tune = {0};

static tuneMenu * getDrawMenu()
{
	if(!g_tune.drawStack[0]) {
		g_tune.drawStack[0] = &g_tune.root;
	}
	return g_tune.drawStack[g_tune.drawStackDepth];
}

static tuneMenu * getAddMenu()
{
	if(!g_tune.addStack[0]) {
		g_tune.addStack[0] = &g_tune.root;
	}
	return g_tune.addStack[g_tune.addStackDepth];
}

static int tuneEnumIndex(tuneVar * var, int value)
{
	int i;
	for(i = 0; i < var->e.count; i++) {
		if(var->e.values[i] == value) {
			return i;
		}
	}
	return 0;
}

static void tuneTick(tuneVar * var, int speed)
{
	switch(var->type) {
		case TUNE_MENU: {
			assert(g_tune.drawStackDepth < TUNE_MAX_DEPTH);
			g_tune.drawStack[++g_tune.drawStackDepth] = var->m.menu;
			break;
		}
		case TUNE_CALLBACK: {
			break;
		}
		case TUNE_BIT: {
			*var->b.value ^= var->b.bit;
			break;
		}
		case TUNE_ENUM: {
			int index = tuneEnumIndex(var, *var->e.value) + speed;
			index = WRAP(index, 0, var->e.count);
			*var->e.value = var->e.values[index];
			break;
		}
		case TUNE_INTEGER: {
			int val = *var->i.value + var->i.tick * speed;
			*var->i.value = CLAMP(val, var->i.min, var->i.max);
			break;
		}
		case TUNE_FLOAT: {
			float val = *var->f.value + var->f.tick * (float)speed;
			*var->f.value = CLAMP(val, var->f.min, var->f.max);
			break;
		}
	}
	if(var->callback) {
		var->callback();
	}
}

void tuneInput(TuneInputCommand * input)
{
	tuneMenu * menu = getDrawMenu();

	for (; input->cmd; input++)
	{
		if(input->cmd == tuneInputToggle) {
			g_tune.enabled = !g_tune.enabled;
			continue;
		}

		if(!g_tune.enabled) {
			continue;
		}

		switch (input->cmd) {
			case tuneInputBack: g_tune.drawStackDepth = MAX(g_tune.drawStackDepth-1, 0); break;
			case tuneInputUp: menu->selected = menu->selected--; break;
			case tuneInputDown: menu->selected = menu->selected++; break;
			case tuneInputLeft: tuneTick(menu->vars + menu->selected, -1 * (input->shift ? 10 : 1) * (input->ctrl ? 100 : 1) * (input->alt ? 1000 : 1)); break;
			case tuneInputRight: tuneTick(menu->vars + menu->selected, +1 * (input->shift ? 10 : 1) * (input->ctrl ? 100 : 1) * (input->alt ? 1000 : 1)); break;
		}
	}

	// Set draw rows to an initial value
	if(!g_tune.drawRows) {
		g_tune.drawRows = 40;
	}

	// Wrap
	if(menu->selected < 0) {
		menu->selected = menu->count-1;
	}
	if(menu->selected >= menu->count) {
		menu->selected = 0;
	}

	// View seleted
	if(menu->top > menu->selected) {
		menu->top = menu->selected;
	}
	if(menu->top <= menu->selected-g_tune.drawRows) {
		menu->top = menu->selected-g_tune.drawRows+1;
	}
}

void tuneDraw()
{
	int i;
	char buf[256];

	float s = 8.0f;
	float x0 = -1.0f;
	float x1 = x0 + s*30.0f;
	float y = s + failTextGetDisplayHt(); // render below the failtext so no overlap
	float z = 0.0f;

	tuneMenu * menu = getDrawMenu();

	if(!g_tune.enabled) {
		return;
	}

	fontDefaults();
	fontSet(0);

	if(menu->name) {
		fontColor(0x00FF00);
		fontText(x1, y, (char*)menu->name);
	}
	if(menu->top > 0) {
		fontColor(0xFF0000);
		fontText(x0, y, "...");
	}
	y += s;

	for(i=menu->top; i<menu->top+g_tune.drawRows && i<menu->count; i++) {
		tuneVar * var = menu->vars + i;

		if(i == menu->selected) {
			fontColor(0xFF7F00);
		} else {
			fontColor(0xFF0000);
		}

		fontText(x0, y, (char*)var->name);
		switch(var->type) {
			case TUNE_MENU: {
				snprintf(buf, sizeof(buf), "...");
				break;
			}
			case TUNE_CALLBACK: {
				snprintf(buf, sizeof(buf), "<->");
				break;
			}
			case TUNE_BIT: {
				snprintf(buf, sizeof(buf), "%s", (*var->b.value & var->b.bit) ? "On" : "Off");
				break;
			}
			case TUNE_ENUM: {
				snprintf(buf, sizeof(buf), "%s", var->e.names[tuneEnumIndex(var, *var->e.value)]);
				break;
			}
			case TUNE_INTEGER: {
				snprintf(buf, sizeof(buf), "%d", *var->i.value);
				break;
			}
			case TUNE_FLOAT: {
				snprintf(buf, sizeof(buf), "%f", *var->f.value);
				break;
			}
		}
		fontText(x1, y, buf);

		y += s;
	}
	if(menu->top + g_tune.drawRows < menu->count) {
		fontColor(0xFF0000);
		fontText(x0, y, "...");
	}
}

static tuneVar * tuneAdd(const char * name)
{
	int i;
	tuneMenu * menu = getAddMenu();

	for(i=0; i<menu->count; i++) {
		if(strcmp(name, menu->vars[i].name)==0) {
			break;
		}
	}
	if(i == menu->count) {
		menu->vars = realloc(menu->vars, ++menu->count * sizeof(tuneVar));		
		memset(menu->vars + i, 0, sizeof(tuneVar));
		menu->vars[i].name = name;
	}
	return menu->vars+i;
}

void tunePushMenu(const char * name)
{
	tuneVar * var = tuneAdd(name);
	if(var->type != TUNE_MENU) {
		var->m.menu = calloc(1, sizeof(tuneMenu));
	}
	var->type = TUNE_MENU;
	var->callback = NULL;
	var->m.menu->name = name;

	assert(g_tune.addStackDepth < TUNE_MAX_DEPTH);
	g_tune.addStack[++g_tune.addStackDepth] = var->m.menu;
}

void tunePopMenu()
{
	assert(g_tune.addStackDepth > 0);
	g_tune.addStackDepth--;
}

void tuneCallback(const char * name, void (*callback)())
{
	tuneVar * var = tuneAdd(name);
	var->type = TUNE_CALLBACK;
	var->callback = callback;
}

void tuneFloat(const char * name, float * f, float tick, float min, float max, void (*callback)())
{
	tuneVar * var = tuneAdd(name);
	var->type = TUNE_FLOAT;
	var->callback = callback;
	var->f.value = f;
	var->f.tick = tick;
	var->f.min = min;
	var->f.max = max;
}

void tuneInteger(const char * name, int * i, int tick, int min, int max, void (*callback)())
{
	tuneVar * var = tuneAdd(name);
	var->type = TUNE_INTEGER;
	var->callback = callback;
	var->i.value = i;
	var->i.tick = tick;
	var->i.min = min;
	var->i.max = max;
}

void tuneBit(const char * name, unsigned int * u, unsigned int bit, void (*callback)())
{
	tuneVar * var = tuneAdd(name);
	var->type = TUNE_BIT;
	var->callback = callback;
	var->b.value = u;
	var->b.bit = bit;
}

void tuneBoolean(const char * name, bool * b, void (*callback)())
{
	tuneBit(name, (unsigned int*)b, 0x1, callback);
}

void tuneEnum(const char * name, int * e, const int * values, const char * const * names, void (*callback)())
{
	const char * const * n;
	tuneVar * var = tuneAdd(name);
	var->type = TUNE_ENUM;
	var->callback = callback;
	var->e.value = e;
	var->e.values = values;
	var->e.names = names;
	var->e.count = 0;
	for(n = names; *n; n++) {
		var->e.count++;
	}
}

