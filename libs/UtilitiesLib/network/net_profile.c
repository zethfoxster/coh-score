#define MEASURE_NETWORK_TRAFFIC_OPT_OUT
#include "net_packet.h"
#include "net_packetutil.h"
#include "StashTable.h"
#include "file.h"
#include "net_structdefs.h"
#include "timing.h"


typedef struct ProfileRecord {
	char fn[MAX_PATH+20];
	int numbits;
	__int64 count;
	__int64 zerocount;
	__int64 sum;
	__int64 smallneg;
	__int64 log2sum;
	__int64 abslog2sum;
} ProfileRecord;

StashTable htSendBits = 0;
StashTable htSendBitsPack = 0;
StashTable htSendIfSetBits = 0;
StashTable htSendIfSetBitsPack = 0;
StashTable htSendString = 0;
StashTable htSendF32 = 0;
StashTable htSetCompression = 0;
StashTable htSendBitsArray = 0;
StashTable htSendStringAligned = 0;
StashTable htAlignBitsArray = 0;
StashTable htAppend = 0;
StashTable htAppendRemainder = 0;
StashTable htMakeCompatible = 0;
StashTable htSendIfSetF32 = 0;
StashTable htSendBitsAuto = 0;
StashTable htSendBitsAuto2 = 0;
StashTable htSendBit = 0;
StashTable htSendZippedAlready = 0;
StashTable htSendZipped = 0;
StashTable htSendGetZipped = 0;
StashTable htSendGetBitsAuto = 0;
StashTable htSendGetStringAligned = 0;
StashTable htSendGetBitsPak = 0;
StashTable htSendGetBitsArray = 0;
StashTable htSendGetF32 = 0;
StashTable htSendGetStringNLength = 0;
StashTable htSendGetBits = 0;
StashTable htSendColor = 0;
StashTable htSendVec3 = 0;
StashTable htSendMat4 = 0;
StashTable htSendVec4 = 0;
StashTable htSendF = 0;
StashTable htSendIfSetString = 0;
StashTable htSendStringf = 0;
StashTable htSendStringfv = 0;
StashTable htSendIndexedString = 0;
StashTable htSendIndexedGeneric = 0;
StashTable htSendF32Comp = 0;
StashTable htSendF32Deg = 0;
StashTable htSendMat4Ex = 0;
static FILE *dumpfile=NULL;

static int s_collectingNetStats = 0;
static int dumpStatsProcessor(StashElement elem)
{
	ProfileRecord *pr = (ProfileRecord*)stashElementGetPointer(elem);

	fprintf(dumpfile, "%s\t%d\t%I64d\t%I64d\t%I64d\t%I64d\t%I64d\t%I64d\n", pr->fn, pr->numbits, pr->count, pr->zerocount, pr->sum, pr->smallneg, pr->log2sum, pr->abslog2sum);

	// JP: Commenting out because I'm not going to looking at the screen for the numbers.
#if 0
	{
		// on-screen stats
		F32 avg = (F32)pr->sum/(F32)pr->count;
		if (strlen(pr->fn) > 39) {
			printf("Location: ...%37s  ", pr->fn + strlen(pr->fn) - 37);
		} else {
			printf("Location: %40s  ", pr->fn);
		}
		printf("Calls: %8I64d ", pr->count);
		printf("Zeroes: %8I64d ", pr->zerocount);
		printf("Avg:%6.2f ", avg);
		if (pr->count == pr->zerocount) {
			avg = 0;
		} else {
			avg = (F32)pr->sum/(F32)(pr->count - pr->zerocount);
		}
		printf("Anz:%6.2f ", avg);
		printf("numbits: %2d", pr->numbits);
		printf("\n");
	}
#endif

	return 1;
}

static void addStats(StashTable *ht, char *fn, int line, int default_num_bits, int bits, int is_zero, int is_small_neg, int log2val, int abslog2val)
{
	char key[MAX_PATH+20];
	ProfileRecord *pr;
	if (*ht==0)
		*ht = stashTableCreateWithStringKeys(100, StashDeepCopyKeys | StashCaseSensitive );

	sprintf_s(SAFESTR(key), "%s (%d)", fn, line);
	stashFindPointer( *ht, key, &pr );
	if (!pr) {
		pr = calloc(sizeof(ProfileRecord), 1);
		strcpy(pr->fn, key);
		pr->numbits = default_num_bits;
		stashAddPointer(*ht, key, pr, false);
	}
	pr->count++;
	pr->sum+=bits;
	if (is_zero)
		pr->zerocount++;
	if (is_small_neg)
		pr->smallneg++;
	pr->log2sum+=log2val;
	pr->abslog2sum+=abslog2val;
}

