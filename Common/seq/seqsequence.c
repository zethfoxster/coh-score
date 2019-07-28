#include "seqsequence.h"
#include "wininclude.h"  // JS: Can't find where this file includes <windows.h> so I'm just sticking this include here.
#include <string.h>
#include <time.h>
#include "seq.h"
#include "cmdcommon.h"
#include "error.h"
#include "utils.h"
#include "assert.h"
#include "anim.h"
#include "file.h"
#include "mathutil.h"
#include "gfxtree.h"
#include "seqstate.h"
#include "entity.h"
#include "strings_opt.h"
#include "seqload.h"
#if SERVER
#include "cmdserver.h"
#include "entsend.h" //Debug: Just so showstate will work
#include "entserver.h" //Debug: Just so showstate will work
#endif
#if CLIENT
#include "camera.h"
#include "render.h"
#include "sound.h"
#include "font.h"
#include "cmdgame.h"
#include "fx.h"
#include "light.h"
#include "entclient.h" //so showstate will work
#include "comm_game.h"
extern void BugReport(const char * desc, int mode);
#endif
#include "EArray.h"
#include "mailbox.h"
#include "fxinfo.h"
#include "StashTable.h"
#include "SharedMemory.h"
#include "tricks.h" // GFXNODE_HIDE
#include "prefetch.h"

int     uniform_random_number = 0;
int		uniform_random_number_updated = 0;

//############################################################
//###########################################################
//##########################################################
//################ Sequnecer Running


//#####################################################################
//############### Manage the sequencer's state ###########################
void seqClearState(U32 state[])
{
	memset(state, 0, STATE_ARRAY_SIZE * sizeof(U32));
}

void seqSetState(U32 *state,int on,int bitnum)
{
	if (on)
		SETB(state,bitnum);
	else
		CLRB(state,bitnum);
}

void seqOrState(U32 *state,int on,int bitnum)
{
	if (on)
		SETB(state,bitnum);
}

bool seqGetState(U32 *state, int bitnum)
{
	return TSTB(state,bitnum);
}

bool testSparseBit(const SparseBits* bits, U32 bit)
{
	U32 i;

	assert(bit < 0xffffUL);

	for (i = 0; i < bits->count; ++i)
	{
		if (bits->bits[i] == bit)
			return true;
	}
	return false;
}

#ifndef CLIENT
// Just call this variant if want to clear all non-sticky state
void seqClearToStickyState(SeqInst* seq){
	memcpy(seq->state, seq->sticky_state, STATE_ARRAY_SIZE * sizeof(U32));
}
#endif

/* Function getSeqStateBitNum
*	Performs a linear search in the SeqStateNames array.  When
*	the corresponding name is found, the index is returned
*	as the bit number.
*
*	Returns:
*		-1 - given seq state name is not valid
*		int - state bit number of give state name
*/
/*
int seqGetSeqStateBitNum(const char* statename){
int i;
for(i = 0; i < STATE_COUNT; i++){
if(0 == stricmp(statename, SeqStateNames[i]))
return i;
}
return -1;
}*/

/* Function seqSetStateByName()
*	Translates the given bit name into a bit number, then sets the bit to the
*	specified value.
*
*	Parameters:
*		state - A pointer to an array of bits of size STATE_ARRAY_SIZE. 
*				Usually, this points to either SeqInst::state or SeqInst::sticky_state
*		on - New value for the specified bit
*		statename - The bit in the state array to be altered.
*
*
*/
void seqSetStateByName(U32* state, int on, const char* statename){
	int bitnum;
	bitnum = seqGetStateNumberFromName(statename);
	if(bitnum != -1)
		seqSetState(state, on, bitnum);
}

/* Function seqGetStateString()
*	Translates the given sequencer state into its corresponding string form.
*	The returned string holds all the names of the sequencer states that are currently
*	active.  Do not save the returned string pointer as the string will be altered the 
*	next time seqGetStateString is called.
*
*/
char* seqGetStateString(U32* state){
	static char buffer[1024];	

	int i;
	char* printCursor = buffer;
	buffer[0] = '\0';

	for( i = 0 ; i < g_MaxUsedStates ; i++ )
	{	
		// put all "on" seqstate bits' string name into the buffer
		if( stateBits[i].name && TSTB( state, stateBits[i].bitNum ) )
			printCursor += sprintf( printCursor, "%s, ", stateBits[i].name );
	}

	return buffer;
}


//#####################  Manipulated Animations ################################################

void turn_off_anim(	U32 * state, int first, ... )
{
	int		i = first;
	va_list marker;

	va_start( marker, first );     /* Initialize variable arguments. */
	while( i != -1 )
	{
		CLRB( state, i );
		i = va_arg( marker, int );
	}
	va_end( marker );              /* Reset variable arguments.      */
}


// set state given variable length argument list (done frequently in combat and lf_server and live menu)
void play_bits_anim1( U32 *state, int first, ... )
{
	int		i = first;
	va_list marker;

	va_start( marker, first );     /* Initialize variable arguments. */
	while( i != -1 )
	{
		SETB( state, i );
		i = va_arg( marker, int );
	}
	va_end( marker );              /* Reset variable arguments.      */
}


// set state given array (done frequently in combat and lf_server and live menu)
#define	MAX_ANIM_MODS				( 12 )
void play_bits_anim( U32 * state, int * list )
{
	int	i;
	for( i = 0; i < MAX_ANIM_MODS && list[i]; i++ )
		SETB( state, list[i] );
}


