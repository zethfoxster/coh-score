///////////////////////////////////////////////////////////////////////////////
//
//  Module: excprpt.h
//
//    Desc: This class generates the dump and xml overview files.
//
// Copyright (c) 2003 Michael Carruth
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _EXCPRPT_H_
#define _EXCPRPT_H_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <atlmisc.h>
#include <dbghelp.h>

// Import MSXML interfaces
#import <msxml.tlb> named_guids raw_interfaces_only

//
// Modules linked list
//
static struct ModuleListEntry
{
   MINIDUMP_MODULE_CALLBACK item;
   struct ModuleListEntry *next;
} start, *node = &start;

//
// COM helper macros
//
#define CHECKHR(x) {HRESULT hr = x; if (FAILED(hr)) goto CleanUp;}
#define SAFERELEASE(p) {if (p) {(p)->Release(); p = NULL;}}


////////////////////////////// Class Definitions /////////////////////////////

// ===========================================================================
// CExceptionReport
// 
// See the module comment at top of file.
//
class CExceptionReport  
{
public:
	CExceptionReport(PEXCEPTION_POINTERS ExceptionInfo, DWORD dwThreadID);

   CString getSymbolFile(int index);
	int getNumSymbolFiles();
	CString getCrashLog(const char *szAuth, const char *szEntity, const char *szShard, const char *szShardTime, const char *szVersion, const char *szMessage);
	CString getCrashFile();
   CString getModuleName() { return m_sModule; };
   CString getExceptionCode() { return m_sException; };
   CString getExceptionAddr() { return m_sAddress; };

   static BOOL CALLBACK miniDumpCallback(PVOID CallbackParam,
                                         CONST PMINIDUMP_CALLBACK_INPUT CallbackInput,
                                         PMINIDUMP_CALLBACK_OUTPUT CallbackOutput);

   char m_sModuleOfCrash[1024];

private:
   static CString m_sModule;
   static CString m_sException;
   static CString m_sAddress;

   PEXCEPTION_POINTERS m_excpInfo;
   DWORD m_dwThreadID;
   CSimpleArray<CString> m_symFiles;

   static MSXML::IXMLDOMNode* CreateDOMNode(MSXML::IXMLDOMDocument* pDoc, 
                                            int type, 
                                            BSTR bstrName);

   static MSXML::IXMLDOMNode* CreateExceptionRecordNode(MSXML::IXMLDOMDocument* pDoc, 
                                                        EXCEPTION_RECORD* pExceptionRecord);

   static MSXML::IXMLDOMNode* CreateProcessorNode(MSXML::IXMLDOMDocument* pDoc);
   static MSXML::IXMLDOMNode* CreateGameNode(MSXML::IXMLDOMDocument* pDoc, const char *szAuth, const char *szEntity, const char *szShard, const char *szShardTime);
   static MSXML::IXMLDOMNode* CreateProgramNode(MSXML::IXMLDOMDocument* pDoc, const char *szVersion, const char* szMessage);

   static MSXML::IXMLDOMNode* CreateOSNode(MSXML::IXMLDOMDocument* pDoc);

   MSXML::IXMLDOMNode* CreateModulesNode(MSXML::IXMLDOMDocument* pDoc, EXCEPTION_RECORD* pExceptionRecord);

};

#endif	// #ifndef _EXCPRPT_H_