static void dumpStashTable(StashTable ht, const char *funcName)
{
	if (ht)
	{
		//printf("***%s\n", funcName);
		stashForEachElement(ht, dumpStatsProcessor);
	}
}

void netprofile_dumpStats(void)
{
	if (!htSendBits)
		return;
	mkdir("c:/temp");
	dumpfile = fopen("c:/temp/pktSendStats.txt", "w");
	if (!dumpfile)
		return;
	dumpStashTable(htSendBits, "htSendBits");
	dumpStashTable(htSendBitsPack, "htSendBitsPack");
	dumpStashTable(htSendIfSetBits, "htSendIfSetBits");
	dumpStashTable(htSendIfSetBitsPack, "htSendIfSetBitsPack");
	dumpStashTable(htSendString, "htSendString");
	dumpStashTable(htSendF32, "htSendF32");
	dumpStashTable(htSetCompression, "htSetCompression");
	dumpStashTable(htSendBitsArray, "htSendBitsArray");
	dumpStashTable(htSendStringAligned, "htSendStringAligned");
	dumpStashTable(htAlignBitsArray, "htAlignBitsArray");
	dumpStashTable(htAppend, "htAppend");
	dumpStashTable(htAppendRemainder, "htAppendRemainder");
	dumpStashTable(htMakeCompatible, "htMakeCompatible");
	dumpStashTable(htSendIfSetF32, "htSendIfSetF32");
	dumpStashTable(htSendBitsAuto, "htSendBitsAuto");
	dumpStashTable(htSendBitsAuto2, "htSendBitsAuto2");
	dumpStashTable(htSendBit, "htSendBit");
	dumpStashTable(htSendZippedAlready, "htSendZippedAlready");
	dumpStashTable(htSendZipped, "htSendZipped");
	dumpStashTable(htSendGetZipped, "htSendGetZipped");
	dumpStashTable(htSendGetBitsAuto, "htSendGetBitsAuto");
	dumpStashTable(htSendGetStringAligned, "htSendGetStringAligned");
	dumpStashTable(htSendGetBitsPak, "htSendGetBitsPak");
	dumpStashTable(htSendGetBitsArray, "htSendGetBitsArray");
	dumpStashTable(htSendGetF32, "htSendGetF32");
	dumpStashTable(htSendGetStringNLength, "htSendGetStringNLength");
	dumpStashTable(htSendGetBits, "htSendGetBits");
	dumpStashTable(htSendColor, "htSendColor");
	dumpStashTable(htSendVec3, "htSendVec3");
	dumpStashTable(htSendMat4, "htSendMat4");
	dumpStashTable(htSendVec4, "htSendVec4");
	dumpStashTable(htSendF, "htSendF");
	dumpStashTable(htSendIfSetString, "htSendIfSetString");
	dumpStashTable(htSendStringf, "htSendStringf");
	dumpStashTable(htSendStringfv, "htSendStringfv");
	dumpStashTable(htSendIndexedString, "htSendIndexedString");
	dumpStashTable(htSendIndexedGeneric, "htSendIndexedGeneric");
	dumpStashTable(htSendF32Comp, "htSendF32Comp");
	dumpStashTable(htSendF32Deg, "htSendF32Deg");
	dumpStashTable(htSendMat4Ex, "htSendMat4Ex");
	fclose(dumpfile);
}

