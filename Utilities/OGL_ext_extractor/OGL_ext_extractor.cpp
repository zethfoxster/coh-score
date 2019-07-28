// OGL_ext_extractor.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string.h>
#include <stdlib.h>

static const char   *s_extensionStartString = "  <ext name=\"";
static const size_t  s_extensionStartStringLength = 13;

static const char   *s_extensionEndString = "  </ext>";
static const size_t  s_extensionEndStringLength = 8;

static const char   *s_rendererString = "    <renderer vendor=\"";
static const size_t  s_rendererStringLength = 22;

static const char   *s_osWinString = "os=\"Win\"";
static const size_t  s_osWinStringLength = 8;

static const char   *s_osMacString = "os=\"Mac\"";
static const size_t  s_osMacStringLength = 8;

typedef enum
{
	PLATFORM_WIN,
	PLATFORM_MAC,
} PLATFORM;

void usage(const char *command)
{
	printf("Usage: %s <platform> <card name> <extensions.xml path>\n\n", command);
	printf("This is a tool to quickly extact extensions for a given card\n");
	printf("from the extensions.xml file of the glview tool available from:\n\n");
	printf("http://www.realtech-vr.com/glview\n\n");
	printf("  <platform> should be -win or -mac\n");
	printf("Example: %s -win \"ATI Radeon HD 3850\" \"C:\\Program Files\\realtech VR\\OpenGL Extensions Viewer 3.0\\extensions.xml\"\n\n", command);
}

int main(int argc, char* argv[])
{
	errno_t error;
	const char *filename;
	const char *cardname;
	PLATFORM platform;
	FILE *inFile;
	char ext_buffer[1024];
	char card_buffer[1024];
	char card_name_search[1024];
	int extensionsFound = 0;
	int extensionsMatched = 0;

	if (argc != 4)
	{
		usage(argv[0]);
		return 0;
	}

	if (strcmp(argv[1], "-win") == 0)
	{
		platform = PLATFORM_WIN;
	}
	else if (strcmp(argv[1], "-mac") == 0)
	{
		platform = PLATFORM_MAC;
	}
	else
	{
		printf("  <platform> should be -win or -mac\n");
		return 0;
	}

	cardname = argv[2];
	filename = argv[3];

	// Build the string for searching for the card (name="<cardname>")
	_snprintf_s(card_name_search, _countof(card_name_search), _TRUNCATE, "name=\"%s\"", cardname);
	
	error = fopen_s(&inFile, filename, "rtS");

	if (!inFile)
	{
		printf("Could not open file \'%s\'\n", filename);
		return 0;
	}

	while(1)
	{
		// First, find an extension line
		//  <ext name="ARB_imaging" alias="" core="" spec="" req="" status="" cat="tex" depends="" affects="">
		if (fgets(ext_buffer, 1024, inFile) == NULL)
			break;

		if (strncmp(ext_buffer, s_extensionStartString, s_extensionStartStringLength) != 0)
			continue;

		extensionsFound++;

		while(1)
		{
			// Now, scan the renderer's for our card
			//    <renderer vendor="3Dlabs" name="Wildcat Realizm" version="1, 0, 0, 1" os="Win" />
			if (fgets(card_buffer, 1024, inFile) == NULL)
				break;

			// End of list of cards for extension
			if (strncmp(card_buffer, s_extensionEndString, s_extensionEndStringLength) == 0)
				break;

			if (strncmp(card_buffer, s_rendererString, s_rendererStringLength) != 0)
				continue;

			// see if this line has the card string
			if (strstr(card_buffer + s_rendererStringLength, card_name_search) == NULL)
				continue;

			if (platform == PLATFORM_WIN && strstr(card_buffer + s_rendererStringLength, s_osWinString) == 0)
				continue;

			if (platform == PLATFORM_MAC && strstr(card_buffer + s_rendererStringLength, s_osMacString) == 0)
				continue;

			// Now, get the extension name from the extension buffer
			char *extensionName = ext_buffer + s_extensionStartStringLength;
			size_t extensionNameLen = strlen(extensionName);
			size_t i = 0;

			while(i < extensionNameLen)
			{
				if (extensionName[i] == '\"')
					break;

				i++;
			}

			extensionName[i] = 0;

			printf("%s\n", extensionName);
			extensionsMatched++;
			break;
		}
	}

	fclose(inFile);

	printf("%d extensions matched out of %d extensions for \'%s\'\n", extensionsMatched, extensionsFound, cardname);

	return 0;
}