//If every bit set in seq_state is also set in curr_state, return true
int requiresMet(U32 *seq_state,U32 *curr_state)
{
	int		i;

	for(i=0;i<STATE_ARRAY_SIZE;i++)
	{
		if ((curr_state[i] & seq_state[i]) != seq_state[i])
			return 0;
	}
	return 1;
}

//If every bit set in seq_state is also set in curr_state, return true
int sparseRequiresMet(const SparseBits *seq_state, const U32 *curr_state)
{
	U32		i;

	for(i=0;i<seq_state->count;i++)
	{
		if (!TSTB(curr_state, seq_state->bits[i]))
			return 0;
	}
	return 1;
}

//Set the state from a string of bit names
int seqSetStateFromString( U32 * state, char * newstate_string )
{
	char delims[]   = " ,\t\n";
	char *state_tok;
	int statebit;
	int success = 1;
	char *  newstate_string_destroyable;

	if( !newstate_string || !state )
		return 0;

	newstate_string_destroyable = strdup( newstate_string );

	state_tok = strtok( newstate_string_destroyable, delims );
	while( state_tok != NULL )
	{
		statebit =  seqGetStateNumberFromName( state_tok );
		if( statebit != -1 )
		{
			SETB( state, statebit );
		}
		else
		{
			success = 0;
		}
		state_tok = strtok( NULL, delims );
	}
	free( newstate_string_destroyable );

	return success;
}

//###################################################################
//############### Handle the synchronized random number generator and
//############## it's use by the sequencer 

//Called on server when you want to update the synchronized random number generator
void seqUpdateServerRandomNumber()
{
	static F32 time = 0;

	time+= TIMESTEP;
	if(time > 1800) //every minute
	{
		time = 0;
		uniform_random_number = (int)rand();
		uniform_random_number_updated = 1;

		//now I need for uniform_random_number to be sent down to all clients
		//who get it and call seqUpdateClientRandomNumber(uniform_random_number);
	}
}

//Called in client networking code when cycle synch message comes down
void seqUpdateClientRandomNumber(int new_number)
{
	uniform_random_number = new_number;
	uniform_random_number_updated = 1;
}

//Called at the end of a frame after you are sure every sequencer that might need to know about the update has seen it
void seqClearRandomNumberUpdateFlag()
{
	uniform_random_number_updated = 0;
}

//Called each frame by all sequencers to get a new synch value if needed, which is hashed 
//by their special id to get their own personal random number seed which they then update 
//with each use. (The same sequencer on different clients use the same special_id)
void seqSynchCycles(SeqInst * seq, int special_id)
{
	if( uniform_random_number_updated || seq->move_predict_seed == 10 ) 
	{
		//Hack here: I never made uniform_random_number actually get send down periodically from the 
		//server, so instead I'm just going to use the entities ownner number.  This should be fine for now.
		//TO DO : actually periodically send a seed.
		seq->move_predict_seed = /*uniform_random_number */ (special_id + 1);
	}
}


//#####################################################################
//###### Advance the Sequencer on the Server ##########################


//make a list of all the states that are set
void seqListAllSetStates( int allstates[MAXSTATES], int * allcnt, U32 * state )
{
	int i, j;
	U32 q;

	memset(allstates, 0, sizeof(int) * g_MaxUsedStates);
	(*allcnt) = 0;

	for(i = 0; i<STATE_ARRAY_SIZE; i++)
	{
		for( j = 0; j < 32 ; j++)
		{
			q = 1 << j;
			if( state[i] & q )
			{
				allstates[(*allcnt)++] = (i*32)+j;
			}
		}
	}
}


int isThisDebugEnt( const SeqInst * seq )
{
	int debugEnt = 0;

	if( strstri( STATE_STRUCT.entity_to_show_state, "selected" ) )
	{
		Entity * e = SELECTED_ENT;
		if( e && (e->seq == seq) )
			debugEnt = 1;
	}
	else if( strstriConst( seq->type->name, STATE_STRUCT.entity_to_show_state ) )
	{
		debugEnt = 1;
	}
	return debugEnt;
}

//This function is kinda dumb, but hey
void stateParse(SeqInst * seq, U32 * state)
{
	static int laststate[MAXSTATES];
	int allstates[MAXSTATES];
	int allcnt;
	int i,printstate = 0;

	if( STATE_STRUCT.show_entity_state && isThisDebugEnt( seq ) )
	{
		seqListAllSetStates( allstates, &allcnt, state ); 

		//has the state list changed?
		printstate = 0;
		for( i=0 ; i < g_MaxUsedStates ; i++)
		{
			if(laststate[i] != allstates[i])
			{
				printstate = 1;
				laststate[i] = allstates[i];
			}
		}

		if(printstate)
		{
			printf("\n\n");
			for(i = 0 ; i < allcnt ; i++)
			{
				consoleSetFGColor( COLOR_RED | COLOR_BLUE | COLOR_BRIGHT );           
				printf("%s \n", seqGetStateNameFromNumber( allstates[i] ) );  
				consoleSetFGColor( COLOR_RED | COLOR_BLUE | COLOR_GREEN);   
			}
			if(!allcnt)
			{
				consoleSetFGColor( COLOR_RED | COLOR_BLUE | COLOR_BRIGHT );           
				printf("NO BITS SET \n" );  
				consoleSetFGColor( COLOR_RED | COLOR_BLUE | COLOR_GREEN); 
			}
		}
	}
}