#define stashTableDestroyIfNotNull(x) if(x) stashTableDestroy(x)
void netprofile_resetStats(void)
{
	stashTableDestroyIfNotNull(htSendBits);
	stashTableDestroyIfNotNull(htSendBitsPack);
	stashTableDestroyIfNotNull(htSendIfSetBits);
	stashTableDestroyIfNotNull(htSendIfSetBitsPack);
	stashTableDestroyIfNotNull(htSendString);
	stashTableDestroyIfNotNull(htSendF32);
	stashTableDestroyIfNotNull(htSetCompression);
	stashTableDestroyIfNotNull(htSendBitsArray);
	stashTableDestroyIfNotNull(htSendStringAligned);
	stashTableDestroyIfNotNull(htAlignBitsArray);
	stashTableDestroyIfNotNull(htAppend);
	stashTableDestroyIfNotNull(htAppendRemainder);
	stashTableDestroyIfNotNull(htMakeCompatible);
	stashTableDestroyIfNotNull(htSendIfSetF32);
	stashTableDestroyIfNotNull(htSendBitsAuto);
	stashTableDestroyIfNotNull(htSendBitsAuto2);
	stashTableDestroyIfNotNull(htSendBit);
	stashTableDestroyIfNotNull(htSendZippedAlready);
	stashTableDestroyIfNotNull(htSendZipped);
	stashTableDestroyIfNotNull(htSendGetZipped);
	stashTableDestroyIfNotNull(htSendGetBitsAuto);
	stashTableDestroyIfNotNull(htSendGetStringAligned);
	stashTableDestroyIfNotNull(htSendGetBitsPak);
	stashTableDestroyIfNotNull(htSendGetBitsArray);
	stashTableDestroyIfNotNull(htSendGetF32);
	stashTableDestroyIfNotNull(htSendGetStringNLength);
	stashTableDestroyIfNotNull(htSendGetBits);
	stashTableDestroyIfNotNull(htSendColor);
	stashTableDestroyIfNotNull(htSendVec3);
	stashTableDestroyIfNotNull(htSendMat4);
	stashTableDestroyIfNotNull(htSendVec4);
	stashTableDestroyIfNotNull(htSendF);
	stashTableDestroyIfNotNull(htSendIfSetString);
	stashTableDestroyIfNotNull(htSendStringf);
	stashTableDestroyIfNotNull(htSendStringfv);
	stashTableDestroyIfNotNull(htSendIndexedString);
	stashTableDestroyIfNotNull(htSendIndexedGeneric);
	stashTableDestroyIfNotNull(htSendF32Comp);
	stashTableDestroyIfNotNull(htSendF32Deg);
	stashTableDestroyIfNotNull(htSendMat4Ex);
	htSendBits = 0;
	htSendBitsPack = 0;
	htSendIfSetBits = 0;
	htSendIfSetBitsPack = 0;
	htSendString = 0;
	htSendF32 = 0;
	htSetCompression = 0;
	htSendBitsArray = 0;
	htSendStringAligned = 0;
	htAlignBitsArray = 0;
	htAppend = 0;
	htAppendRemainder = 0;
	htMakeCompatible = 0;
	htSendIfSetF32 = 0;
	htSendBitsAuto = 0;
	htSendBitsAuto2 = 0;
	htSendBit = 0;
	htSendZippedAlready = 0;
	htSendZipped = 0;
	htSendGetZipped = 0;
	htSendGetBitsAuto = 0;
	htSendGetStringAligned = 0;
	htSendGetBitsPak = 0;
	htSendGetBitsArray = 0;
	htSendGetF32 = 0;
	htSendGetStringNLength = 0;
	htSendGetBits = 0;
	htSendColor = 0;
	htSendVec3 = 0;
	htSendMat4 = 0;
	htSendVec4 = 0;
	htSendF = 0;
	htSendIfSetString = 0;
	htSendStringf = 0;
	htSendStringfv = 0;
	htSendIndexedString = 0;
	htSendIndexedGeneric = 0;
	htSendF32Comp = 0;
	htSendF32Deg = 0;
	htSendMat4Ex = 0;
}

static unsigned int s_lastTime = 0;
static int s_packetDebugOn;
void netprofile_setTimer(unsigned int timeToCollect)
{
	s_collectingNetStats = timeToCollect;

	s_lastTime = timerSecondsSince2000();
	s_packetDebugOn = pktGetDebugInfo();
	pktSetDebugInfoTo(0);
}
void netprofile_decrementTimer()
{
	if (s_collectingNetStats)
	{
		if (timerSecondsSince2000() != s_lastTime)
		{
			int newTime = timerSecondsSince2000();
			s_collectingNetStats -= (newTime - s_lastTime);
			s_lastTime = newTime;
			//	timer expired
			if (s_collectingNetStats <= 0)
			{
				s_collectingNetStats = 0;
				netprofile_dumpStats();
				pktSetDebugInfoTo(s_packetDebugOn);
				if (s_packetDebugOn)
					Errorf("Packet debugging re-enabled.");
			}
		}
	}
}
static int checkSmallNeg(U32 val)
{
	U32 negValue = (~val) + 1U;
	return negValue > 0 && negValue < 65536;
}

static int getAbsLog2Val(U32 val)
{
	return min(count_bits((~val) + 1U), count_bits(val));
}

void profile_pktSendBits(char *file, int line, Packet *pak, int numbits, U32 val)
{
	if (s_collectingNetStats)
	{
		int startPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		int endPosition;

		pktSendBits(pak, numbits, val);

		endPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		devassert(!pak->hasDebugInfo);
		if (!pak->hasDebugInfo)

			addStats(&htSendBits, file, line, numbits, endPosition - startPosition, !val, checkSmallNeg(val), count_bits(val), getAbsLog2Val(val));
	}
	else
	{
		pktSendBits(pak, numbits, val);
	}
}

