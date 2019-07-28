//----------------------------------------------------------------------------
//
// Simple example of how to open a dump file and get its stack.
//
// This is not a debugger extension.  It is a tool that can be used to replace
// the debugger.
//
//
// Copyright (C) Microsoft Corporation, 2000.
//
//----------------------------------------------------------------------------

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <windows.h>
#include <dbgeng.h>

#include "unzip.h"

#include "out.hpp"

#pragma comment(lib, "lib/dbghelp.lib")
#pragma comment(lib, "lib/dbgeng.lib")

void cleanupTempFiles();

PSTR g_DumpFile;
PSTR g_ZipFile;
PSTR g_ImagePath;
PSTR g_SymbolPath;
bool g_DumpFileZipped=false;

ULONG64 g_TraceFrom[3];

IDebugClient* g_Client;
IDebugControl* g_Control;
IDebugSymbols* g_Symbols;

void
Exit(int Code, PCSTR Format, ...)
{
    // Clean up any resources.
    if (g_Symbols != NULL)
    {
        g_Symbols->Release();
    }
    if (g_Control != NULL)
    {
        g_Control->Release();
    }
    if (g_Client != NULL)
    {
        //
        // Request a simple end to any current session.
        // This may or may not do anything but it isn't
        // harmful to call it.
        //

        // We don't want to see any output from the shutdown.
        g_Client->SetOutputCallbacks(NULL);
        
        g_Client->EndSession(DEBUG_END_PASSIVE);
        
        g_Client->Release();
    }

    // Output an error message if given.
    if (Format != NULL)
    {
        va_list Args;

        va_start(Args, Format);
        vfprintf(stderr, Format, Args);
        va_end(Args);
    }

	cleanupTempFiles();
    
    exit(Code);
}

void
CreateInterfaces(void)
{
    HRESULT Status;

    // Start things off by getting an initial interface from
    // the engine.  This can be any engine interface but is
    // generally IDebugClient as the client interface is
    // where sessions are started.
    if ((Status = DebugCreate(__uuidof(IDebugClient),
                              (void**)&g_Client)) != S_OK)
    {
        Exit(1, "DebugCreate failed, 0x%X\n", Status);
    }

    // Query for some other interfaces that we'll need.
    if ((Status = g_Client->QueryInterface(__uuidof(IDebugControl),
                                           (void**)&g_Control)) != S_OK ||
        (Status = g_Client->QueryInterface(__uuidof(IDebugSymbols),
                                           (void**)&g_Symbols)) != S_OK)
    {
        Exit(1, "QueryInterface failed, 0x%X\n", Status);
    }
}

void
ParseCommandLine(int Argc, char** Argv)
{
    int i;
    
    while (--Argc > 0)
    {
        Argv++;

        if (!strcmp(*Argv, "-a32"))
        {
            if (Argc < 4)
            {
                Exit(1, "-a32 missing arguments\n");
            }

            for (i = 0; i < 3; i++)
            {
                int Addr;
                
                Argv++;
                Argc--;

                if (sscanf(*Argv, "%i", &Addr) == EOF)
                {
                    Exit(1, "-a32 illegal argument type\n");
                }

                g_TraceFrom[i] = (ULONG64)(LONG64)(LONG)Addr;
            }
        }
        else if (!strcmp(*Argv, "-a64"))
        {
            if (Argc < 4)
            {
                Exit(1, "-a64 missing arguments\n");
            }

            for (i = 0; i < 3; i++)
            {
                Argv++;
                Argc--;

                if (sscanf(*Argv, "%I64i", &g_TraceFrom[i]) == EOF)
                {
                    Exit(1, "-a64 illegal argument type\n");
                }
            }
        }
        else if (!strcmp(*Argv, "-i"))
        {
            if (Argc < 2)
            {
                Exit(1, "-i missing argument\n");
            }

            Argv++;
            Argc--;

            g_ImagePath = *Argv;
        }
        else if (!strcmp(*Argv, "-y"))
        {
            if (Argc < 2)
            {
                Exit(1, "-y missing argument\n");
            }

            Argv++;
            Argc--;

            g_SymbolPath = *Argv;
        }
		else if (!strcmp(*Argv, "-f"))
		{
			if (Argc < 2)
			{
				Exit(1, "-f missing argument\n");
			}

			Argv++;
			Argc--;

			g_DumpFile = *Argv;
		}
		else if (!strcmp(*Argv, "-z"))
		{
			if (Argc < 2)
			{
				Exit(1, "-z missing argument\n");
			}

			Argv++;
			Argc--;

			g_DumpFile = *Argv;
			g_DumpFileZipped = true;
		}
        else
        {
            Exit(1, "Unknown command line argument '%s'\n", *Argv);
        }
    }

    if (g_DumpFile == NULL)
    {
        Exit(1, "No dump file specified, use -f <file> or -z <file.zip>\n");
    }
}

int strEndsWith(const char* str, const char* ending)
{
	size_t strLength;
	size_t endingLength;
	if(!str || !ending)
		return 0;

	strLength = strlen(str);
	endingLength = strlen(ending);

	if(endingLength > strLength)
		return 0;

	if(_stricmp(str + strLength - endingLength, ending) == 0)
		return 1;
	else
		return 0;
}

char temp_fname[MAX_PATH]={0};