enum
{
	NO_NEWMOVE_FOUND											= (0),
	NEWMOVE_FOUND_BY_REQUIREMENTS_FAILED_IN_CYCLE_OR_REQINPUTS	= (1 << 1),
	NEWMOVE_FOUND_BY_SEARCH_OF_INTERRUPTS						= (1 << 2),
	NEWMOVE_FOUND_BY_REQUIREMENTS_FAILED_IN_FINISHCYCLE			= (1 << 3),	
	NEWMOVE_FOUND_BY_NONCYCLING_MOVE_ENDED						= (1 << 4),
	NEWMOVE_FOUND_BY_COMPLEX_CYCLING							= (1 << 5),

	NEWMOVE_COMMANDED_FROM_ON_HIGH								= (1 << 6),
};

enum
{
	SEQPROCESSCLIENTINST = 0,
	SEQPROCESSINST		 = 1,
};

static void seqDebugShowMove(const SeqInst * seq, int debug_info, int processType)
{
	const Animation * anim;

	anim = &seq->animation; 

	printf("\n");

	consoleSetFGColor( COLOR_RED | COLOR_BRIGHT);   
	printf( "*%s* ", anim->move->name );  
	if( processType == SEQPROCESSCLIENTINST ) 
	{
		consoleSetFGColor( COLOR_GREEN );
		printf( "(light)" );
	}

	consoleSetFGColor(COLOR_GREEN | COLOR_RED | COLOR_BLUE);

	if( debug_info & NEWMOVE_FOUND_BY_SEARCH_OF_INTERRUPTS ) 
		printf("\t interrupted %s", anim->prev_move->name );

	if( debug_info & NEWMOVE_FOUND_BY_NONCYCLING_MOVE_ENDED )
		printf( "\t %s ended, went to nextmove", anim->prev_move->name );

	if( debug_info & NEWMOVE_FOUND_BY_REQUIREMENTS_FAILED_IN_CYCLE_OR_REQINPUTS )
		printf( "\t reqs not met for %s, went to nextmove", anim->prev_move->name );

	if( debug_info & NEWMOVE_FOUND_BY_REQUIREMENTS_FAILED_IN_FINISHCYCLE )
		printf( "\t reqs not met for %s, went to nextmove after finishing cycle", anim->prev_move->name );

	if( debug_info & NEWMOVE_FOUND_BY_COMPLEX_CYCLING )
		printf( "\t %s ended, went to cyclemove", anim->prev_move->name );

	if( debug_info & NEWMOVE_COMMANDED_FROM_ON_HIGH )
		printf( "\t move commanded from on high" );
	printf("\n");
}


static void JfdChecker(const SeqMove * other_newmove[], int other_newmove_count, const SeqInst * seq)
{
	int highest_priority_yet_found, i;
	const SeqInfo *info = seq->info;
	const U32 *state = seq->state;
	const SeqMove **newmoves = NULL;
	int jfdalert = 0;

	highest_priority_yet_found = 0;
	for(i = 0; i < eaSize(&info->moves); i++)
	{
		const SeqMove *move = info->moves[i];

		if ( move->priority >= highest_priority_yet_found && sparseRequiresMet( &move->raw.requires, state ) ) 
		{	
			if ( move->priority > highest_priority_yet_found )
			{
				eaClearConst(&newmoves);
				highest_priority_yet_found = move->priority;
			}
			eaPushConst(&newmoves, move);
		}
	}

	if(other_newmove_count != eaSize(&newmoves))
		jfdalert = 1;

	for(i = 0; i < other_newmove_count; i++)
	{
		if(eaFind(&newmoves, other_newmove[i]) < 0) // no match
		{
			jfdalert = 1;
			break;
		}
	}

	if(jfdalert)
	{
		printf("\n##JustFuckingDoIt disagreement for %s\n", seq->type->name);
		printf("##Martin seemed to want one of these moves:\n##\t");
		for(i = 0 ; i < eaSize(&newmoves); i++)
			printf("%s  ", newmoves[i]->name);
		printf("\n##but these are the moves found:\n##\t");
		for(i = 0 ; i < other_newmove_count ; i++)
			printf("%s  ", other_newmove[i]->name);
		printf("\n\n");
	}

}//*/
//End Debug stuff

//TIMED MOVE functions ###################################
/*Takes all moves in the TriggeredMove array and puts them into the FutureMoveTracker hopper so they
can begin to be tracked. Note that the only possible triggers at the moment are a timer and a 
particular fx hits you.
*/
#ifdef CLIENT
int seqSetMoveTrigger(FutureMoveTracker fmt_array[], TriggeredMove tm_array[], int * tm_count)
{
	int i, j, result = 0;
	FutureMoveTracker * fmt = 0;
	TriggeredMove * tm = 0;

	if(!fmt_array || !tm_array || !tm_count || *tm_count < 1 || *tm_count > MAX_TRIGGEREDMOVES)
		return 0;

	//For each new triggeredmove
	for( i = 0 ; i < *tm_count ; i++)
	{
		tm = &tm_array[i];

		//Find an empty future move tracker.
		for(j = 0 ; j < MAX_FUTUREMOVETRACKERS ; j++)
		{
			fmt = &(fmt_array[j]);
			if(!fmt->inuse)
			{	
				//Write the triggered_move into the FutureMoveTracker
				fmt->inuse			= 1;
				fmt->move_to_play	= tm->idx;

				fmt->currtime		= 0.0;  //a lot like setFxTrigger, but not enough to merge?
				fmt->triggertime	= tm->ticksToDelay;
				if(tm->triggerFxNetId)
				{
					fmt->triggerFxNetId	= tm->triggerFxNetId; //who will send a message
				}
				result = 1;
				break;
			}
		}
		memset(tm, 0, sizeof(TriggeredMove)); //triggeredmove has been handled 
	}
	*tm_count = 0; //triggeredmoves have all been handled
	return result;
}