void profile_pktSendBitsPack(char *file, int line, Packet *pak, int minbits, U32 val)
{
	if (s_collectingNetStats)
	{
		int startPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		int endPosition;

		pktSendBitsPack(pak, minbits, val);

		endPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		devassert(!pak->hasDebugInfo);
		if (!pak->hasDebugInfo)
			addStats(&htSendBitsPack, file, line, minbits, endPosition - startPosition, !val, checkSmallNeg(val), count_bits(val), getAbsLog2Val(val));
	}
	else
	{
		pktSendBitsPack(pak, minbits, val);
	}
}

void profile_pktSendString(char *file, int line, Packet *pak, const char *str)
{
	if (s_collectingNetStats)
	{
		int startPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		int endPosition;

		pktSendString(pak, str);

		endPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		devassert(!pak->hasDebugInfo);
		if (!pak->hasDebugInfo)
			addStats(&htSendString, file, line, 0, endPosition - startPosition, !str || !*str, 0, 0, 0);
	}
	else
	{
		pktSendString(pak, str);
	}
}

void profile_pktSendF32(char *file, int line, Packet *pak, F32 f)
{
	if (s_collectingNetStats)
	{
		int startPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		int endPosition;

		pktSendF32(pak, f);

		endPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		devassert(!pak->hasDebugInfo);
		if (!pak->hasDebugInfo)
			addStats(&htSendF32, file, line, 32, endPosition - startPosition, f == 0.f, 0, 0, 0);
	}
	else
	{
		pktSendF32(pak, f);
	}
}

void profile_pktSendIfSetBits(char *file, int line, Packet *pak, int numbits, int val)
{
	if (s_collectingNetStats)
	{
		int startPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		int endPosition;

		pktSendIfSetBits(pak, numbits, val);

		endPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		devassert(!pak->hasDebugInfo);
		if (!pak->hasDebugInfo)
			addStats(&htSendIfSetBits, file, line, numbits, endPosition - startPosition, !val, checkSmallNeg(val), count_bits(val), getAbsLog2Val(val));
	}
	else
	{
		pktSendIfSetBits(pak, numbits, val);
	}

}

void profile_pktSendIfSetBitsPack(char *file, int line, Packet *pak,int minbits,int val)
{
	if (s_collectingNetStats)
	{
		int startPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		int endPosition;

		pktSendIfSetBitsPack(pak, minbits, val);

		endPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		devassert(!pak->hasDebugInfo);
		if (!pak->hasDebugInfo)
			addStats(&htSendIfSetBitsPack, file, line, minbits, endPosition - startPosition, !val, checkSmallNeg(val), count_bits(val), getAbsLog2Val(val));
	}
	else
	{
		pktSendIfSetBitsPack(pak, minbits, val);
	}

}

void profile_pktSetCompression(char *file, int line, Packet* pak, int compression)
{
	if (s_collectingNetStats)
	{
		int startPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		int endPosition;

		pktSetCompression(pak, compression);

		endPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		devassert(!pak->hasDebugInfo || !pak->link);
		if (!pak->hasDebugInfo || !pak->link)
			addStats(&htSetCompression, file, line, 0, endPosition - startPosition, 0, 0, 0, 0);
	}
	else
	{
		pktSetCompression(pak, compression);
	}

}

void profile_pktSendBitsArray(char *file, int line, Packet *pak, int numbits, const void *data)
{
	if (s_collectingNetStats)
	{
		int startPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		int endPosition;

		pktSendBitsArray(pak, numbits, data);

		endPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		devassert(!pak->hasDebugInfo);
		if (!pak->hasDebugInfo)
			addStats(&htSendBitsArray, file, line, numbits, endPosition - startPosition, 0, 0, 0, 0);
	}
	else
	{
		pktSendBitsArray(pak, numbits, data);
	}

}

void profile_pktSendStringAligned(char *file, int line, Packet *pak, const char *str)
{
	if (s_collectingNetStats)
	{
		int startPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		int endPosition;

		pktSendStringAligned(pak, str);

		endPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		devassert(!pak->hasDebugInfo);
		if (!pak->hasDebugInfo)
			addStats(&htSendStringAligned, file, line, 0, endPosition - startPosition, !str || !*str, 0, 0, 0);
	}
	else
	{
		pktSendStringAligned(pak, str);
	}

}

void profile_pktAlignBitsArray(char *file, int line, Packet *pak)
{
	if (s_collectingNetStats)
	{
		int startPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		int endPosition;

		pktAlignBitsArray(pak);

		endPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		devassert(!pak->hasDebugInfo || !pak->link);
		if (!pak->hasDebugInfo || !pak->link)
			addStats(&htAlignBitsArray, file, line, 0, endPosition - startPosition, 0, 0, 0, 0);
	}
	else
	{
		pktAlignBitsArray(pak);
	}

}

