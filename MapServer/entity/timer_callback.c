/***************************************************************************
 *     Copyright (c) 2000-2006, Cryptic Studios
 *     All Rights Reserved
 *     Confidential Property of Cryptic Studios
 ***************************************************************************/
#include "entity.h"
#include "cmdcommon.h"
#include "timer_callback.h"
#include "wininclude.h"

#define MAX_TIMERS     ( 100 )
#define MAX_ALTER_WORK ( 100 )

#define CHECK_INIT  if( !initialized ) { initTimerCallbacks(); }

static void initAMemPool(MemPool *mp, int node_size, int num_nodes, char *mem);
static void freeMemNode(MemPool **mem, void *chunk);
static void *getMemNode( MemPool * mp );
static void freeMe(int curr, int prev);
static void initTimerCallbacks(void);
static int findMyPrevLink(int me, int start);
static void deleteNode(int curr, int prev);
static int inFreeList( int idx );
static void checkTimerCallbacks(void);

static int           initialized;
static int           first_game_obj, first_free_obj;
static TimerCallback timers[ MAX_TIMERS ];
static AlterWork     alter_objs[ MAX_ALTER_WORK ];

MemPool     alter_work_mem_pool;

static void initAMemPool(MemPool *mp, int node_size, int num_nodes, char *mem)
{
	int i, *last = ( int * )NULL;

	for(i=0; i<num_nodes; i++ )
	{
		int *j;

		j    = ( int * )( mem + i * node_size );     //first int in each node is a * to free list.
		*j   = ( int )last;
		last = j;
	}

	mp->storage     = mem;
	mp->free_list   = last;
}

static void freeMemNode(MemPool **mem, void *chunk)
{
	if( *mem )
	{
		*( int *)chunk    = ( int )(*mem)->free_list; //Point chunk to old top of free_list and push freed chunk back to head of
		(*mem)->free_list = chunk;                    //free_list.  Remember 0th int in MemPool struct is a * to free_list.
		*mem              = ( MemPool * )NULL;
	}
}

static void *getMemNode( MemPool * mp )
{
	void *a = ( void * )NULL;

	if( mp->free_list )
	{
		a               = mp->free_list;
		mp->free_list   = ( int * )*(( int * )a);   //pointer to next in free list.
	}

	return a;
}


static void freeMe(int curr, int prev)
{

	checkTimerCallbacks();

	deleteNode( curr, prev );
	checkTimerCallbacks();

	freeMemNode( &timers[ curr ].mp, timers[ curr ].data );

	checkTimerCallbacks();
}


//
//
//
static void initTimerCallbacks(void)
{
	int i;

	initialized = TRUE;

	for( i = 0; i < ( MAX_TIMERS - 1 ); i++ )
		timers[ i ].next = i + 1;

	timers[ MAX_TIMERS - 1 ].next = -1;   //end of list.
	first_free_obj                      = 0;    //point to start of free list
	first_game_obj                      = -1;   //none in use.

	initAMemPool( &alter_work_mem_pool, sizeof( AlterWork ), MAX_ALTER_WORK, ( void * )&alter_objs );
	checkTimerCallbacks();

}


//
//
//
static int findMyPrevLink(int me, int start)
{
	int prev = -1, curr;

	curr = start;

	while( me != curr )
	{
		prev = curr;
		curr = timers[ curr ].next;
	}

	return prev;
}


//
//
//
static void deleteNode(int curr, int prev)
{
	checkTimerCallbacks();

	if( prev == -1 )
	{
		prev = findMyPrevLink( curr, first_game_obj );
		if( prev == -1 )
			first_game_obj = timers[ curr ].next;
		else
		{
			timers[prev].next = timers[curr].next;
		}
	}
	else timers[ prev ].next = timers[ curr ].next;

	timers[ curr ].next  = first_free_obj;
	first_free_obj          = curr;

	checkTimerCallbacks();
}

//
//
//
static int inFreeList( int idx )
{
	int i, cnt = 0;

	i = first_free_obj;

	while( cnt < MAX_TIMERS + 1 && i != -1 )
	{
		if( i == idx )
			return TRUE;
		i = timers[ i ].next;
	}

	if( cnt >= MAX_TIMERS + 1 )
	{
		printf("Free list is corrupt.\n");
	}

	return FALSE;
}

//
//
//
static void checkTimerCallbacks(void)
{
	int i, cnt = 0, failed = FALSE;

	i = first_game_obj;

	while( i != -1 && cnt < MAX_TIMERS + 1 && !failed )
	{
		if( inFreeList( i ) )
			failed = TRUE;
		else
		{
			cnt++;
			i = timers[ i ].next;
		}
	}

	if( failed || cnt >= MAX_TIMERS + 1 )
	{
		printf("Game obj list corrupted.\n");
	}
}

//
//
//
TimerCallback *timerCreateCallback(int (* code )( void *go ), MemPool *mem)
{
	int     next_free, save;

	CHECK_INIT

	checkTimerCallbacks();

	next_free = timers[ first_free_obj ].next;

	if( next_free != -1 )
	{
		void    *data;

		if( code == 0 )
		{
			int  halt;

			halt = 1;
		}

		if( data = getMemNode( mem ) )
		{
			timers[ first_free_obj ].next    = first_game_obj;
			first_game_obj                      = first_free_obj;
			save                                = first_free_obj;
			first_free_obj                      = next_free;

			timers[ save ].mp                = mem;
			timers[ save ].data              = data;
			timers[ save ].code              = code;
			timers[ save ].timer             = 0;    //amount of ticks to elapse before calling.

			checkTimerCallbacks();

			return &timers[ save ];
		}
	}

	return ( TimerCallback * )NULL;
}

//
//
//
void timerServeCallbacks(void)
{
	int curr, prev = -1;
	int cnt = 0;    //for debug only

	CHECK_INIT

	curr = first_game_obj;

	checkTimerCallbacks();

	while( curr != -1 )
	{
		int next = timers[ curr ].next;

		timers[ curr ].timer -= global_state.frame_time_30;

		if( timers[ curr ].timer <= 0 )
		{
			if( !timers[ curr ].code( &timers[ curr ] ) )
			{
				checkTimerCallbacks();
				freeMe( curr, prev );
				checkTimerCallbacks();
			}
			else prev = curr;

			cnt++;
		}
		else prev = curr;

		curr = next;
	}
}

/* End of File */