/*Checks each move in the FutureMoveTracker hopper against their timers and/or the mesages in the mailbox.
If a move was triggered, then return that move. Note time and fx hit you are the only curent triggers
*/
extern Mailbox globalEntityFxMailbox;
int seqCheckMoveTriggers( FutureMoveTracker fmt_array[] ) 
{
	FutureMoveTracker * fmt;
	int i, move, trigger = 0;

	if( !fmt_array )
		return -1;

	for( i = 0 ; i < MAX_FUTUREMOVETRACKERS ; i++ )
	{
		trigger = 0;
		fmt = &(fmt_array[i]);
		if(fmt->inuse)
		{
			fmt->currtime += TIMESTEP;  
			if(fmt->triggertime && fmt->currtime >= fmt->triggertime)
			{
				trigger = 1;
			}

			if(fmt->triggerFxNetId)
			{
				trigger = lookForThisMsgInMailbox( &globalEntityFxMailbox, fmt->triggerFxNetId, FX_STATE_PRIMEHIT  );
				//if(fmt->currtime > 240) //if after 8 seconds no contact, then clearit
				//	trigger = 1;
			}
		}

		if(trigger)
		{
			move = fmt->move_to_play;
			memset(fmt, 0, sizeof(FutureMoveTracker));
			return move; //only trigger one per frame regardless...
		}
	}

	return -1;
}
#endif

/*setMove:  changes the sequencer's current move, 
If this is a move that 
will change the state, do that, too.  Note that seqClearStates doesn't clear
states set by by SetOutputs (how do they ever get cleared???)  
If this is a simple cycle going to another simple cycle, preserve the frame, otherwise reset it to zero
*/
void seqSetOutputs(U32 * state, const U32 * addedState)
{
	int	i;

	for(i=0;i<STATE_ARRAY_SIZE;i++)
		state[i] |= addedState[i];
}

void seqSetSparseOutputs(U32 * state, const SparseBits * addedState)
{
	U32	i;

	for(i=0;i<addedState->count;i++)
		SETB(state, addedState->bits[i]);
}

//Is this flag set in this move
int getMoveFlags(SeqInst * seq, eMoveFlag flag)
{
	if(!(seq && seq->animation.move))//if anyone asks for the bits on a bad seq, give em zero
		return 0;

	return (seq->animation.move->flags & flag);
}

void seqSetMove( SeqInst * seq, const SeqMove *newmove, int is_obvious )
{
	F32 last_frame;
	F32 first_frame;
	Animation * anim;

	anim = &seq->animation;

	//A teeny bit clunky
	if( newmove )
	{
		anim->typeGfx = seqGetTypeGfx( seq->info, newmove, seq->calculatedSeqTypeName ); //TO DO, SeqInst * seq is overkill

		// fpe for testing, to see animation changes easily in console
		//printf("New move: ENTTYPE %s, sequencer %s, move %s\n", seq->type->name, seq->info->name, newmove->name);

		//Debug
		if( STATE_STRUCT.quickLoadAnims && anim->typeGfx && isDevelopmentMode() && !isSharedMemory(anim->typeGfx) )
			seqAttachAnAnim( cpp_const_cast(TypeGfx*)(anim->typeGfx), CONTEXT_CORRECT_LOADTYPE );
		//End Debug

		if( !anim->typeGfx )
		{
			if (isDevelopmentMode())
				Errorf( "Bad Sequencer Data: ENTTYPE %s sequencer %s No seqType %s (or any parent)  in move %s.", seq->type->name, seq->info->name, seq->type->seqTypeName, newmove->name );
			anim->typeGfx =	newmove->typeGfx[0];
		}

		first_frame = anim->typeGfx->animP[0]->firstFrame;
		last_frame	= anim->typeGfx->animP[0]->lastFrame;
	}
	else
	{
		first_frame = 0;
		last_frame	= 0;
	}
	
	//Reset the frame if any one these is false (usually reset the frame) otherwise preserve frames
	if(!(anim && anim->move && 
		(newmove->flags & SEQMOVE_CYCLE) && 
		(anim->move->flags & SEQMOVE_CYCLE) && 
		anim->frame < last_frame && 
		anim->frame > first_frame && 
		!(anim->move->flags & SEQMOVE_COMPLEXCYCLE) ) ) 
	{
		anim->frame			= first_frame;
		anim->prev_frame	= -1;
	}
	if(seq->gfx_root)
	{
		if(newmove->flags & SEQMOVE_NODRAW)
			seq->gfx_root->flags |= GFXNODE_HIDE;
		else
			seq->gfx_root->flags &= ~GFXNODE_HIDE;
	}
	anim->prev_move		= anim->move;
	anim->move			= newmove;

	if( !is_obvious )
		anim->move_to_send = newmove;

}

/*Wraps cycling animation back around to start
*/
static void seqCycleFrame(F32 * frame, F32 * prev_frame, int first, int last)
{
	*frame -= last - first;
	if (*frame > last)	//protects against very short cycling anims
		*frame = first;	
	*prev_frame = -1;
}