void cleanupTempFiles()
{
	if (temp_fname[0]) {
		remove(temp_fname);
		temp_fname[0] = '\0';
	}
}

int extractZippedMinidump(char *zipfilename)
{
	unzFile zipfile;
	int ret=0;

	zipfile = unzOpen(zipfilename);
	if (!zipfile)
		return 0;

	if (UNZ_OK == unzGoToFirstFile(zipfile)) {
		do {
			char filename[MAX_PATH];
			unz_file_info info;
			unzGetCurrentFileInfo(zipfile,
				&info,
				filename,
				sizeof(filename),
				NULL,
				0,
				NULL,
				0);
			if (strEndsWith(filename, ".mdmp")) {
				// Found the appropriate file!
				// Extract it
				char *data = (char*)malloc(info.uncompressed_size+1);
				if (data) {

					if (UNZ_OK==unzOpenCurrentFile(zipfile)) {
						int numread = unzReadCurrentFile(zipfile, data, info.uncompressed_size+1);
						if (numread == info.uncompressed_size) {
							// Write data to file and free!
							GetTempFileName(".", "dp_", 0, temp_fname);
							FILE *f = fopen(temp_fname, "wb");
							if (f) {
								fwrite(data, 1, numread, f);
								fclose(f);
								g_ZipFile = g_DumpFile;
								g_DumpFile = temp_fname;
								ret = 1;
							} else {
								printf("Failed to open temp file for writing: %s\n", temp_fname);
							}
						} else {
							printf("Failed to read entire minidump file\n");
						}
					}
					free(data);
					data = NULL;
				} else {
					printf("Error allocating temporary memory buffer of size %d\n", info.uncompressed_size);
				}

				unzClose(zipfile);
				return ret;
			}
		} while (unzGoToNextFile(zipfile)==UNZ_OK);
	}
	printf("Could not find a .mdmp file in the .zip file: %s\n", zipfilename);

	unzClose(zipfile);
	return ret;
}


void
ApplyCommandLineArguments(void)
{
    HRESULT Status;

    // Install output callbacks so we get any output that the
    // later calls produce.
    if ((Status = g_Client->SetOutputCallbacks(&g_OutputCb)) != S_OK)
    {
        Exit(1, "SetOutputCallbacks failed, 0x%X\n", Status);
    }

    if (g_ImagePath != NULL)
    {
        if ((Status = g_Symbols->SetImagePath(g_ImagePath)) != S_OK)
        {
            Exit(1, "SetImagePath failed, 0x%X\n", Status);
        }
    }
    if (g_SymbolPath != NULL)
    {
        if ((Status = g_Symbols->SetSymbolPath(g_SymbolPath)) != S_OK)
        {
            Exit(1, "SetSymbolPath failed, 0x%X\n", Status);
        }
    }

	// If it's a zipped file, then extract a .mdmp file and open that instead
	if (g_DumpFileZipped)
	{
		if (0==extractZippedMinidump(g_DumpFile)) {
			Exit(1, "extractZippedMinidump failed\n");
		}
	}

	// Everything's set up so open the dump file.
    if ((Status = g_Client->OpenDumpFile(g_DumpFile)) != S_OK)
    {
        Exit(1, "OpenDumpFile failed, 0x%X\n", Status);
    }

    // Finish initialization by waiting for the event that
    // caused the dump.  This will return immediately as the
    // dump file is considered to be at its event.
    if ((Status = g_Control->WaitForEvent(DEBUG_WAIT_DEFAULT,
                                          INFINITE)) != S_OK)
    {
        Exit(1, "WaitForEvent failed, 0x%X\n", Status);
    }

    // Everything is now initialized and we can make any
    // queries we want.
}

void
DumpStack(void)
{
    HRESULT Status;
    PDEBUG_STACK_FRAME Frames = NULL;
    int Count = 50;

    printf("\nFirst %d frames of the call stack:\n", Count);

    if (g_TraceFrom[0] || g_TraceFrom[1] || g_TraceFrom[2])
    {
        ULONG Filled;
        
        Frames = new DEBUG_STACK_FRAME[Count];
        if (Frames == NULL)
        {
            Exit(1, "Unable to allocate stack frames\n");
        }
        
        if ((Status = g_Control->
             GetStackTrace(g_TraceFrom[0], g_TraceFrom[1], g_TraceFrom[2],
                           Frames, Count, &Filled)) != S_OK)
        {
            Exit(1, "GetStackTrace failed, 0x%X\n", Status);
        }

        Count = Filled;
    }
                           
    // Print the call stack.
    if ((Status = g_Control->
         OutputStackTrace(DEBUG_OUTCTL_ALL_CLIENTS, Frames,
                          Count, DEBUG_STACK_SOURCE_LINE |
                          DEBUG_STACK_FRAME_ADDRESSES |
                          DEBUG_STACK_COLUMN_NAMES |
                          DEBUG_STACK_FRAME_NUMBERS)) != S_OK)
    {
        Exit(1, "OutputStackTrace failed, 0x%X\n", Status);
    }

    delete[] Frames;
}

void __cdecl
main(int Argc, char** Argv)
{
    CreateInterfaces();
    
    ParseCommandLine(Argc, Argv);

    ApplyCommandLineArguments();
    
    DumpStack();

    Exit(0, NULL);
}
