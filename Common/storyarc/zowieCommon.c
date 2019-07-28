
#include "SuperAssert.h"
#include "utils.h"
#include "mathutil.h"
#include "zowieCommon.h"

Zowie **g_Zowies = NULL;

// Zowie common stuff

int zowie_type_matches(const char *names1, const char *names2)
{
	int i;
	int j;
	char *copy1;
	char *copy2;
	int argc1;
	int argc2;
	char *argv1[100];
	char *argv2[100];

	if (names1 == NULL || names1[0] == 0 || names2 == NULL || names2[0] == 0)
	{
		return 0;
	}

	strdup_alloca(copy1, names1);
	strdup_alloca(copy2, names2);
	argc1 = tokenize_line(copy1, argv1, 0);
	argc2 = tokenize_line(copy2, argv2, 0);

	// Ugh.  Not pretty, doing the N squared thing.  However, in practice, argc1 and argc2 will rarely exceed 4, unless design are doing something
	// really wacky.  So this won't get out of hand.
	for(i = 0; i < argc1; i++)
	{
		for (j = 0; j < argc2; j++)
		{
			if (stricmp(argv1[i], argv2[j]) == 0)
			{
				return 1;
			}
		}
	}
	return 0;
}

//count how many zowies prior to N match the zowietaskname
int count_matching_zowies_up_to(char *task_zowie_name, int n)
{
	int count, i;

	count = 0;
	for (i = 0; i < n; i++)
	{
		if (zowie_type_matches(task_zowie_name, g_Zowies[i]->name))
		{
			count++;
		}
	}
	return count;
}

// find the zowie that has the same name and position.
int find_zowie_from_pos_and_name(char *name, Vec3 zPos)
{
	int i;

	for (i = eaSize(&g_Zowies) - 1; i >= 0; i--)
	{
		if (stricmp(name, g_Zowies[i]->name) == 0 && distance3Squared(zPos, g_Zowies[i]->pos) < 1)
		{
			break;
		}
	}
	return i;
}

// This actually works backwards through the earray of zowies.  As long as we do the same
int find_nth_zowie_of_this_type(char *type, int n)
{
	int i;

	for (i = eaSize(&g_Zowies) - 1; i >= 0; i--)
	{
		if (zowie_type_matches(type, g_Zowies[i]->name) && n-- <= 0)
		{
			break;
		}
	}
	return i;
}

// Takes a set of avail zowies, and selects count of them at random.
// PRNG is seeded with the provided seed, indices of resulting choices
// are returned in result.  It is assumed that result points to an array
// at least as large as count.
unsigned int ZowieBuildPool(int count, int avail, unsigned int seed, int *result)
{
	int i;
	int swap;
	int work[MAX_GLOWING_ZOWIE_COUNT];

	// Silently clamp avail.  This will only ever trip if World Design drop a metric crap-ton of a single zowie type into a map.  I have
	// no compunction at all about doing this, since the only result of this is that the higher numbered ones will never be used.  Bummer.
	if (avail > MAX_GLOWING_ZOWIE_COUNT)
	{
		avail = MAX_GLOWING_ZOWIE_COUNT;
	}
	// This should never trip, the caller should have already checked this.
	assertmsg(count <= avail, "Task has zowie count greater than available");
	if (count == avail)
	{
		for (i = 0; i < count; i++)
		{
			result[i] = i;
		}
		return seed;
	}

	// Rule 30 fails when the seed is zero, cover that case
	if (seed == 0)
	{
		seed = 0xbaadf00d;
	}

	work[0] = 0;
	for (i = 1; i < avail; i++)
	{
		seed = (seed | _lrotl(seed, 1)) ^ _lrotr(seed, 1);
		swap = seed % (i + 1);		// Yes, I did mean (i + 1).  See the section about initializing and shuffling at the same time 
									// towards the bottom of http://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle
		work[i] = work[swap];
		work[swap] = i;
	}

	// Work now holds the whole set, shuffled.  Grab the first count entries for a nice random selection
	for (i = 0; i < count; i++)
	{
		result[i] = work[i];
	}
	return seed;
}

#define za (*((Zowie **) a))
#define zb (*((Zowie **) b))

int zowieSortCompare(const void *a, const void *b)
{
	if (za->pos[0] != zb->pos[0])
	{
		return za->pos[0] < zb->pos[0] ? -1 : 1;
	}
	if (za->pos[2] != zb->pos[2])
	{
		return za->pos[2] < zb->pos[2] ? -1 : 1;
	}
	return za->pos[1] < zb->pos[1] ? -1 : 1;
}	