/*Advance the current move animation along it's track, and handle going past the last frame:
If this move cycles and it's requirements are still met, then 
1. either continue the cycle or 
2.if this move has cyclemoves, pick one at random and go to it.
Otherwise go to the nextmove 
(Note: we could only fail requiresmet if SEQMOVE_FINISHCYCLE flag is set,
without it, the requiresmet check in ProcessInst would have caught it)
*/
static const SeqMove * seqStep( Animation * anim, const SeqInfo * info, U32 state[], F32 timestep, int * newmove_cause, int * seed, bool bInRagdoll )
{
	const SeqMove *move;
	const SeqMove *complexcyclemove = 0;
	F32 last_frame;
	F32 first_frame;
	const TypeGfx * typeGfx;
	F32 scale;

	typeGfx = anim->typeGfx; //seqGetTypeGfx( seq->info, move, seq->type->seqTypeName ); //TO DO SeqInst passed in is overkill

	move = anim->move;	
	anim->prev_frame	= anim->frame;

	//This is kinda dumb, but this overriding thing is cuz of how I set steve up with unified seqeucner and now I can't change it.  Well, maybe it's a good thing anyway
	if( typeGfx->scale != 1 )
		scale = typeGfx->scale;
	else
		scale = move->scale;

	anim->frame += scale * timestep;

	//Debug
	if( STATE_STRUCT.quickLoadAnims && !typeGfx->animTrack && isDevelopmentMode() && !isSharedMemory(anim->typeGfx) )
		seqAttachAnAnim( cpp_const_cast(TypeGfx*)(anim->typeGfx), CONTEXT_CORRECT_LOADTYPE );
	//End Debug

	first_frame = typeGfx->animP[0]->firstFrame;
	last_frame  = typeGfx->animP[0]->lastFrame;


	// Switch ragdoll state
	if ( anim->typeGfx->ragdollFrame > 0 && anim->frame > anim->typeGfx->ragdollFrame )
	{
		anim->bRagdoll = true;
	}
	else if ( anim->typeGfx->ragdollFrame < 0 && anim->bRagdoll )
	{
		anim->bRagdoll = false;
	}

	if ( anim->bRagdoll && bInRagdoll )
	{
		anim->frame = last_frame + 0.1f; // ragdoll "animations" always advance just past the last frame
	}

	if ( anim->frame > last_frame )
	{
		if ( ( move->flags & SEQMOVE_CYCLE ) && sparseRequiresMet( &move->raw.requires, state ) )
		{	
			if( move->flags & SEQMOVE_COMPLEXCYCLE )
			{
				int i, j;
				i = randUIntGivenSeed( seed );
				j = i % move->raw.cycleMoveCnt;
				complexcyclemove = (info->moves[ move->raw.cycleMove[j] ] );
				i = ( j == 3 );
				//printf("%d", i);
			}

			if( complexcyclemove && complexcyclemove != move )
			{
				*newmove_cause |= NEWMOVE_FOUND_BY_COMPLEX_CYCLING;
				return complexcyclemove;
			}
			else
			{
				seqCycleFrame( &anim->frame, &anim->prev_frame, first_frame, last_frame );	
			}
		}
		else
		{
			if( move->flags & SEQMOVE_CYCLE )
				*newmove_cause |= NEWMOVE_FOUND_BY_REQUIREMENTS_FAILED_IN_FINISHCYCLE;
			else
				*newmove_cause |= NEWMOVE_FOUND_BY_NONCYCLING_MOVE_ENDED;

			return info->moves[move->raw.nextMove[0]]; //SEQPARSE nextmove
		}
	}
	return 0;
}

//After interruptedBy has been initted, use this
bool seqAInterruptsB( const SeqMove * a, const SeqMove * b )
{
#if 0
	int		i;
	for( i = 0 ; i < MAX_IRQ_ARRAY_SIZE ; i++ )
	{
		if (a->raw.interruptsBitArray[i] & b->raw.memberBitArray[i])
			return true;
	}
	return false;
#else

#if MAX_IRQ_ARRAY_SIZE != 14
#error MAX_IRQ_ARRAY_SIZE is not 14 (required by unrolled code below)
#endif
	const U32* pA = a->raw.interruptsBitArray;
	const U32* pB = b->raw.memberBitArray;

	return !!(	(pA[0] & pB[0]) | 
				(pA[1] & pB[1]) | 
				(pA[2] & pB[2]) | 
				(pA[3] & pB[3]) | 
				(pA[4] & pB[4]) | 
				(pA[5] & pB[5]) | 
				(pA[6] & pB[6]) | 
				(pA[7] & pB[7]) | 
				(pA[8] & pB[8]) | 
				(pA[9] & pB[9]) | 
				(pA[10] & pB[10]) | 
				(pA[11] & pB[11]) | 
				(pA[12] & pB[12]) | 
				(pA[13] & pB[13])
			);
#endif
}

//Given a state array, fill out all the bits that are set
//Assumes setBits array is at least MAXSTATES big
int getSetBits( U32 state[], U32 setBits[] )
{
	int	i, setBitsCount = 0;

	for( i = 0 ; i < g_MaxUsedStates ; i++ )
	{
		if( !state[i/32] ) //Shortcut
			i += 31; //+31, then else, so I get g_MaxUsedState check again
		else if( TSTB(state,i) ) 
			setBits[setBitsCount++] = i;
	}
	return setBitsCount;
}


