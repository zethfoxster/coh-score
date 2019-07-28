#ifndef RT_TUNE_H
#define RT_TUNE_H

enum TuneInputDir {
	tuneInputTerminator = 0,
	tuneInputToggle,
	tuneInputBack,
	tuneInputUp,
	tuneInputDown,
	tuneInputLeft,
	tuneInputRight,
};

typedef struct TuneInputCommand {
	U8 cmd;
	U32 shift:1;
	U32 ctrl:1;
	U32 alt:1;
} TuneInputCommand;

void tuneInput(TuneInputCommand * input);
void tuneDraw();

void tunePushMenu(const char * name);
void tunePopMenu();
void tuneCallback(const char * name, void (*callback)());
void tuneFloat(const char * name, float * f, float tick, float min, float max, void (*callback)());
void tuneInteger(const char * name, int * i, int tick, int min, int max, void (*callback)());
void tuneBit(const char * name, unsigned int * u, unsigned int bit, void (*callback)());
void tuneBoolean(const char * name, bool * b, void (*callback)());
void tuneEnum(const char * name, int * e, const int * values, const char * const * names, void (*callback)());

#endif

