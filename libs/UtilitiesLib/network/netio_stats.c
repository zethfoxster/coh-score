#include "netio_stats.h"
#include "netio_core.h"
#include "network\net_link.h"
#include <assert.h>
#include <stdio.h>
#include <time.h>
#include "timing.h"
#include "mathutil.h"

/**********************************************************************************************
 * Transfer rate tracking
 */
/* Function pktRate()
 *	Given some packet history, this function calculates how many bytes per second
 *	has been transferred for the past 128 packets.
 *
 */
int pktRate(PktHist* hist){
	F32		time, rate = 0;
	int		i;
	int		curTime;


	netioEnterCritical();
	// Only calculate the sum of all the bytes sent and all the time deltas when
	// necessary.
	if(!hist->lastTimeSum || !hist->lastByteSum){
		// Consider all history records.
		for(i=0;i<MAX_PKTHIST;i++)
		{
			// Count how many bytes has been transferred.
			hist->lastByteSum += hist->bytes[i];
			
			// Count how much time has elapsed.
			hist->lastTimeSum += hist->times[i];
		}
	}

	curTime = timerCpuTicks();
	time = hist->lastTimeSum + timerSeconds(curTime - hist->last_time);

	// If there is any history at all...
	if(hist->lastByteSum && time)
		// Calculate the throughput in bytes per second.
		rate =  hist->lastByteSum / time;

	netioLeaveCritical();
	return (int)rate;
}

/* Function pktAddHist()
 *	Add a new history entry into the given packet history.
 *
 *	Packet History makes use of a circular array.  Whenever a new entry
 *	is inserted into the array, the oldest entry will be removed.
 *
 */
void pktAddHist(PktHist* hist, int bytes, int print_stats){
	U32		curr_time;

	// Make sure that the index is pointing at a valid entry.
	assert(hist->idx >= 0 || hist->idx < MAX_PKTHIST);

	// Get the index of the new entry.
	// Make sure the index does not go out of bounds.
	if (++hist->idx >= MAX_PKTHIST)
		hist->idx = 0;

	// We're about to wipe out an existing entry in the circular array.
	// Update the running sums first, before the entry is wiped.
	hist->lastByteSum -= hist->bytes[hist->idx];
	hist->lastTimeSum -= hist->times[hist->idx];

	curr_time = timerCpuTicks();

	// Record how much time has elapsed since the last time an entry
	// was added.  This equates to the time since the last time
	// a packet was sent.
	if (hist->last_time == 0)
		hist->times[hist->idx] = 0.1;
	else
		hist->times[hist->idx] = timerSeconds(curr_time - hist->last_time);

	if(print_stats){
		/*static int lastTime = -1;
		static int count = 0;
		int curTime = time(NULL) % 10;
		
		if(curTime != lastTime){
			lastTime = curTime;
			count = 0;
		}else{
			count++;
		}
		
		printf("(%d:%2.2d) added to packet history 0x%8.8x: %d bytes @ %1.3f seconds - %1.3f/%1.3f\n", curTime, count, hist, bytes, timerSeconds(curr_time - hist->last_time), hist->lastByteSum, hist->lastTimeSum);
		*/
	}

	// Record how much data was sent.
	hist->bytes[hist->idx] = (F32)bytes;
	hist->last_time = curr_time;

	// Update the running sums with the new values.
	hist->lastByteSum += hist->bytes[hist->idx];
	hist->lastTimeSum += hist->times[hist->idx];

	assert(hist->idx >= 0 || hist->idx < MAX_PKTHIST);
}
/*
 * Transfer rate tracking
 **********************************************************************************************/

/**********************************************************************************************
 * Ping time tracking
 */
int pingAvgRate(PingHistory* history){
	if(!history->size)
		return 0;

	return history->lastPingSum / history->size;
}

int pingInstantRate(PingHistory* history){
	if(!history->size)
		return 0;

	return history->lastPing;
}

void pingAddHistInternal(PingHistory* history, int ping)
{
	// Make sure that the index is pointing at a valid entry.
	assert(history->nextEntry >= 0 || history->nextEntry < MAX_PINGHIST);

	// We're about to wipe out an existing entry in the circular array.
	// Update the running sums first, before the entry is wiped.
	history->lastPingSum -= history->pings[history->nextEntry];

	// Record the new ping time.
	history->lastPing = ping;
	history->pings[history->nextEntry] = ping;

	// Update the running sums with the new values.
	history->lastPingSum += history->pings[history->nextEntry];

	if(history->size < MAX_PINGHIST)
		history->size++;

	// Get the index of the new entry.
	// Make sure the index does not go out of bounds.
	if(++history->nextEntry >= MAX_PINGHIST)
		history->nextEntry = 0;

	assert(history->nextEntry >= 0 || history->nextEntry < MAX_PKTHIST);
}

void pingBatchBegin(PingHistory* history)
{
	history->inBatch = 1;
	history->lastPing = 0;
}

void pingBatchEnd(PingHistory* history)
{
	if (!history->inBatch)
		return;
	if (history->lastPing == 0) {
		// Nothing recorded
		if (history->size) {
			// Restore to the most recent recorded ping
			history->lastPing = history->pings[(history->nextEntry + MAX_PINGHIST - 1) % MAX_PINGHIST];
		}
	} else {
		// The minimum ping from the batch is recorded
		pingAddHistInternal(history, history->lastPing);
	}
	history->inBatch = 0;
}


void pingAddHist(PingHistory* history, int ping){
	if (history->inBatch) {
		// We're recording a batch
		if (history->lastPing == 0) {
			history->lastPing = ping;
		} else {
			history->lastPing = MIN(history->lastPing, ping);
		}
	} else {
		pingAddHistInternal(history, ping);
	}
}



/************************************************************************************
 * Packet send time record
 *	Tracks the send times of recently sent unreliable packets so that their ping
 *	times can be calculated properly.
 *
 *	Reliable packets do not need to be tracked since the entire packet, including
 *	its send time is kept until proper delivery can be made.
 */
void lnkAddSendTime(NetLink* link, int pakID, int sendTime){
	int index;	// This is where the new element is going to go.

	// If the send time array is full, just randomly pick an sendtime to be dropped.
	if(link->pakSendTimesSize == MAX_SENDTIMESSIZE){
		index = randInt(MAX_SENDTIMESSIZE);
	}else{
		index = link->pakSendTimesSize;
		link->pakSendTimesSize++;
	}

	link->pakSendTimes[index].pakID = pakID;
	link->pakSendTimes[index].sendTime = sendTime;
}

int lnkGetSendTime(NetLink* link, int pakID){
	int i;
	unsigned int sendTime = 0;

	for(i = 0; i < link->pakSendTimesSize; i++){
		if(link->pakSendTimes[i].pakID == (unsigned int)pakID){
			sendTime = link->pakSendTimes[i].sendTime;
			break;
		}
	}

	// Did we find a send time record with the matching given packet ID?
	if(sendTime && link->pakSendTimesSize){
		// If so, get rid of the record.  We're never going to need it again.
		// Ping for a packet is only ever calculated once.

		link->pakSendTimes[i] = link->pakSendTimes[link->pakSendTimesSize - 1];
		link->pakSendTimesSize--;
	}

	return sendTime;
}
/*
 *
 ************************************************************************************/

/*
 * Ping time tracking
 **********************************************************************************************/
