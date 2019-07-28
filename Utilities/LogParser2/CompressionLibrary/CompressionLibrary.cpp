// This is the main DLL file.

#include "stdafx.h"

#include "CompressionLibrary.h"
#include "include/zlib.h"
#include <stdio.h>
#include <stdlib.h>
#include <vcclr.h>

namespace CompressionLibrary
{
	char* allocateFileName(String^ fileName)
	{
		size_t unused;
		size_t sizeInBytes = (fileName->Length + 1) * 2;
		char* usableFileName = new char[sizeInBytes];
		pin_ptr<const wchar_t> wideFileName = PtrToStringChars(fileName);
		wcstombs_s(&unused, usableFileName, sizeInBytes, wideFileName, sizeInBytes);
		return usableFileName;
	}
	
	void freeFileName(char* fileName)
	{
		delete[] fileName;
	}

	void GZipper::Read(String^ gzipFileName, String^ outputFileName)
	{
		static const size_t BUFFER_SIZE = 4096;

		char* gzipUsableFileName = allocateFileName(gzipFileName);
		gzFile gzipFile = gzopen(gzipUsableFileName, "rb");
		freeFileName(gzipUsableFileName);
		
		char* outputUsableFileName = allocateFileName(outputFileName);
		FILE* outputFile;
		fopen_s(&outputFile, outputUsableFileName, "wb");
		freeFileName(outputUsableFileName);

		unsigned char buffer[BUFFER_SIZE];
		int bytesRead;
		while ((bytesRead = gzread(gzipFile, buffer, BUFFER_SIZE)) > 0)
		{
			fwrite(buffer, 1, bytesRead, outputFile);
		}

		fclose(outputFile);
		gzclose(gzipFile);
	}

	void GZipper::Write(String^ inputFileName, String^ gzipFileName)
	{
		static const size_t BUFFER_SIZE = 4096;

		char* inputUsableFileName = allocateFileName(inputFileName);
		FILE* inputFile;
		fopen_s(&inputFile, inputUsableFileName, "rb");
		freeFileName(inputUsableFileName);

		char* gzipUsableFileName = allocateFileName(gzipFileName);
		gzFile gzipFile = gzopen(gzipUsableFileName, "wb");
		freeFileName(gzipUsableFileName);

		unsigned char buffer[BUFFER_SIZE];
		int bytesRead;
		while ((bytesRead = fread(buffer, 1, BUFFER_SIZE, inputFile)) > 0)
		{
			gzwrite(gzipFile, buffer, bytesRead);
		}

		gzclose(gzipFile);
		fclose(inputFile);
	}
}
