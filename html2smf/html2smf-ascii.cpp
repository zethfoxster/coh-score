// html2smf.cpp : simple tool to convert from html to smf for use in City of Heroes game engine
// This was made mainly to convert the EULA documents for use in the game.
// Use Microsoft Word to first export the document to an .html or .htm file and
// then run this tool on the .html or .htm file.
//
// Either of the "Web Page" or "Web Page, Filtered" options in the "Save As" dialog of
// Microsoft Word will work fine.
//
// IMPORTANT: Make sure the message store files (*.ms) are saved with UTF-8 encoding!!
//
// TODO: automatically make <a href="URL">URL</a> of unanchored "http://URL.html" text
// TODO: automatically make <a href="URL">URL</a> of unanchored "http://URL.php" text
// TODO: automatically make <a href="URL">URL</a> of unanchored "http://URL.txt" text
//

#include "stdafx.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

typedef enum
{
	PARSING_COPY,
	PARSING_LOAD_SPECIAL,
	PARSING_LOAD_TAG,
	PARSING_LOAD_COMMENT,
	PARSING_IGNORE_TAG,
	PARSING_IGNORE_TAG2
} PARSING_MODE;

static char s_IgnoreTag[256];

static unsigned char s_OutputBuffer[4096];
static size_t s_OutputBufferUsed = 0;

static bool s_InAnchor = false;

//*********************************************************************
void AddAsciiToUTF8(char character)
{
	if (character < 128)
	{
		s_OutputBuffer[s_OutputBufferUsed] = character;
		s_OutputBufferUsed++;
	}
	else if (character < 192)
	{
		s_OutputBuffer[s_OutputBufferUsed] = 194;
		s_OutputBufferUsed++;
		s_OutputBuffer[s_OutputBufferUsed] = character;
		s_OutputBufferUsed++;
	}
	else
	{
		s_OutputBuffer[s_OutputBufferUsed] = 195;
		s_OutputBufferUsed++;
		s_OutputBuffer[s_OutputBufferUsed] = character & 0xbf;
		s_OutputBufferUsed++;
	}
}

//*********************************************************************
void OutputChar(char character, FILE *outFile)
{
	/*
	// Special open/close quotes
	if (character == '“' || character == '”')
		character = '"';
	*/

	// Ignore leading spaces
	if (s_OutputBufferUsed == 0 && character == ' ')
		return;

	// Ignore duplicate spaces
	if (s_OutputBufferUsed > 0 && s_OutputBuffer[s_OutputBufferUsed-1] == ' ' && character == ' ')
		return;

	if (s_OutputBufferUsed == 4090)
	{
		s_OutputBuffer[s_OutputBufferUsed] = 0;
		fputs((char*)s_OutputBuffer, outFile);
		s_OutputBufferUsed = 0;
		AddAsciiToUTF8(character);
		return;
	}

	if (character != '\n')
	{
		AddAsciiToUTF8(character);
	}
	else
	{
		if (s_OutputBufferUsed == 0)
			return;

		s_OutputBuffer[s_OutputBufferUsed] = character;
		s_OutputBufferUsed++;
		s_OutputBuffer[s_OutputBufferUsed] = 0;
		fputs((char*)s_OutputBuffer, outFile);
		s_OutputBufferUsed = 0;
	}
}

//*********************************************************************
void OutputString(const char *string, FILE *outFile)
{
	size_t i, len = strlen(string);

	for (i = 0; i < len; i++)
		OutputChar(string[i], outFile);
}

