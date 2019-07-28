#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <windows.h>

#include "unzip.h"
#include "zip.h"
#include <sys/stat.h>
#include <io.h>
#include <fcntl.h>

// stristr /////////////////////////////////////////////////////////
//
// performs a case-insensitive lookup of a string within another
// (see C run-time strstr)
//
// str1 : buffer
// str2 : string to search for in the buffer
//
// example char* s = stristr("Make my day","DAY");
//
// S.Rodriguez, Jan 11, 2004
//
char* strstri(char* str1, const char* str2)
{
	__asm
	{
		mov ah, 'A'
		mov dh, 'Z'

		mov esi, str1
		mov ecx, str2
		mov dl, [ecx]
		test dl,dl ; NULL?
		jz short str2empty_label

outerloop_label:
		mov ebx, esi ; save esi
		inc ebx
innerloop_label:
		mov al, [esi]
		inc esi
		test al,al
		je short str2notfound_label ; not found!

        cmp     dl,ah           ; 'A'
        jb      short skip1
        cmp     dl,dh           ; 'Z'
        ja      short skip1
        add     dl,'a' - 'A'    ; make lowercase the current character in str2
skip1:		

        cmp     al,ah           ; 'A'
        jb      short skip2
        cmp     al,dh           ; 'Z'
        ja      short skip2
        add     al,'a' - 'A'    ; make lowercase the current character in str1
skip2:		

		cmp al,dl
		je short onecharfound_label
		mov esi, ebx ; restore esi value, +1
		mov ecx, str2 ; restore ecx value as well
		mov dl, [ecx]
		jmp short outerloop_label ; search from start of str2 again
onecharfound_label:
		inc ecx
		mov dl,[ecx]
		test dl,dl
		jnz short innerloop_label
		jmp short str2found_label ; found!
str2empty_label:
		mov eax, esi // empty str2 ==> return str1
		jmp short ret_label
str2found_label:
		dec ebx
		mov eax, ebx // str2 found ==> return occurence within str1
		jmp short ret_label
str2notfound_label:
		xor eax, eax // str2 nt found ==> return NULL
		jmp short ret_label
ret_label:
	}

}


int fileSize(const char *fname){
	struct stat status;

	if(!stat(fname, &status)){
		if(!(status.st_mode & _S_IFREG))
			return -1;
		return status.st_size;
	}

	return -1;
}


void *fileAlloc(char *fname,int *lenp)
{
	FILE	*file;
	int		total=0,bytes_read;
	char	*mem=0;

	total = fileSize(fname);

	file = fopen(fname,"rb");
	if (!file)
		return 0;

	mem = (char*)malloc(total+1);
	bytes_read = (int)fread(mem,1,total,file);
	fclose(file);
	mem[bytes_read] = 0;
	if (lenp)
		*lenp = bytes_read;
	return mem;
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

int removeFromZip(char *zipfilename, char *srcfilename)
{
	char tempname[MAX_PATH];
	zipFile tempfile=0;
	unzFile zipfile;
	int ret = 1;
	char *data=NULL;
	bool didWork=false;

	strcpy(tempname, zipfilename);
	*strrchr(tempname, '.')='\0';
	strcat(tempname, "_temp.zip");

	// Open input file
	zipfile = unzOpen(zipfilename);
	if (!zipfile) {
		printf("Error openging %s\n", zipfilename);
		goto fail;
	}

	// Open temporary output file
	tempfile = zipOpen(tempname, APPEND_STATUS_CREATE);
	if (!tempfile) {
		printf("Error opening %s\n", tempname);
		goto fail;
	}

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

			if (_stricmp(filename, srcfilename)!=0) {
				// Found an appropriate file!
				// Extract it
				data = (char*)malloc(info.uncompressed_size+1);
				if (data) {
					if (UNZ_OK==unzOpenCurrentFile(zipfile)) {
						int numread = unzReadCurrentFile(zipfile, data, info.uncompressed_size+1);
						if (numread == info.uncompressed_size) {
							// Write data to file and free!
							if (ZIP_OK != zipOpenNewFileInZip(tempfile,
								filename,
								NULL,
								NULL,
								0,
								NULL,
								0,
								NULL,
								Z_DEFLATED,
								Z_DEFAULT_COMPRESSION))
							{
								printf("Failure to open new file in zip\n");
								goto fail;
							}

							if (ZIP_OK != zipWriteInFileInZip(tempfile, data, info.uncompressed_size)) {
								printf("Error writing  file to zip\n");
								goto fail;
							}

							zipCloseFileInZip(tempfile);

						} else {
							printf("Failed to read entire minidump file\n");
							goto fail;
						}
					}
					free(data);
					data = NULL;
				} else {
					printf("Error allocating temporary memory buffer of size %d\n", info.uncompressed_size);
					goto fail;
				}

			} else {
				didWork = true;
			}
		} while (unzGoToNextFile(zipfile)==UNZ_OK);
	} else {
		printf("No files found in %s\n", zipfilename);
	}

	if (tempfile) {
		zipClose(tempfile, NULL);
		tempfile = NULL;
	}
	if (zipfile) {
		unzClose(zipfile);
		zipfile = NULL;
	}
	// Rename files around
	if (!didWork) {
		remove(tempname);
	} else {
		remove(zipfilename);
		rename(tempname, zipfilename);
	}

	ret = 0;