void profile_pktAppend(char *file, int line, Packet* pak, Packet* data)
{
	if (s_collectingNetStats)
	{
		int startPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		int endPosition;

		pktAppend(pak, data);

		endPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		devassert(!pak->hasDebugInfo);
		if (!pak->hasDebugInfo)
			addStats(&htAppend, file, line, 0, endPosition - startPosition, 0, 0, 0, 0);
	}
	else
	{
		pktAppend(pak, data);
	}

}

void profile_pktAppendRemainder(char *file, int line, Packet* pak, Packet* data)
{
	if (s_collectingNetStats)
	{
		int startPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		int endPosition;

		pktAppendRemainder(pak, data);

		endPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		devassert(!pak->hasDebugInfo);
		if (!pak->hasDebugInfo)
			addStats(&htAppendRemainder, file, line, 0, endPosition - startPosition, 0, 0, 0, 0);
	}
	else
	{
		pktAppendRemainder(pak, data);
	}

}

void profile_pktSendIfSetF32(char *file, int line, Packet *pak,float f)
{
	if (s_collectingNetStats)
	{
		int startPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		int endPosition;

		pktSendIfSetF32(pak, f);

		endPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		devassert(!pak->hasDebugInfo);
		if (!pak->hasDebugInfo)
			addStats(&htSendIfSetF32, file, line, 0, endPosition - startPosition, f == 0.f, 0, 0, 0);
	}
	else
	{
		pktSendIfSetF32(pak, f);
	}

}

void profile_pktSendBitsAuto(char *file, int line, Packet *pak, U32 val)
{
	if (s_collectingNetStats)
	{
		int startPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		int endPosition;

		pktSendBitsAuto(pak, val);

		endPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		devassert(!pak->hasDebugInfo);
		if (!pak->hasDebugInfo)
			addStats(&htSendBitsAuto, file, line, 0, endPosition - startPosition, !val, checkSmallNeg(val), count_bits(val), getAbsLog2Val(val));
	}
	else
	{
		pktSendBitsAuto(pak, val);
	}

}

void profile_pktSendBitsAuto2(char *file, int line, Packet *pak, U64 val)
{
	if (s_collectingNetStats)
	{
		int startPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		int endPosition;
		U32 low = ((U32*)&val)[0];
		U32 high = ((U32*)&val)[1];

		pktSendBitsAuto2(pak, val);

		endPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		devassert(!pak->hasDebugInfo);
		if (!pak->hasDebugInfo)
			addStats(&htSendBitsAuto2, file, line, 0, endPosition - startPosition, !val, high == UINT_MAX && checkSmallNeg(low), count_bits(low) + count_bits(high), getAbsLog2Val(low) + getAbsLog2Val(high));
	}
	else
	{
		pktSendBitsAuto2(pak, val);
	}

}

int profile_pktSendBit(char *file, int line, Packet *pak, int val)
{
	if (s_collectingNetStats)
	{
		int startPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		int endPosition;

		int ret = pktSendBit(pak, val);

		endPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		devassert(!pak->hasDebugInfo);
		if (!pak->hasDebugInfo)
			addStats(&htSendBit, file, line, 1, endPosition - startPosition, !val, 0, 0, 0);

		return ret;
	}
	else
	{
		return pktSendBit(pak, val);
	}

}

void profile_pktSendZippedAlready(char *file, int line, Packet *pak, int numbytes, int zipbytes, void *zipdata)
{
	if (s_collectingNetStats)
	{
		int startPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		int endPosition;

		pktSendZippedAlready(pak, numbytes, zipbytes, zipdata);

		endPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		devassert(!pak->hasDebugInfo);
		if (!pak->hasDebugInfo)
			addStats(&htSendZippedAlready, file, line, zipbytes*8+2, endPosition - startPosition, 0, 0, 0, 0);
	}
	else
	{
		pktSendZippedAlready(pak, numbytes, zipbytes, zipdata);
	}

}

void profile_pktSendZipped(char *file, int line, Packet *pak, int numbytes, void *data)
{
	if (s_collectingNetStats)
	{
		int startPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		int endPosition;

		pktSendZipped(pak, numbytes, data);

		endPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		devassert(!pak->hasDebugInfo);
		if (!pak->hasDebugInfo)
			addStats(&htSendZipped, file, line, numbytes, endPosition - startPosition, 0, 0, 0, 0);
	}
	else
	{
		pktSendZipped(pak, numbytes, data);
	}

}