//*********************************************************************
PARSING_MODE ParseTag(const char *tagBuffer, int tagSize, FILE *outFile)
{
	// "<a href="
	if (tagSize > 3 && strncmp(tagBuffer, "<a href=", 8) == 0)
	{
		assert(!s_InAnchor);
		OutputString(tagBuffer, outFile);
		s_InAnchor = true;
		return PARSING_COPY;
	}

	// "</a>"
	if (tagSize == 4 && strncmp(tagBuffer, "</a>", 4) == 0)
	{
		if (s_InAnchor)
		{
			OutputString(tagBuffer, outFile);
			s_InAnchor = false;
		}

		return PARSING_COPY;
	}

	// "<p "
	if (tagSize > 3 && strncmp(tagBuffer, "<p ", 3) == 0)
	{
		OutputString("<p>", outFile);

		// Handle indentation specified by style='margin-left:
		const char *cursor = strstr(tagBuffer, "style='margin-left:");
		if (cursor != NULL)
		{
			float value = 0;
			bool foundDecimal = false;
			float decimalValue = 0.1f;

			// skip over "style='margin-left:"
			cursor += strlen("style='margin-left:");

			// Might be ".75in" or ".75in" or "58.0pt" or 
			const char *cursor2 = cursor;

			while(*cursor2 != '>' && *cursor2 != ';' && *cursor2 != '\'')
			{
				if (cursor2[0] == 'p' && cursor2[1] == 't')
				{
					// assumes 72 points per inch
					value /= 72.0f;
					break;
				}

				if (cursor2[0] == 'i' && cursor2[1] == 'n')
				{
					break;
				}

				if (*cursor2 == '.')
				{
					foundDecimal = true;
				}
				else if (*cursor2 >= '0' && *cursor2 <= '9')
				{
					if (foundDecimal)
					{
						value += (*cursor2 - '0') * decimalValue;
						decimalValue *= 0.1f;
					}
					else
					{
						value = value * 10 + (*cursor2 - '0');
					}
				}

				cursor2++;
			}

			// We use an image placeholder since smf will strip leading spaces
			// and does not support tabs
			char buf[256];
			float size = value * 72;
			sprintf_s(buf, 256, "<img src=invisible.tga width=%.1f height=1>", size);
			OutputString(buf, outFile);
		}

		return PARSING_COPY;
	}

	// "<p>"
	if (tagSize == 3 && strncmp(tagBuffer, "<p>", 3) == 0)
	{
		OutputString(tagBuffer, outFile);
		return PARSING_COPY;
	}

	// "</p>"
	if (tagSize == 4 && strncmp(tagBuffer, "</p>", 4) == 0)
	{
		OutputString("</p>\n", outFile);
		return PARSING_COPY;
	}

	// "<b "
	if (tagSize > 3 && strncmp(tagBuffer, "<b ", 3) == 0)
	{
		OutputString("<b>", outFile);
		return PARSING_COPY;
	}

	// "<b>"
	if (tagSize == 3 && strncmp(tagBuffer, "<b>", 3) == 0)
	{
		OutputString(tagBuffer, outFile);
		return PARSING_COPY;
	}

	// "</b>"
	if (tagSize == 4 && strncmp(tagBuffer, "</b>", 4) == 0)
	{
		OutputString(tagBuffer, outFile);
		return PARSING_COPY;
	}

	// <title>
	if (tagSize == 7 && strncmp(tagBuffer, "<title>", 7) == 0)
	{
		strcpy_s(s_IgnoreTag, 256, "</title>");
		return PARSING_IGNORE_TAG;
	}

	return PARSING_COPY;
}

//*********************************************************************
PARSING_MODE ParseSpecial(const char *tagBuffer, int tagSize, FILE *outFile)
{
	// Change &quot; to "
	if (strcmp(tagBuffer, "&quot;") == 0)
	{
		OutputString("\"", outFile);
		return PARSING_COPY;
	}

	// These are handled propery by smf
	if (strcmp(tagBuffer, "&nbsp;") == 0 ||
		strcmp(tagBuffer, "&lt;") == 0 ||
		strcmp(tagBuffer, "&gt;") == 0 ||
		strcmp(tagBuffer, "&amp;") == 0 ||
		strcmp(tagBuffer, "&#37;") == 0)
	{
		OutputString(tagBuffer, outFile);
		return PARSING_COPY;
	}

	return PARSING_COPY;
}