const SeqMove * seqForceGetMove( SeqInst * seq, U32 state[], U32 requiresThisBit )
{
	int	highest_priority_yet_found, newmove_count = 0;
	const SeqMove *move;
	const SeqMove *newmove[100]; 

	const SeqMove ** allmoves;
	int allmovesSize;

	const SeqBitInfo* bits = seq->info->bits;

	allmoves	= seq->info->moves;
	allmovesSize = eaSize(&allmoves);
	newmove[0]	= 0;
	highest_priority_yet_found	= 0;

	{
		U32 setBits[MAXSTATES];  //TO DO get the list of set bits and the count
		int setBitsCount;
		int i, j;

		setBitsCount = getSetBits( state, setBits );

		if ( setBitsCount > 0 )
			assert( bits );

		//Search the set bit's lists of moves
		for( i = 0 ; i < setBitsCount ; i++ )
		{
			const SeqBitInfo* bitInfo = &bits[setBits[i]];
			for( j = 0 ; j < bitInfo->cnt ; j++ )
			{
				move = seq->info->moves[eaiGet(&bitInfo->movesRemainingThatRequireMe, j)];

				/////// Copy and paste from above 
				if( !requiresThisBit || testSparseBit( &move->raw.requires, requiresThisBit ) ) //hack for HIT bit
				{
					if ( move->priority >= highest_priority_yet_found) 
					{	
						if ( move->priority > highest_priority_yet_found )
						{
							newmove_count = 0;
							highest_priority_yet_found = move->priority;
						}	
						newmove[ newmove_count ] = move;
						newmove_count++;
						assert( newmove_count < allmovesSize );
					}
				}
				/////End Copy and paste from above
			}
		}
	}

	//If several equally best moves were found, choose not randomly amoung them
	if( newmove_count > 1 ) 
	{
		if( newmove[0]->flags & SEQMOVE_PICKRANDOMLY ) //unless you were specifically told to be random
			newmove[0] = newmove[ qrand() % newmove_count ];
		else
			newmove[0] = newmove[ (seq->anim_incrementor++) % newmove_count ];
	}

	return newmove[0];
}

/*### 3. Search the moves that interrupt the current move
for the one with the highest priority (most requirements) whose requirements are met 
If any move is found set it as newmove (choosing amoung several if necessary)
(Note that a new move with a lower priority than the current move that interrupts the 
current move *will* be chosen over the current move itself, assuming the currentmove doesn't 
interrupt itself. Strange)
seq->anim_incrementor++ menas seq is not canst in this function which may be a problem?
/
requiresThisBit is a hack to allow one bit to be flagged as required in the interrupting move.  Used for HIT
*/
static const SeqMove * seqSearchInterrupts( SeqInst * seq, const SeqMove * currmove, U32 state[], U32 requiresThisBit )
{
	int	highest_priority_yet_found, newmove_count = 0;
	const SeqMove *move;
	const SeqMove *newmove[100]; 

	const SeqMove ** allmoves;
	int allmovesSize;

	const SeqBitInfo* bits = seq->info->bits;

	allmoves	= seq->info->moves;
	allmovesSize = eaSize(&allmoves);
	newmove[0]	= 0;
	highest_priority_yet_found	= 0;

	{
		U32 setBits[MAXSTATES];  //TO DO get the list of set bits and the count
		int setBitsCount;
		int i, j;

		setBitsCount = getSetBits( state, setBits );

		if ( setBitsCount > 0 )
			assert( bits );

		//Search the set bit's lists of moves
		for( i = 0 ; i < setBitsCount ; i++ )
		{
			const SeqBitInfo* bitInfo = &bits[setBits[i]];
			for( j = 0 ; j < bitInfo->cnt ; j++ )
			{
				// Prefetch
#define FETCH_AHEAD 3
#if FETCH_AHEAD>0
				if (j+FETCH_AHEAD < bitInfo->cnt)
				{
					move = seq->info->moves[eaiGet(&bitInfo->movesRemainingThatRequireMe, j+FETCH_AHEAD)];
					//Prefetch(move);
					Prefetch(move);
					Prefetch(&move->raw.requires);
					Prefetch(move->raw.interruptsBitArray);
				}
#endif
#undef FETCH_AHEAD

				move = seq->info->moves[eaiGet(&bitInfo->movesRemainingThatRequireMe, j)];

				if( !seq->forceInterrupt && !seqAInterruptsB( move, currmove ) )
					continue;

				/////// Copy and paste from above 
				if( !requiresThisBit || testSparseBit( &move->raw.requires, requiresThisBit ) ) //hack for HIT bit
				{
					if ( move->priority >= highest_priority_yet_found && sparseRequiresMet( &move->raw.requires, state ) ) 
					{	
						if ( move->priority > highest_priority_yet_found )
						{
							newmove_count = 0;
							highest_priority_yet_found = move->priority;
						}	
						newmove[ newmove_count ] = move;
						newmove_count++;
						assert( newmove_count < allmovesSize );

						//TO DO figure out hte effects of this
						if( !( move->priority >= currmove->priority ) )
						{
							//	printf("Hmmm, lower priority move being played, hmmm");
						}
					}
				}
				/////End Copy and paste from above
			}
		}
	}

#ifdef SERVER 	//Debug Info
	if(server_state.check_jfd && TSTB( state, STATE_JUSTFUCKINDOIT )) //debug
		JfdChecker(newmove, newmove_count, seq);
#endif          //End Debug

	//If several equally best moves were found, choose not randomly amoung them
	if( newmove_count > 1 ) 
	{
		if( newmove[0]->flags & SEQMOVE_PICKRANDOMLY ) //unless you were specifically told to be random
			newmove[0] = newmove[ qrand() % newmove_count ];
		else
			newmove[0] = newmove[ (seq->anim_incrementor++) % newmove_count ];
	}

	return newmove[0];
}


