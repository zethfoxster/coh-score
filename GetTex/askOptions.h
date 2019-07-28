
typedef enum OptionListFlags {
	OLF_NORMAL	=1<<0,
	OLF_CHILD	=1<<1,
} OptionListFlags;

typedef struct OptionList {
	char *name;
	int *value;
	char *regKey;
	OptionListFlags flags;
} OptionList;

// Returns 0 if they hit Cancel, 1 for OK
int doAskOptions(const char *appName, OptionList *options, int numOptions, float timeout, int flags, const char *showOnceHelpText);