//*********************************************************************
void Parse(FILE *inFile, FILE *outFile)
{
	int character;
	char tagBuffer[2048];
	int tagSize = 0;
	PARSING_MODE currentMode = PARSING_COPY;

	while((character = fgetc(inFile)) != EOF)
	{
		if (currentMode == PARSING_COPY)
		{
			if (character == '<')
			{
				currentMode = PARSING_LOAD_TAG;
				tagSize = 0;
			}
			else if (character == '&')
			{
				currentMode = PARSING_LOAD_SPECIAL;
				tagSize = 0;
			}
			else
			{
				if (character == '\n')
					character = ' ';

				OutputChar(character, outFile);
			}

		}

		if (currentMode == PARSING_IGNORE_TAG)
		{
			if (character == '<')
			{
				currentMode = PARSING_IGNORE_TAG2;
				tagSize = 0;
			}
		}
		if (currentMode == PARSING_IGNORE_TAG2)
		{
			tagBuffer[tagSize] = character;
			tagSize++;
			assert(tagSize < 2048);

			if (character == '>')
			{
				tagBuffer[tagSize] = 0;

				if (strcmp(tagBuffer, s_IgnoreTag) == 0)
				{
					currentMode = PARSING_COPY;
				}
				else
				{
					currentMode = PARSING_IGNORE_TAG;
				}
			}
		}

		// Load and parse tags
		if (currentMode == PARSING_LOAD_TAG)
		{
			if (character == '\n')
				character = ' ';

			tagBuffer[tagSize] = character;
			tagSize++;
			assert(tagSize < 2048);

			// Look for "<!--" which starts a comment
			if (tagSize == 4 && strncmp(tagBuffer, "<!--", 4) == 0)
			{
				currentMode = PARSING_LOAD_COMMENT;
				tagSize = 0;
			}

			// Look for ">" which ends a tag
			if (character == '>')
			{
				// NULL terminate the tag buffer
				tagBuffer[tagSize] = 0;

				currentMode = ParseTag(tagBuffer, tagSize, outFile);
			}
		}

		// Load and parse special characters
		if (currentMode == PARSING_LOAD_SPECIAL)
		{
			tagBuffer[tagSize] = character;
			tagSize++;
			assert(tagSize < 2048);

			// Look for ";" which ends a special character
			if (character == ';')
			{
				// NULL terminate the tag buffer
				tagBuffer[tagSize] = 0;

				currentMode = ParseSpecial(tagBuffer, tagSize, outFile);
			}
		}

		// Ignore comments
		if (currentMode == PARSING_LOAD_COMMENT)
		{
			if (tagSize < 3)
			{
				tagBuffer[tagSize] = character;
				tagSize++;
			}
			else
			{
				tagBuffer[0] = tagBuffer[1];
				tagBuffer[1] = tagBuffer[2];
				tagBuffer[2] = character;
			}

			if (tagSize == 3 && strncmp(tagBuffer, "-->", 3) == 0)
			{
				currentMode = PARSING_COPY;
			}
		}
	}

}

//*********************************************************************
int main(int argc, char* argv[])
{
	char *outputFilename = NULL;

	// Check Arguments
	if (argc < 2 || argc > 3)
	{
		printf("Usage: %s <input file> [<output file>]\n", argv[0]);
		printf("   Converts HTML files to SMF format.");
		return 1;
	}

	// Open Input File
	FILE *inFile;
	fopen_s(&inFile, argv[1], "rtS");

	if (!inFile)
	{
		printf("Could not open %s for reading\n", argv[1]);
		return 1;
	}

	printf("Opened input file %s\n", argv[1]);

	// Get Output file name
	if (argc < 3)
	{
		size_t len = strlen(argv[1]);
		outputFilename = _strdup(argv[1]);

		if (_stricmp(outputFilename + len - 4, ".htm") == 0)
		{
			strcpy_s(outputFilename + len - 4, 5, ".smf");
		}
		else if (_stricmp(outputFilename + len - 5, ".html") == 0)
		{
			strcpy_s(outputFilename + len - 5, 6, ".smf");
		}
		else
		{
			free(outputFilename);
			fclose(inFile);

			printf("Input file '%s' does not end in '.html' or '.htm'\n", argv[1]);
			return 1;
		}
	}
	else
	{
		outputFilename = argv[2];
	}

	// Open output file
	FILE *outFile;
	fopen_s(&outFile, outputFilename, "wtS");
	if (!outFile)
	{
		printf("Could not open %s for writing\n", outputFilename);
		return 1;
	}

	printf("Opened output file %s\n", outputFilename);

	if (argc < 2)
	{
		free(outputFilename);
	}

	// Do the work
	printf("Converting\n");
	Parse(inFile, outFile);

	// Close files
	printf("Closing output file %s\n", outputFilename);
	fclose(outFile);

	printf("Closing input file %s\n", argv[1]);
	fclose(inFile);

	return 0;
}

