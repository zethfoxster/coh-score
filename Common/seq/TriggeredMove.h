#ifndef TRIGGEREDMOVE_H
#define TRIGGEREDMOVE_H

#define MAXSTATES 877 // keep this clamped down tightly to the required size, for optimal performance
#define STATE_ARRAY_SIZE ((MAXSTATES + 31)/32) //now adjective array size.  anything over this is a noun and gets it's own number

typedef struct TriggeredMove  //figure out where to put this...
{
	int		ticksToDelay;		// condition: how many 1/30's of a second to delay - or - 
	int		triggerFxNetId;  // condition: fx whose hit will trigger this move.
	int		idx;	// move to play
	F32		timeCreated;
	U8		move_found;		//has this triggeredmove been evaluated by the sequencer yet? server only
	U32		state[STATE_ARRAY_SIZE]; //state to use to find the move. server only

} TriggeredMove;

#endif