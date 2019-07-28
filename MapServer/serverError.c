#include "serverError.h"
#include "utils.h"
#include "error.h"
#include "AppVersion.h"
#include "assert.h"
#include "dbcomm.h"
#include "winutil.h"
#include "sysutil.h"
#include "cmdserver.h"
#include "logcomm.h"
#include "file.h"

static int dialogBoxOverride;

static void addErrorToQueue(char *str);

//------------------------------------------------------------
// Error callbacks
//------------------------------------------------------------
// MAK - do some scrounging around for the author info, then do popup
void PerforceErrorDialog(char* errMsg)
{
	errorDialog(compatibleGetConsoleWindow(), errMsg, 0, NULL, errorWasForceShown());
}

#define NEVER_SHOW_DIALOG_BOX 1
#define FORCE_SHOW_DIALOG_BOX 2

void serverErrorfSetNeverShowDialog()
{
	dialogBoxOverride = NEVER_SHOW_DIALOG_BOX;
}

void serverErrorfSetForceShowDialog()
{
	dialogBoxOverride = FORCE_SHOW_DIALOG_BOX;
}

void serverErrorfCallback(char* errMsg)
{
	extern int write_templates;

	if( isDevelopmentMode() )
		printf_stderr("%s\n", errMsg);

	// Put up a blocking dialog box if this is a local mapserver (not a spawned mapserver)
	if ((dialogBoxOverride == FORCE_SHOW_DIALOG_BOX
			// These items are overridden by FORCE_SHOW_DIALOG_BOX
			|| (dialogBoxOverride != NEVER_SHOW_DIALOG_BOX && db_state.local_server && !server_state.tsr))
		// These items are not overridden by FORCE_SHOW_DIALOG_BOX
		&& ErrorfCount() < 5 && !server_state.create_bins && !write_templates)
	{
		PerforceErrorDialog(errMsg);
	}
	else
	{
		addErrorToQueue(errMsg);
	}

	// Rather than have Errorf silently send log messages to logserver, I'd rather make that call more explicit
	//dbLogBug("\n(Server Error Msg)\nServer Ver:%s\n%s\n@@END\n\n\n\n\n\n\n\n\n", getExecutablePatchVersion("CoH Server"), errMsg);
}

//------------------------------------------------------------
// Error queuing
//------------------------------------------------------------
static int error_queue_count,error_queue_max;
static char *error_queue;

static void addErrorToQueue(char *str)
{
	char	*s;

	s = dynArrayAdd(&error_queue,1,&error_queue_count,&error_queue_max,(int)strlen(str)+1);
	strcpy(s,str);
	s[strlen(s)] = 0;
}

char *errorGetQueued()
{
	char	*s;
	static	int		curr_pos;

	if (!error_queue_count)
		return 0;
	s = error_queue + curr_pos;
	curr_pos += (int)strlen(s)+1;
	if (curr_pos >= error_queue_count)
	{
		error_queue_count = 0;
		curr_pos = 0;
	}
	return s;
}