fail:
	if (tempfile)
		zipClose(tempfile, NULL);
	if (zipfile)
		unzClose(zipfile);
	if (data)
		free(data);
	return ret;
}

int addtozip(char *zipfilename, char *srcfilename)
{
	zipFile zipfile=0;
	int ret=1;

	int len;
	char *data;
	data = (char*)fileAlloc(srcfilename, &len);
	if (!data || len<=0) {
		printf("Error opening %s\n", srcfilename);
		goto fail;
	}

	// Remove old occurrences
	removeFromZip(zipfilename, srcfilename);

	zipfile = zipOpen(zipfilename, APPEND_STATUS_ADDINZIP);
	if (!zipfile) {
		printf("Error opening %s\n", zipfilename);
		goto fail;
	}

	if (ZIP_OK != zipOpenNewFileInZip(zipfile,
		srcfilename,
		NULL,
		NULL,
		0,
		NULL,
		0,
		NULL,
		Z_DEFLATED,
		Z_DEFAULT_COMPRESSION))
	{
		printf("Failure to open new file in zip\n");
		goto fail;
	}

	if (ZIP_OK != zipWriteInFileInZip(zipfile, data, len)) {
		printf("Error writing  file to zip\n");
		goto fail;
	}

	zipCloseFileInZip(zipfile);

	remove(srcfilename);

	ret = 0;

fail:
	if (data)
		free(data);

	if (zipfile)
		zipClose(zipfile, NULL);
	return ret;
}

char *starts[] = {"Windows", "Product", "System", "Process"};
#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

// Set of "expected" garbage at the begining of the call stack
char *expected[] = {

	// One of these two
	"SharedUserData!SystemCallStub",
	"ntdll!KiFastSystemCallRet",

	// One of these two
	"ntdll!NtWaitForSingleObject",
	"ntdll!ZwWaitForSingleObject",

	"kernel32!WaitForSingleObjectEx",
	"kernel32!WaitForSingleObject",

	"CrashRpt!WTL::CString::UnlockBuffer", // Old?

	"CrashRpt!assertWriteMiniDump",
	"CrashRpt!CExceptionReport::getCrashFile",
	"CrashRpt!CCrashHandler::GenerateErrorReport",
	"CrashRpt!CrashRptGenerateErrorReport",
	"CityOfHeroes!CallReportFault",
	"CityOfHeroes!assertExcept",
	
	// Couple options here for which thread it's called from
	"CityOfHeroes!WinMain",
	"CityOfHeroes!texWordsThread",
	"CityOfHeroes!soundSystemThread",
	"CityOfHeroes!soundPlayingThread",
	"CityOfHeroes!renderThread",
	"CityOfHeroes!animLoadingThread",
	
	"CityOfHeroes!_except_handler",

	"ntdll!RtlConvertUlongToLargeInteger", // Old?
	"ntdll!RtlConvertUlongToLargeInteger", // Old?
	"ntdll!ExecuteHandler2",
	"ntdll!ExecuteHandler",

	"ntdll!RtlDispatchException", // Optional on some W2K crashes?

	"ntdll!KiUserExceptionDispatcher",

	// Actually need to know about these in order to rewind properly when categorizing
//	"CityOfHeroes!clientFatalErrorfCallback",
//	"CityOfHeroes!FatalErrorf",
//	"CityOfHeroes!FatalErrorFilenamef",
};