//Make animations play slower for bigger characters
//If you are not predictable, or you don't want it, you don't get speed scale,
//otherwise you get capped speedscale unless you specifically ask for uncapped speed scale.
//It's capped so huge monsters don't move crazy slow
//I imagine movement should ask for uncapped speed scale so feet register right.
F32 seqGetAnimScale( const SeqInst * seq )
{
#define UPPER_CAP_TO_ANIM_SIZE_SCALE 1.5
#define LOWER_CAP_TO_ANIM_SIZE_SCALE 0.5
	const SeqMove *currmove;

	F32 animscale = 1.0;

	currmove = seq->animation.move; 

	//Default for predictable is SEQ_CAPPEDSIZESCALE
	if( currmove && ((currmove->flags & SEQMOVE_PREDICTABLE) || (currmove->flags & SEQMOVE_ALWAYSSIZESCALE)) && !(currmove->flags & SEQMOVE_NOSIZESCALE) )  
	{
		animscale = seq->curranimscale; 

		if(!(currmove->flags & SEQMOVE_FULLSIZESCALE) )
			animscale = MINMAX( animscale, LOWER_CAP_TO_ANIM_SIZE_SCALE, UPPER_CAP_TO_ANIM_SIZE_SCALE );
	}

	return animscale * seq->type->animScale;
}


static void seqDebugPrintState( const SeqInst * seq, const SeqMove *newmove, int newmove_cause, int processType )
{
	if( STATE_STRUCT.show_entity_state && isThisDebugEnt( seq )) 
	{
		static int framesPrinted = 0;
		if ( newmove )  
		{
			framesPrinted = 0;
			seqDebugShowMove(seq, newmove_cause, processType );
		}

		if( framesPrinted < 60 ) 
		{ 
			if( processType == SEQPROCESSCLIENTINST )
				consoleSetFGColor( COLOR_GREEN );
			else //( processType == SEQPROCESSINST )
				consoleSetFGColor( COLOR_BRIGHT );        
			printf( "%0.0f ", seq->animation.frame );
			consoleSetFGColor( COLOR_RED | COLOR_BLUE | COLOR_GREEN);  
		}
		if( framesPrinted == 60 )
		{
			if( processType == SEQPROCESSCLIENTINST )
				consoleSetFGColor( COLOR_GREEN );
			else //( processType == SEQPROCESSINST )
				consoleSetFGColor( COLOR_BRIGHT );          
			printf( "...and so on" );
			consoleSetFGColor( COLOR_RED | COLOR_BLUE | COLOR_GREEN);  
		}
		framesPrinted++;
	}
}


/*Advances the Animation through the SeqInfo given.  If the current move changes, it returns a ptr to
that move.  
(Note: move_predicted_on_client is set to TRUE if the move found is a nextmove not arrived at by breaking
out of a cycle. the newmove is considered an extension of the previous move, and the server assumes the clients 
have found the move on their own, and won't send that move to them)
*/
const SeqMove * seqProcessInst( SeqInst *seq, F32 timestep )
{
	const SeqMove *currmove;
	const SeqMove *newmove;
	Animation	*anim;
	U32			*state;
	int			newmove_cause = 0;

	anim		= &seq->animation;
	state		= seq->state;
	currmove	= anim->move;
	newmove		= 0;

#ifdef CLIENT
	if( STATE_STRUCT.show_entity_state ) //Nice little debug thing to get a particular entity, works client and server
	{ Entity * e = SELECTED_ENT;
	if( e && (e->seq == seq) )
		xyprintf( 30, 30, "Animation Scale: %f", seqGetAnimScale( seq ) );
	}
#endif

	assert(currmove);

	timestep *= seqGetAnimScale( seq ); 

	//### 1. If the move or state has changed fom last frame, Search current move's interrupts for a better move
	if( seq->forceInterrupt || seq->animation.move_lastframe != currmove || memcmp(seq->state_lastframe, state, sizeof(seq->state)) )
	{
		memcpy( seq->state_lastframe, state, sizeof(seq->state) );	//record the move and state... 
		seq->animation.move_lastframe = currmove;					//..change for next frame

		newmove = seqSearchInterrupts(seq, currmove, state, 0 );

		if( newmove )
			newmove_cause |= NEWMOVE_FOUND_BY_SEARCH_OF_INTERRUPTS;
	}

	//### 2. If no newmove and current move's reqs must be met every frame, check that.  
	if ( !seq->forceInterrupt && (!newmove || newmove == currmove)
		&& ( currmove->flags & (SEQMOVE_CYCLE | SEQMOVE_REQINPUTS) ) && !( currmove->flags & SEQMOVE_FINISHCYCLE ) 
		&& !sparseRequiresMet(&currmove->raw.requires, state) )
	{	
		//SEQPARSE nextmove
		newmove = seq->info->moves[currmove->raw.nextMove[0]];
		newmove_cause |= NEWMOVE_FOUND_BY_REQUIREMENTS_FAILED_IN_CYCLE_OR_REQINPUTS;
	}

	//### 3. If still no newmove, advance the current move; check to see if it rolls over to it's nextmove
	if(!seq->forceInterrupt && (!newmove || newmove == currmove))
	{
		newmove = seqStep( anim, seq->info, state, timestep, &newmove_cause, &seq->move_predict_seed, seq->bCurrentlyInRagdollMode );
	}

	//### 4.If there is a new move, set it up
	if(!seq->forceInterrupt && newmove == currmove) //don't bother if the move hasn't changed
		newmove = 0;

	if ( newmove )
	{
		int move_predicted_on_client = 0;
		if(!anim->bRagdoll)
		{
			if(newmove_cause & NEWMOVE_FOUND_BY_NONCYCLING_MOVE_ENDED)
				move_predicted_on_client = 1;
			else if((newmove_cause&NEWMOVE_FOUND_BY_COMPLEX_CYCLING) && !seqScaledOnServer(seq))
				move_predicted_on_client = 1;
		}
		seqSetMove(seq, newmove, move_predicted_on_client);
	}

	seqDebugPrintState( seq, newmove, newmove_cause, SEQPROCESSINST ); //Debug
	seq->forceInterrupt = false;

	return newmove;
}