void profile_pktSendGetZipped(char *file, int line, Packet *pak_out, Packet *pak_in)
{
	if (s_collectingNetStats)
	{
		int startPosition = pak_out->stream.cursor.byte * 8 + pak_out->stream.cursor.bit;
		int endPosition;

		pktSendGetZipped(pak_out, pak_in);

		endPosition = pak_out->stream.cursor.byte * 8 + pak_out->stream.cursor.bit;
		devassert(!pak_out->hasDebugInfo);
		addStats(&htSendGetZipped, file, line, 0, endPosition - startPosition, 0, 0, 0, 0);
	}
	else
	{
		pktSendGetZipped(pak_out, pak_in);
	}

}

int profile_pktSendGetBitsAuto(char *file, int line, Packet *pak_out, Packet *pak_in)
{
	if (s_collectingNetStats)
	{
		int startPosition = pak_out->stream.cursor.byte * 8 + pak_out->stream.cursor.bit;
		int endPosition;

		int ret = pktSendGetBitsAuto(pak_out, pak_in);

		endPosition = pak_out->stream.cursor.byte * 8 + pak_out->stream.cursor.bit;
		devassert(!pak_out->hasDebugInfo);
		addStats(&htSendGetBitsAuto, file, line, 0, endPosition - startPosition, !ret, checkSmallNeg(ret), count_bits(ret), getAbsLog2Val(ret));

		return ret;
	}
	else
	{
		return pktSendGetBitsAuto(pak_out, pak_in);
	}

}

long long profile_pktSendGetBitsAuto2(char *file, int line, Packet *pak_out, Packet *pak_in)
{
	if (s_collectingNetStats)
	{
		int startPosition = pak_out->stream.cursor.byte * 8 + pak_out->stream.cursor.bit;
		int endPosition;

		long long ret = pktSendGetBitsAuto2(pak_out, pak_in);
		U32 low = ((U32*)&ret)[0];
		U32 high = ((U32*)&ret)[1];

		endPosition = pak_out->stream.cursor.byte * 8 + pak_out->stream.cursor.bit;
		assert(!pak_out->hasDebugInfo);
		addStats(&htSendBitsAuto2, file, line, 0, endPosition - startPosition, !ret, high == UINT_MAX && checkSmallNeg(low), count_bits(low) + count_bits(high), getAbsLog2Val(low) + getAbsLog2Val(high));

		return ret;
	}
	else
	{
		return pktSendGetBitsAuto2(pak_out, pak_in);
	}
}

const char *profile_pktSendGetStringAligned(char *file, int line, Packet *pak_out, Packet *pak_in)
{
	if (s_collectingNetStats)
	{
		int startPosition = pak_out->stream.cursor.byte * 8 + pak_out->stream.cursor.bit;
		int endPosition;

		const char *ret = pktSendGetStringAligned(pak_out, pak_in);

		endPosition = pak_out->stream.cursor.byte * 8 + pak_out->stream.cursor.bit;
		devassert(!pak_out->hasDebugInfo);
		addStats(&htSendGetStringAligned, file, line, 0, endPosition - startPosition, !ret || !*ret, 0, 0, 0);

		return ret;
	}
	else
	{
		return pktSendGetStringAligned(pak_out, pak_in);
	}

}

int profile_pktSendGetBitsPak(char *file, int line, Packet *pak_out, Packet *pak_in, int minbits)
{
	if (s_collectingNetStats)
	{
		int startPosition = pak_out->stream.cursor.byte * 8 + pak_out->stream.cursor.bit;
		int endPosition;

		int ret = pktSendGetBitsPak(pak_out, pak_in, minbits);

		endPosition = pak_out->stream.cursor.byte * 8 + pak_out->stream.cursor.bit;
		devassert(!pak_out->hasDebugInfo);
		addStats(&htSendGetBitsPak, file, line, minbits, endPosition - startPosition, !ret, checkSmallNeg(ret), count_bits(ret), getAbsLog2Val(ret));

		return ret;
	}
	else
	{
		return pktSendGetBitsPak(pak_out, pak_in, minbits);
	}

}

void profile_pktSendGetBitsArray(char *file, int line, Packet *pak_out, Packet *pak_in, int numbits, void *data)
{
	if (s_collectingNetStats)
	{
		int startPosition = pak_out->stream.cursor.byte * 8 + pak_out->stream.cursor.bit;
		int endPosition;

		pktSendGetBitsArray(pak_out, pak_in, numbits, data);

		endPosition = pak_out->stream.cursor.byte * 8 + pak_out->stream.cursor.bit;
		devassert(!pak_out->hasDebugInfo);
		addStats(&htSendGetBitsArray, file, line, numbits, endPosition - startPosition, 0, 0, 0, 0);
	}
	else
	{
		pktSendGetBitsArray(pak_out, pak_in, numbits, data);
	}

}

