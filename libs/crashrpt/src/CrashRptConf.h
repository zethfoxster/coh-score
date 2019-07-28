#ifndef CRASHRPTCONF_H
#define CRASHRPTCONF_H

// CrashRpt.h
#ifndef CREXTERN 
	#ifdef _USRDLL
		// CRASHRPTAPI should be defined in all of the DLL's source files as
		#define CREXTERN __declspec(dllexport)
	#else
		// This header file is included by an EXE - export
		#define CREXTERN __declspec(dllimport)
	#endif
#endif

#define CREXPORT __cdecl

#endif