//########################################################################
///################# Advance SeqInst on the Client #######################

/*Step to the next frame of the animation. If the end is reached, either cycle or go to next move (prediction).
note: don't check the requirements to keep cycling, just do it
*/
static const SeqMove * seqClientStep( Animation * anim, const SeqInfo * info, F32 timestep, int * seed, int * newmove_cause, bool bInRagdoll)
{
	const SeqMove *move;
	const SeqMove *complexcyclemove = 0;
	int	firstFrame, lastFrame;

	move = anim->move;
	anim->prev_frame	= anim->frame;
	anim->frame		   += move->scale * timestep;

	{ //TO DO only purpose of this is getting first and last frame, think of a fast way 
		const TypeGfx *typeGfx = anim->typeGfx; //seqGetTypeGfx( seq->info, move, seq->type->seqTypeName ); //TO DO SeqInst passed in is overkill

		//Debug
		if( STATE_STRUCT.quickLoadAnims && !typeGfx->animTrack && isDevelopmentMode() && !isSharedMemory(anim->typeGfx) )
			seqAttachAnAnim( cpp_const_cast(TypeGfx*)(anim->typeGfx), CONTEXT_CORRECT_LOADTYPE );
		//End Debug

		firstFrame	= typeGfx->animP[0]->firstFrame;
		lastFrame	= typeGfx->animP[0]->lastFrame;
	}

	// Switch ragdoll state
	if ( anim->typeGfx->ragdollFrame > 0 && anim->frame > anim->typeGfx->ragdollFrame )
	{
		anim->bRagdoll = true;
	}
	else if ( anim->typeGfx->ragdollFrame < 0 && anim->bRagdoll )
	{
		anim->bRagdoll = false;
	}

	if ( bInRagdoll && anim->bRagdoll )
	{
		anim->frame = lastFrame + 0.1f; // always advance ragdoll "anims" just past the end of the anim
	}

	if ( anim->frame > lastFrame )
	{
		if ( move->flags & SEQMOVE_CYCLE )
		{
			if( move->flags & SEQMOVE_COMPLEXCYCLE )
				complexcyclemove = info->moves[ move->raw.cycleMove[ randUIntGivenSeed( seed ) % move->raw.cycleMoveCnt ] ] ;

			if( complexcyclemove && complexcyclemove != move )
			{
				*newmove_cause |= NEWMOVE_FOUND_BY_COMPLEX_CYCLING;
				return complexcyclemove;
			}
			else
			{
				seqCycleFrame(&anim->frame, &anim->prev_frame, firstFrame, lastFrame);	
			}
		}
		else
		{	
			*newmove_cause |= NEWMOVE_FOUND_BY_NONCYCLING_MOVE_ENDED;
			return info->moves[move->raw.nextMove[0]]; //go to next move  //SEQPARSE nextmove
		}
	}
	return 0;
}

/*Advance the sequencer along current track unless explicitly given a different move
in which case set it. (Go to next moves if move !cycle) TO DO: add debugger for client prediction output  
*/
const SeqMove * seqProcessClientInst( SeqInst * seq, F32 timestep, int move_idx, U8 move_updated )
{
	Animation * anim;
	const SeqInfo * info;
	const U32 * state;
	const SeqMove	* newmove = 0;
	int hasNewMove;
	int	newmove_cause = 0;

	anim = &seq->animation;
	info = seq->info;
	state = seq->state;
	hasNewMove = 0;
	timestep *= seqGetAnimScale( seq ); 

	//Get the new move if it exists; if error, goto ready, 
	if(move_updated)
	{
		if( move_idx >= eaSize( &info->moves ) ) //error check
			move_idx = 0;
		newmove = info->moves[ move_idx ];
		newmove_cause = NEWMOVE_COMMANDED_FROM_ON_HIGH;
	}

	//If no newmove, advance the current one
	if( !newmove || newmove == anim->move )
	{	
		newmove = seqClientStep( anim, info, timestep, &seq->move_predict_seed, &newmove_cause, seq->bCurrentlyInRagdollMode);
	}

	//If there is a new move, set it up
	if ( newmove && newmove != anim->move)
	{
		seqSetMove( seq, newmove, 0 );
		hasNewMove = 1;
	}	

	seqDebugPrintState( seq, newmove, newmove_cause, SEQPROCESSCLIENTINST ); //Debug

	return hasNewMove ? newmove : NULL;
}


/*TimedMove given a state, find the move to play for it and return the move number only called on server*/
int seqFindAMove(SeqInst * seq, U32 state[], U32 requiresThisBit)
{
	const SeqMove * move = 0;

	move = seqSearchInterrupts( seq, seq->animation.move, state, requiresThisBit );

	if(move)
		return move->raw.idx;
	else
		return -1;
}