F32 profile_pktSendGetF32(char *file, int line, Packet *pak_out, Packet *pak_in)
{
	if (s_collectingNetStats)
	{
		int startPosition = pak_out->stream.cursor.byte * 8 + pak_out->stream.cursor.bit;
		int endPosition;

		F32 ret = pktSendGetF32(pak_out, pak_in);

		endPosition = pak_out->stream.cursor.byte * 8 + pak_out->stream.cursor.bit;
		devassert(!pak_out->hasDebugInfo);
		addStats(&htSendGetF32, file, line, 32, endPosition - startPosition, ret == 0.f, 0, 0, 0);

		return ret;
	}
	else
	{
		return pktSendGetF32(pak_out, pak_in);
	}

}

const char *profile_pktSendGetStringNLength(char *file, int line, Packet *pak_out, Packet *pak_in)
{
	if (s_collectingNetStats)
	{
		int startPosition = pak_out->stream.cursor.byte * 8 + pak_out->stream.cursor.bit;
		int endPosition;

		const char *ret = pktSendGetStringNLength(pak_out, pak_in);

		endPosition = pak_out->stream.cursor.byte * 8 + pak_out->stream.cursor.bit;
		devassert(!pak_out->hasDebugInfo);
		addStats(&htSendGetStringNLength, file, line, 0, endPosition - startPosition, !ret || !*ret, 0, 0, 0);

		return ret;
	}
	else
	{
		return pktSendGetStringNLength(pak_out, pak_in);
	}

}

U32 profile_pktSendGetBits(char *file, int line, Packet *pak_out, Packet *pak_in, int numbits)
{
	if (s_collectingNetStats)
	{
		int startPosition = pak_out->stream.cursor.byte * 8 + pak_out->stream.cursor.bit;
		int endPosition;

		U32 ret = pktSendGetBits(pak_out, pak_in, numbits);

		endPosition = pak_out->stream.cursor.byte * 8 + pak_out->stream.cursor.bit;
		devassert(!pak_out->hasDebugInfo);
		addStats(&htSendGetBits, file, line, numbits, endPosition - startPosition, !ret, checkSmallNeg(ret), count_bits(ret), getAbsLog2Val(ret));

		return ret;
	}
	else
	{
		return pktSendGetBits(pak_out, pak_in, numbits);
	}

}

void profile_pktSendColor(char *file, int line, Packet *pak, Color clr)
{
	if (s_collectingNetStats)
	{
		int startPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		int endPosition;

		pktSendColor(pak, clr);

		endPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		devassert(!pak->hasDebugInfo);
		if (!pak->hasDebugInfo)
			addStats(&htSendColor, file, line, 32, endPosition - startPosition, !clr.integer, 0, 0, 0);
	}
	else
	{
		pktSendColor(pak, clr);
	}

}

void profile_pktSendVec3(char *file, int line, Packet *pak, const Vec3 vec)
{
	if (s_collectingNetStats)
	{
		int startPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		int endPosition;

		pktSendVec3(pak, vec);

		endPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		devassert(!pak->hasDebugInfo);
		if (!pak->hasDebugInfo)
			addStats(&htSendVec3, file, line, 0, endPosition - startPosition, 0, 0, 0, 0);
	}
	else
	{
		pktSendVec3(pak, vec);
	}

}

void profile_pktSendMat4(char *file, int line, Packet *pak, const Mat4 mat)
{
	if (s_collectingNetStats)
	{
		int startPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		int endPosition;

		pktSendMat4(pak, mat);

		endPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		devassert(!pak->hasDebugInfo);
		if (!pak->hasDebugInfo)
			addStats(&htSendMat4, file, line, 0, endPosition - startPosition, 0, 0, 0, 0);
	}
	else
	{
		pktSendMat4(pak, mat);
	}

}

void profile_pktSendVec4(char *file, int line, Packet *pak, const Vec4 vec)
{
	if (s_collectingNetStats)
	{
		int startPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		int endPosition;

		pktSendVec4(pak, vec);

		endPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		devassert(!pak->hasDebugInfo);
		if (!pak->hasDebugInfo)
			addStats(&htSendVec4, file, line, 0, endPosition - startPosition, 0, 0, 0, 0);
	}
	else
	{
		pktSendVec4(pak, vec);
	}

}