void parseOutput()
{
	int len;
	char *data;
	char *line;
	char output[1024*256]={0};
	int i;
	int addmeall = 0;
	int expected_matching=1;
	int stack_index=0;
	int in_stack = 0;
	int found_error = 0;

	data = (char*)fileAlloc("FullStack.txt", &len);
	line = strtok(data, "\r\n");
	while (line) {
		int addme = 0;
		for (i=0; i<ARRAY_SIZE(starts); i++) {
			if (_strnicmp(starts[i], line, strlen(starts[i]))==0) {
				addme = 1;
			}
		}
		if (strstr(line, "violation")!=0) {
			addme = 1;
		}

		if (strstr(line, "RetAddr")!=0) {
			addmeall = 1;
			in_stack = 1;
			stack_index=-2; // inced to 0 on next loop
		}
		if (addmeall) {
			addme = 1;
		}

// #define PREFIX_ERROR "*** Error:"
#define PREFIX_ERROR			"*** ERROR:"
#define PREFIX_WARN_CHECKSUM	"*** WARNING: Unable to verify checksum for"
#define PREFIX_WARN_UNWIND		"WARNING: Stack unwind information"

		if (strncmp(line,PREFIX_ERROR,strlen(PREFIX_ERROR))==0)
		{
			// Note in the stack when we find an actual error, means the stack is probably useless
			if(!found_error)
				strcat(output, "*** BAD CRASH REPORT STACK ***\n");
			found_error = 1;
			addme = 0;
		}else if (strncmp(line,PREFIX_WARN_CHECKSUM,strlen(PREFIX_WARN_CHECKSUM))==0
			|| strncmp(line,PREFIX_WARN_UNWIND,strlen(PREFIX_WARN_UNWIND))==0) {
			addme = 0;
		} else if (in_stack) {
			stack_index++;
			while(addme && expected_matching && stack_index >= 0 && stack_index < ARRAY_SIZE(expected))
			{
				if (strstri(line, expected[stack_index])!=0) {
					// found a match
					addme = 0;
				} else {
					stack_index++;
				}
			}
			if(stack_index >= (int)(ARRAY_SIZE(expected)))
				expected_matching = 0;
		}

		if (addme) {
			strcat(output, line);
			strcat(output, "\n");
		}

		line = strtok(NULL, "\r\n");
	}

	FILE *fout = fopen("Stack.txt", "wt");
	fwrite(output, 1, strlen(output), fout);
	fclose(fout);
}

int __cdecl
main(int argc, char** argv)
{
	int ret;
	char buf[MAX_PATH];
	char *filename;
	char *cdb_cmds;


	if(argc == 2)
	{
		filename = argv[1];
		cdb_cmds = ".ecxr;knL;q";
	}
	else if(argc == 3)
	{
		filename = argv[2];
		cdb_cmds = argv[1];
	}
	else
	{
		printf("Extracts the stack from a minidump in a zip and readds to the .zip as stack.txt\n");
		printf("Usage: dumpandpack [cdb commands] filename.zip\n");
		return 1;
	}

	if (fileSize(filename)<=0) {
		printf("File %s not found\n", filename);
		return 2;
	}
//	sprintf(buf, "dumpstk -z %s > FullStack.txt", filename);

	if(!extractZippedMinidump(filename)) {
		printf("Extract Minidump failed.\n");
		return 3;
	}

	sprintf(buf, "cdb -z %s -logo FullStack.txt -lines -c \"%s\"", temp_fname, cdb_cmds);
	ret = system(buf);
	remove(temp_fname);

	if (ret!=0) {
		printf("cdb call failed.\n");
		return 4;
	}

	parseOutput();

	ret = addtozip(filename, "FullStack.txt");
	ret |= addtozip(filename, "Stack.txt");

	if (ret) {
		printf("Error adding new file(s) to .zips\n");
	}

	return ret;
}
