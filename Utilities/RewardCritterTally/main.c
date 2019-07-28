#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define CATEGORY_MAX_LEN 2
#define TALLY_MAX_LEVEL 60

typedef struct category_tally_s
{
	char category[CATEGORY_MAX_LEN + 1];	// Need to hold the terminating NULL
	unsigned int tally[TALLY_MAX_LEVEL];
	struct category_tally_s* next;
} category_tally;

void bump_tally(category_tally* tallies, char* category, int level)
{
	category_tally* tptr;
	int count;

	tptr = tallies;

	// Search for a matching category.
	while (strcmp(tptr->category, category) != 0)
	{
		// If we reach the end of the list we must create a new category holder.
		if (!(tptr->next))
		{
			tptr->next = malloc(sizeof(category_tally));

			sprintf(tptr->next->category, category);

			for (count = 0; count < TALLY_MAX_LEVEL; count++)
			{
				tptr->next->tally[count] = 0;
			}

			tptr->next->next = NULL;
		}

		// We were either not at the end, or we inserted the appropriate category at the end
		// so we can always move onward.
		tptr = tptr->next;
	}

	// The category is always found because we create it if it's not there already.
	tptr->tally[level]++;
}

int main(int argc, char* argv[])
{
	FILE* infile;
	char fline[500];
	char cname[500];
	int clevel;
	char ccategory[500];
	int cmaxcontrib;
	int ctotalcontrib;
	int cparticipants;
	category_tally tallies;
	unsigned int total_lines;
	unsigned int malformed_lines;
	int count;
	category_tally* tptr;

	// Prime the list of tallies with a valid category.
	sprintf(tallies.category, "Mi");

	for (count = 0; count < TALLY_MAX_LEVEL; count++)
	{
		tallies.tally[count] = 0;
	}

	tallies.next = NULL;

	total_lines = 0;
	malformed_lines = 0;

	if (argc != 2)
	{
		printf("Syntax: %s <filename>\n", argv[0]);

		return 1;
	}

	infile = fopen(argv[1], "r");

	if (!infile)
	{
		printf("%s: Could not find '%s'\n", argv[0], argv[1]);

		return 1;
	}

	while (!feof(infile))
	{
		fgets(fline, 500, infile);

		// Skip comment lines which demarcate source files.
		if (fline[0] != '-' && fline[1] != '-')
		{
			total_lines++;

			sscanf(fline, "%s level %d, %s %d, %d, %d", cname, &clevel, ccategory, &cmaxcontrib, &ctotalcontrib, &cparticipants);
//			printf("Got %s : %d : %s : %d : %d : %d :\n", cname, clevel, ccategory, cmaxcontrib, ctotalcontrib, cparticipants);

			ccategory[CATEGORY_MAX_LEN] = '\0';

			if (clevel < 1 || clevel >= TALLY_MAX_LEVEL || strlen(ccategory) != CATEGORY_MAX_LEN || ccategory[0] == ' ' || ccategory[1] == ' ')
			{
				malformed_lines++;
			}
			else
			{
				bump_tally(&tallies, ccategory, clevel);
			}
		}
	}

	fclose(infile);

	tptr = &tallies;

	while (tptr)
	{
		for (count = 0; count < TALLY_MAX_LEVEL; count++)
		{
			if (tptr->tally[count] != 0)
			{
				printf("%s , %d , %d\n", tptr->category, count, tptr->tally[count]);
			}
		}

		tptr = tptr->next;
	}

	return 0;
}
