//--------------------------------------------------------------------------------------
// SymbolHelper.h
//
// Microsoft Game Technology Group.
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#ifndef SYMBOLHELPER_H
#define SYMBOLHELPER_H

#include <vector>
#include <atlbase.h>
#include <dia2.h>


//--------------------------------------------------------------------------------------
// Name: class Sample
// Desc: Structure for keeping track of loaded symbols when using DIA2.
//       Each Module object records the memory range (start address and size) covered by
//       this code module, along with a DIA2 session object to use for symbol lookups.
//--------------------------------------------------------------------------------------
struct Module
{
    Module( ULONG_PTR address, size_t size, CComPtr <IDiaSession>& psession ) : m_Address( address ),
                                                                                m_Size( size ),
                                                                                m_psession( psession )
    {
    }

    ULONG_PTR m_Address;
    size_t m_Size;
    CComPtr <IDiaSession> m_psession;
};

struct AddressSymbol	// information for an address lookup
{
	DWORD	address;
	CHAR	symbolName[1000];
	ULONG	displacement;
	CHAR	filename[500];
	ULONG	lineNumber;

	AddressSymbol(void)
	{}

	AddressSymbol( DWORD anAddress )
	{
		memset( this, sizeof(*this), 0 );
		address = anAddress;
	}

	std::string str()	// generate string representation of symbol information
	{
		std::string s;
		char buffer[2048];

		// Print out the symbol name and the offset of the address from
		// that symbol.
		sprintf_s( buffer, sizeof(buffer), "    %8X: %s+%u", address, symbolName, displacement );
		s += std::string(buffer);

		// Now print out the filename/linenumber information if we have it.
		if( filename[0] )
		{
			sprintf_s( buffer, sizeof(buffer), " - %s(%u)\n", filename, lineNumber );
			s += std::string(buffer);
		}
		else
			s += " - n\\a\n";
		return s;
	}
};

//--------------------------------------------------------------------------------------
// Name: class SymbolHelper
// Desc: Helper class for hiding the details of finding and loading symbols. Finding
//       symbols is done with DbgHelp and symbol lookups are done with DIA2. Additional
//       symbol lookup functions can be added as needed.
//--------------------------------------------------------------------------------------
class SymbolHelper
{
public:
            SymbolHelper();
            ~SymbolHelper();

	// Walk module list and try to retrieve symbol information for the address in the
	// address block, including the address, symbol name, offset, source file, line-number, etc.
	bool    ResolveAddress( AddressSymbol& as );

    // Print a summary of the specified symbol including the address, symbol name and
    // offset, and source file(line-number).
    VOID    PrintSymbolSummary( DWORD address );

    // Attempt to load the symbols for the specified module.
	// This function prints a success/failure message and returns true for success.
	// @todo include looking on a symbol server. 
    bool    LoadSymbolsForModule( const VOID* baseAddress,
                                  size_t size, DWORD timeStamp, const char* aModulePath );

private:
    // List of code modules (DLLs and EXEs) to look for symbols in.
    std::vector <Module> m_ModuleList;

    // Unique identifier for DbgHelp functions.
    HANDLE m_DebugProcess;
};

#endif
