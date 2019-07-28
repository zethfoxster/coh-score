
#include "stdtypes.h"

#include <stdio.h>
#include <windows.h>
#include <assert.h>
#include <file.h>

#include "package.h"

static PackageHeader *pkgHdr = NULL;

#define MAX_FILES 1000
#define MAX_FILENAME_LEN 1000

static int GetFileIndex(const char *filename)
{
	U32 i;

	assert(pkgHdr);

	for (i = 0; i < pkgHdr->numFiles; i++)
	{
		if (stricmp(filename, pkgHdr->fileInfos[i].filename) == 0)
			return i;
	}

	printf("Could not find file '%s' in package '%s'\n", filename, pkgHdr->filename);
	return -1;
}

static void DoCreateHeader(const char *filename_with_filenames)
{
	FILE *file;
	int numFiles = 0;
	char *filenames[MAX_FILES];
	char tempBuf[MAX_FILENAME_LEN];

	file = fopen(filename_with_filenames, "rt");

	if (!file)
	{
		printf("Failed to open file '%s'\n", filename_with_filenames);
		return;
	}

	while(fgets(tempBuf, MAX_FILENAME_LEN, file) != NULL)
	{
		// Remove Line-Feed character at the end of the string
		int len = strlen(tempBuf);
		if (tempBuf[len-1] == 10)
			tempBuf[len-1] = 0;

		filenames[numFiles] = _strdup(tempBuf);
		numFiles++;

		if (numFiles == MAX_FILES)
			break;
	}

	fclose(file);

	if (numFiles > 1)
	{
		pkgHdr = Package_CreatePackageHeader(filenames[0], filenames + 1, numFiles - 1);
	}
	else
	{
		printf("Not enough filenames in '%s'\n", filename_with_filenames);
	}
}



int main(int argc, char *argv[])
{
	int argsLeft = argc - 1;
	int currentArg = 1;
	int rc = 0;

	Package_InitOnce();

	while (argsLeft > 0)
	{
		if (stricmp(argv[currentArg], "-createheader") == 0)
		{
			if (argsLeft > 1)
			{
				DoCreateHeader(argv[currentArg + 1]);
				argsLeft -= 2;
				currentArg += 2;
			}
			else
			{
				printf("Not enough arguments for -createheader\n");
				rc = 1;
				break;
			}
		}
		else if (stricmp(argv[currentArg], "-loadheader") == 0)
		{
			if (argsLeft > 1)
			{
				pkgHdr = Package_ReadHeader(argv[currentArg + 1]);
				argsLeft -= 2;
				currentArg += 2;
			}
			else
			{
				printf("Not enough arguments for -loadheader\n");
				rc = 1;
				break;
			}
		}
		else if (stricmp(argv[currentArg], "-markzero") == 0)
		{
			if (argsLeft > 1 && pkgHdr)
			{
				int fileIndex = GetFileIndex(argv[currentArg + 1]);

				if (fileIndex >= 0)
				{
					Package_MarkFileZeros(pkgHdr, fileIndex);
				}

				argsLeft -= 2;
				currentArg += 2;
			}
			else
			{
				if (argsLeft < 2)
					printf("Not enough arguments for -markzero\n");
				else
					printf("No package has been loaded for -markzero\n");
				rc = 1;
				break;
			}
		}
		else if (stricmp(argv[currentArg], "-markunavailable") == 0)
		{
			if (argsLeft > 1 && pkgHdr)
			{
				int fileIndex = GetFileIndex(argv[currentArg + 1]);

				if (fileIndex >= 0)
				{
					Package_MarkFileUnavailable(pkgHdr, fileIndex);
				}

				argsLeft -= 2;
				currentArg += 2;
			}
			else
			{
				if (argsLeft < 2)
					printf("Not enough arguments for -markunavailable\n");
				else
					printf("No package has been loaded for -markunavailable\n");
				rc = 1;
				break;
			}
		}
		else if (stricmp(argv[currentArg], "-create") == 0)
		{
			if (pkgHdr)
			{
				Package_Create(pkgHdr);
				argsLeft -= 1;
				currentArg += 1;
			}
			else
			{
				printf("No package has been loaded for -create\n");
				rc = 1;
				break;
			}
		}
		else if (stricmp(argv[currentArg], "-flushavailable") == 0)
		{
			if (pkgHdr)
			{
				if ( Package_FlushAvailability(pkgHdr) != 0 )
				{
					printf("Package_FlushAvailability() succeeded\n");
				}
				else
				{
					printf("Package_FlushAvailability() failed\n");
				}
				argsLeft -= 1;
				currentArg += 1;
			}
			else
			{
				printf("No package has been loaded for -flushavailable\n");
				rc = 1;
				break;
			}
		}
		else if (stricmp(argv[currentArg], "-unpackfile") == 0)
		{
			if (argsLeft > 1 && pkgHdr)
			{
				int fileIndex = GetFileIndex(argv[currentArg + 1]);

				if (fileIndex >= 0)
				{
					Package_UnpackFile(pkgHdr, fileIndex);
				}

				argsLeft -= 2;
				currentArg += 2;
			}
			else
			{
				if (argsLeft < 2)
					printf("Not enough arguments for -unpackfile\n");
				else
					printf("No package has been loaded for -unpackfile\n");
				rc = 1;
				break;
			}
		}
		else if (stricmp(argv[currentArg], "-updatefile") == 0)
		{
			if (argsLeft > 1 && pkgHdr)
			{
				int fileIndex = GetFileIndex(argv[currentArg + 1]);

				if (fileIndex >= 0)
				{
					Package_UpdateFile(pkgHdr, fileIndex);
				}

				argsLeft -= 2;
				currentArg += 2;
			}
			else
			{
				if (argsLeft < 2)
					printf("Not enough arguments for -updatefile\n");
				else
					printf("No package has been loaded for -updatefile\n");
				rc = 1;
				break;
			}
		}
		else
		{
			printf("Unknown argument '%s'\n", argv[currentArg]);

			argsLeft--;
			currentArg++;
		}

	}

	Package_Shutdown();

	return rc;
}