void profile_pktSendF(char *file, int line, Packet *pak, int precision, F32 min, F32 max, F32 f)
{
	if (s_collectingNetStats)
	{
		int startPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		int endPosition;

		pktSendF(pak, precision, min, max, f);

		endPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		devassert(!pak->hasDebugInfo);
		if (!pak->hasDebugInfo)
			addStats(&htSendF, file, line, precision, endPosition - startPosition, f == 0.f, 0, 0, 0);
	}
	else
	{
		pktSendF(pak, precision, min, max, f);
	}

}

void profile_pktSendIfSetString(char *file, int line, Packet *pak, const char *str)
{
	if (s_collectingNetStats)
	{
		int startPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		int endPosition;

		pktSendIfSetString(pak, str);

		endPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		devassert(!pak->hasDebugInfo);
		if (!pak->hasDebugInfo)
			addStats(&htSendIfSetString, file, line, 0, endPosition - startPosition, !str || !*str, 0, 0, 0);
	}
	else
	{
		pktSendIfSetString(pak, str);
	}

}

void profile_pktSendStringf(char *file, int line, Packet *pak, FORMAT fmt, ...)
{
	if (s_collectingNetStats)
	{
		int startPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		int endPosition;
		va_list va;

		va_start(va, fmt);
		pktSendStringfv(pak, fmt, va);
		va_end(va);

		endPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		devassert(!pak->hasDebugInfo);
		if (!pak->hasDebugInfo)
			addStats(&htSendStringf, file, line, 0, endPosition - startPosition, 0, 0, 0, 0);
	}
	else
	{
		va_list va;

		va_start(va, fmt);
		pktSendStringfv(pak, fmt, va);
		va_end(va);
	}

}

void profile_pktSendStringfv(char *file, int line, Packet *pak, const char *fmt, va_list va)
{
	if (s_collectingNetStats)
	{
		int startPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		int endPosition;

		pktSendStringfv(pak, fmt, va);

		endPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		devassert(!pak->hasDebugInfo);
		if (!pak->hasDebugInfo)
			addStats(&htSendStringfv, file, line, 0, endPosition - startPosition, 0, 0, 0, 0);
	}
	else
	{
		pktSendStringfv(pak, fmt, va);
	}

}

void profile_pktSendIndexedString(char *file, int line, Packet *pak, const char *str, cStashTable htStrings)
{
	if (s_collectingNetStats)
	{
		int startPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		int endPosition;

		pktSendIndexedString(pak, str, htStrings);

		endPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		devassert(!pak->hasDebugInfo);
		if (!pak->hasDebugInfo)
			addStats(&htSendIndexedString, file, line, 0, endPosition - startPosition, !str || !*str, 0, 0, 0);
	}
	else
	{
		pktSendIndexedString(pak, str, htStrings);
	}

}

void profile_pktSendIndexedGeneric(char *file, int line, Packet *pak, int val, cStashTable ht)
{
	if (s_collectingNetStats)
	{
		int startPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		int endPosition;

		pktSendIndexedGeneric(pak, val, ht);

		endPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		devassert(!pak->hasDebugInfo);
		if (!pak->hasDebugInfo)
			addStats(&htSendIndexedGeneric, file, line, 0, endPosition - startPosition, !val, 0, 0, 0);
	}
	else
	{
		pktSendIndexedGeneric(pak, val, ht);
	}

}

void profile_pktSendF32Comp(char *file, int line, Packet *pak, int val)
{
	if (s_collectingNetStats)
	{
		int startPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		int endPosition;

		pktSendF32Comp(pak, val);

		endPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		devassert(!pak->hasDebugInfo);
		if (!pak->hasDebugInfo)
			addStats(&htSendF32Comp, file, line, 0, endPosition - startPosition, !val, 0, 0, 0);
	}
	else
	{
		pktSendF32Comp(pak, val);
	}

}

void profile_pktSendF32Deg(char *file, int line, Packet *pak, int val)
{
	if (s_collectingNetStats)
	{
		int startPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		int endPosition;

		pktSendF32Deg(pak, val);

		endPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		devassert(!pak->hasDebugInfo);
		if (!pak->hasDebugInfo)
			addStats(&htSendF32Deg, file, line, 0, endPosition - startPosition, !val, 0, 0, 0);
	}
	else
	{
		pktSendF32Deg(pak, val);
	}
}

void profile_sendMat4(char *file, int line, Packet *pak, Mat4 mat)
{
	if (s_collectingNetStats)
	{
		int startPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		int endPosition;

		sendMat4(pak, mat);

		endPosition = pak->stream.cursor.byte * 8 + pak->stream.cursor.bit;
		devassert(!pak->hasDebugInfo);
		if (!pak->hasDebugInfo)
			addStats(&htSendMat4Ex, file, line, 0, endPosition - startPosition, 0, 0, 0, 0);
	}
	else
	{
		sendMat4(pak, mat);
	}

}
