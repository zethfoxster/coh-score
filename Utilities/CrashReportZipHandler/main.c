#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <windows.h>

#include "unzip.h"
#include <sys/stat.h>
#include <io.h>
#include <fcntl.h>
#include <conio.h>
#include <direct.h>

#include "solution.h"

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

	if(stricmp(str + strLength - endingLength, ending) == 0)
		return 1;
	else
		return 0;
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


char *getVersion(char *zipfilename)
{
	unzFile zipfile;
	char *ret=NULL;
	
	zipfile = unzOpen(zipfilename);
	if (!zipfile)
		return NULL;

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
			if (strEndsWith(filename, ".xml")) {
				// Found the appropriate file!
				// Extract it
				char *data = (char*)malloc(info.uncompressed_size+1);
				if (data) {

					if (UNZ_OK==unzOpenCurrentFile(zipfile)) {
						int numread = unzReadCurrentFile(zipfile, data, info.uncompressed_size+1);
						if (numread == info.uncompressed_size) {
							// Get data and free!
							char *ver = strstr(data, "Version: ");
							if (ver) {
								ver += strlen("Version: ");
								{
									char *end = strstr(ver, "\\n");
									if (end) {
										*end='\0';
										printf("Using version: %s\n", ver);
										ret = strdup(ver);
									}
								}
							}
						} else {
							printf("Failed to read entire file\n");
						}
					}
					free(data);
					data = NULL;
				} else {
					printf("Error allocating temporary memory buffer of size %d\n", info.uncompressed_size);
				}

			}
			if (strEndsWith(filename, ".mdmp")) {
				// Found the appropriate file!
				// Extract it
				char *data = (char*)malloc(info.uncompressed_size+1);
				if (data) {

					if (UNZ_OK==unzOpenCurrentFile(zipfile)) {
						int numread = unzReadCurrentFile(zipfile, data, info.uncompressed_size+1);
						if (numread == info.uncompressed_size) {
							// Get data and write it!
							FILE *fout = fopen("C:\\temp\\CityOfHeroes.mdmp", "wb");
							if (fout) {
								fwrite(data, info.uncompressed_size, 1, fout);
								fclose(fout);
							} else {
								printf("Error opening c:\\temp\\CityOfHeroes.mdmp for writing!\n");
							}
						} else {
							printf("Failed to read entire file\n");
						}
					}
					free(data);
					data = NULL;
				} else {
					printf("Error allocating temporary memory buffer of size %d\n", info.uncompressed_size);
				}

			}
		} while (unzGoToNextFile(zipfile)==UNZ_OK);
	}
	if (!ret) {
		printf("Could not find a .xml file in the .zip file: %s\n", zipfilename);
	}

	unzClose(zipfile);
	return ret;
}

void launchVS(char *solutionPath)
{
	char *paths[] = {"C:\\VS2003\\Common7\\IDE\\devenv.exe",
		"C:\\Program Files\\Microsoft Visual Studio .NET 2003\\Common7\\IDE\\devenv.exe",
		"C:\\Program Files\\Microsoft Visual Studio .NET\\Common7\\IDE\\devenv.exe",
		"C:\\Program Files (x86)\\Microsoft Visual Studio .NET 2003\\Common7\\IDE\\devenv.exe",
		"C:\\Program Files (x86)\\Microsoft Visual Studio .NET\\Common7\\IDE\\devenv.exe"};

	int index=-1;
	int i;
	char buf[1024];

	for (i=0; i<sizeof(paths)/sizeof(paths[0]); i++) {
		if (fileSize(paths[i])>0) {
			index = i;
			break;
		}
	}
	if (index==-1)
		return;

	sprintf(buf, "devenv.exe %s /run", solutionPath);


	{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	ZeroMemory( &pi, sizeof(pi) );

	// Start the child process. 
	if( !CreateProcess( paths[index], // No module name (use command line). 
		buf, // Command line. 
		NULL,             // Process handle not inheritable. 
		NULL,             // Thread handle not inheritable. 
		FALSE,            // Set handle inheritance to FALSE. 
		0,                // No creation flags. 
		NULL,             // Use parent's environment block. 
		NULL,             // Use parent's starting directory. 
		&si,              // Pointer to STARTUPINFO structure.
		&pi )             // Pointer to PROCESS_INFORMATION structure.
		) 
	{
		return;
	}

	// Wait until child process exits.
	WaitForSingleObject( pi.hProcess, INFINITE );

	// Close process and thread handles. 
	CloseHandle( pi.hProcess );
	CloseHandle( pi.hThread );
	}
}

int extractVersion(char *versionname)
{
	char buf[MAX_PATH];
	int ret;

	sprintf(buf, "ExtractSourceCode.bat %s", versionname);

	_chdrive(3);
	chdir("C:\\");
	ret = system(buf);

	if (ret) {
		printf("Error calling ExtractSourceCode.bat\n");
		_getch();
		return 1;
	}

	{
		FILE *fout = fopen("C:\\temp\\CityOfHeroes.sln", "w");
		if (!fout) {
			printf("Error opening C:\\temp\\cityofheroes.sln");
			_getch();
			return 1;
		}
		fprintf(fout, "%s", solution);
		fclose(fout);
	}

	launchVS("c:\\temp\\CityOfHeroes.sln");

	return 0;
}

int __cdecl main(int argc, char** argv)
{
	char *version;

	if (argc!=2) {
		printf("Usage: CrashReportZipHandler filename.zip\n");
		_getch();
		return 1;
	}

	mkdir("C:\\temp");
	if (fileSize(argv[1])<=0) {
		printf("File %s not found\n", argv[1]);
		_getch();
		return 2;
	}

	version = getVersion(argv[1]);
	if (!version) {
		printf("Error unzipping\n");
		_getch();
		return 3;
	}

	return extractVersion(version);

}