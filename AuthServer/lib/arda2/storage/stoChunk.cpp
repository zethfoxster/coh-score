/*****************************************************************************
    created:    2002/04/08
    copyright:  2002, NCSoft. All Rights Reserved
    author(s):  Peter M. Freese
    
    purpose:    
*****************************************************************************/

#include "arda2/storage/stoFirst.h"
#include "arda2/storage/stoChunk.h"

#include <cctype>

using namespace std;

stoChunkFileWriter::stoChunkFileWriter( stoFile &file, eSerializeMode mode,
                                       stoChunk::eEndianMode endianMode,
                                       eHeaderMode headerMode ) : 
    m_file(file),
    m_result(ER_Success),
    m_mode(mode),
    m_column(0)
{
    errAssert(file.CanWrite());
    errAssert(file.CanSeek());

    if ( endianMode != stoChunk::EM_LittleEndian )
    {
        ERR_UNIMPLEMENTED();
    }

    if ( headerMode == HM_WriteHeader )
    {
        WriteHeader();
    }
}


void stoChunkFileWriter::WriteHeader()
{
    char cEndian = 'L';

    // write the file header
    m_result |= m_file.Write("CHNK", 4);
    m_result |= m_file.Write(m_mode == kBinary ? "B" : "T", 1);
    m_result |= m_file.Write(cEndian);
    m_result |= m_file.Write("XX", 2);  // future flags

    if ( m_mode == kText )
        m_result |= m_file.Write("\r\n", 2);
}

errResult stoChunkFileWriter::EnterChunk( TAG4 tag, int nVersion)
{
    StackEntry chunk;
    ZeroObject(chunk);
    chunk.m_header.m_name = tag;
    chunk.m_header.m_version = nVersion;

    if ( m_mode == kBinary )
    {
        // write incomplete chunk header, patch it after chunk exit
        chunk.m_chunkStart = m_file.Tell();
        m_result |= m_file.Write(chunk.m_header);
    }
    else
    {
        m_result |= WriteFormattedText("CHUNK \"%s\" %i", TagToString(tag).c_str(), nVersion);
        m_result |= EndTextLine();
        m_result |= WriteFormattedText("{");
        m_result |= EndTextLine();
    }

    m_stack.push(chunk);

    return m_result;
}


errResult stoChunkFileWriter::ExitChunk()
{
    if ( m_mode == kBinary )
    {
        errAssert(!m_stack.empty());
        StackEntry &chunk = m_stack.top();
        int nPos = m_file.Tell();
        chunk.m_header.m_size = nPos - chunk.m_chunkStart - sizeof(stoChunk::Header);
        m_result |= m_file.Seek(chunk.m_chunkStart, stoFile::kSeekBegin);
        m_result |= m_file.Write(chunk.m_header);
        m_result |= m_file.Seek(nPos, stoFile::kSeekBegin);
    }

    m_stack.pop();

    if ( m_mode == kText )
    {
        m_result |= WriteFormattedText("}");
        m_result |= EndTextLine();
    }

    return m_result;
}


errResult stoChunkFileWriter::WriteIndent()
{
    int nIndent = (int)m_stack.size() * kChunkIndentation;
    m_column = nIndent;
    while ( --nIndent >= 0 )
    {
        m_result |= m_file.Write(" ", 1);
    }
    return m_result;
}

// Print-like output
errResult stoChunkFileWriter::WriteFormattedText( const char *fmt, ... )
{
    char buffer[1024];

    // get varargs stuff
    va_list args;
    va_start(args, fmt);

    // format in buffer
    int nLen = vsnprintf(buffer, sizeof(buffer), fmt, args);

    // end varargs
    va_end(args);

    // done
    if ( m_column > 0 && m_column + 1 + nLen > kMaxLineWidth )
    {
        m_result |= EndTextLine();
    }

    if ( m_column == 0 )
    {
        m_result |= WriteIndent();
    }
    else
    {
        m_result |= m_file.Write(" ", 1);
        ++m_column;
    }

    m_result |= m_file.Write(buffer, nLen);
    m_column += nLen;
    return m_result;
}

errResult stoChunkFileWriter::EndTextLine()
{
    m_result |= m_file.Write("\r\n", 2);
    m_column = 0;
    return m_result;
}


errResult stoChunkFileWriter::WriteText( bool u )
{
    return WriteFormattedText("%u", u);
}
errResult stoChunkFileWriter::WriteText( byte b )
{
    return WriteFormattedText("%u", b);
}
errResult stoChunkFileWriter::WriteText( uint16 u )
{
    return WriteFormattedText("%u", u);
}
errResult stoChunkFileWriter::WriteText( int16 i )
{
    return WriteFormattedText("%i", i);
}
errResult stoChunkFileWriter::WriteText( uint32 u )
{
    return WriteFormattedText("%u", u);
}
errResult stoChunkFileWriter::WriteText( int32 i )
{
    return WriteFormattedText("%i", i);
}
errResult stoChunkFileWriter::WriteText( uint64 u )
{
    return WriteFormattedText("%I64u", u);
}
errResult stoChunkFileWriter::WriteText( int64 i )
{
    return WriteFormattedText("%I64i", i);
}
errResult stoChunkFileWriter::WriteText( float32 f )
{
    return WriteFormattedText("%+#.8g", f);
}
errResult stoChunkFileWriter::WriteText( float64 f )
{
    return WriteFormattedText("%#.16g", f);
}
errResult stoChunkFileWriter::WriteText( const char* s )
{
    errAssert(s != NULL);
    return WriteFormattedText("\"%s\"", s);
}
errResult stoChunkFileWriter::WriteText( const string& s )
{
    return WriteFormattedText("\"%s\"", s.c_str());
}

errResult stoChunkFileWriter::PutBlock( IN const void *pBlock, 
                                        IN const stoChunk::eBlockDataType dataTypes[], 
                                        IN int nCount )
{
    using namespace stoChunk;
    switch ( m_mode )
    {
        case kText:
        {
            byte *pBlockBytes = (byte *)pBlock;
            while ( nCount-- )
            {
                for ( const eBlockDataType *pDataType = &dataTypes[0]; *pDataType != BT_END; ++pDataType )
                {
                    switch ( *pDataType )
                    {
                        case BT_byte:
                            m_result |= WriteText(*(byte *)pBlockBytes);
                            pBlockBytes += 1;
                            break;

                        case BT_char:
                            m_result |= WriteText(*(char *)pBlockBytes);
                            pBlockBytes += 1;
                            break;

                        case BT_uint16:
                            m_result |= WriteText(*(uint16 *)pBlockBytes);
                            pBlockBytes += 2;
                            break;

                        case BT_int16:
                            m_result |= WriteText(*(int16 *)pBlockBytes);
                            pBlockBytes += 2;
                            break;

                        case BT_uint32:
                            m_result |= WriteText(*(uint32 *)pBlockBytes);
                            pBlockBytes += 4;
                            break;

                        case BT_int32:
                            m_result |= WriteText(*(int32 *)pBlockBytes);
                            pBlockBytes += 4;
                            break;

                        case BT_float32:
                            m_result |= WriteText(*(float32 *)pBlockBytes);
                            pBlockBytes += 4;
                            break;

                        case BT_uint64:
                            m_result |= WriteText(*(uint64 *)pBlockBytes);
                            pBlockBytes += 8;
                            break;

                        case BT_int64:
                            m_result |= WriteText(*(int64 *)pBlockBytes);
                            pBlockBytes += 8;
                            break;

                        case BT_float64:
                            m_result |= WriteText(*(float64 *)pBlockBytes);
                            pBlockBytes += 8;
                            break;

                        default:
                            ERR_RETURN(ES_Error, "Unhandled block data type");
                    }
                }
                m_result |= EndTextLine();
            }
            return m_result;
        }

        case kBinary:
        {
            // determine stride
            int nStride = 0;
            for ( const eBlockDataType *pDataType = &dataTypes[0]; *pDataType != BT_END; ++pDataType )
            {
                nStride += *pDataType & 0xFF;   // data width in lower byte
            }

            if ( CHUNK_BYTE_SWAP_REQUIRED )
            {
                // this code copied from read block; needs work to support byte swapping out of
                // buffer, or simply skip the whole block write thingummie and do as text version
                // above
                ERR_UNIMPLEMENTED(); 

                // byte swap anything larger than a byte
                byte *pBlockBytes = (byte *)pBlock;
                while ( nCount-- )
                {
                    for ( const eBlockDataType *pDataType = &dataTypes[0]; *pDataType != BT_END; ++pDataType )
                    {
                        switch ( *pDataType )
                        {
                        case BT_char:
                        case BT_byte:
                            pBlockBytes += 1;
                            break;

                        case BT_uint16:
                        case BT_int16:
                            stoSwapByteOrder16(pBlockBytes);
                            pBlockBytes += 2;
                            break;

                        case BT_uint32:
                        case BT_int32:
                        case BT_float32:
                            stoSwapByteOrder32(pBlockBytes);
                            pBlockBytes += 4;
                            break;

                        case BT_uint64:
                        case BT_int64:
                        case BT_float64:
                            stoSwapByteOrder64(pBlockBytes);
                            pBlockBytes += 8;
                            break;

                        default:
                            ERR_RETURN(ES_Error, "Unhandled block data type");
                        }
                    }
                }
            }

            // write as a large block
            m_result |= m_file.Write(pBlock, nStride * nCount);
            return m_result;
        }
    }
    return ER_Failure;
}





stoChunkFileReader::stoChunkFileReader( stoFile &file ) : 
    m_file(file),
    m_result(ER_Success),
    m_bValid(false),
    m_nextToken("")
{
    errAssert(file.CanRead());
    errAssert(file.CanSeek());

    ReadHeader();
}

// special constructor for when not using a header
stoChunkFileReader::stoChunkFileReader( stoFile &file, eSerializeMode mode, stoChunk::eEndianMode endianMode ) :
    m_file(file),
    m_result(ER_Success),
    m_mode(mode),
    m_bValid(false),
    m_nextToken("")
{
    errAssert(file.CanRead());
    errAssert(file.CanSeek());

    if ( endianMode != stoChunk::EM_LittleEndian )
    {
        ERR_UNIMPLEMENTED();
    }

    m_bValid = true;
}

void stoChunkFileReader::ReadHeader()
{
    char header[8];
    errResult er = m_file.Read(header);

    // silently fail, since asset manager will use IsValid() to determine whether file is chunk file
    if ( !ISOK(er) || strncmp(header, "CHNK", 4) != 0 )
    {
        m_file.Seek(0);
        return;
    }

    if ( header[4] == 'B' )
        m_mode = kBinary;
    else if ( header[4] == 'T' )
        m_mode = kText;
    else
    {
        ERR_REPORT(ES_Error, "Invalid mode in chunk file header");
        return;
    }

    bool bBigEndian;
    if ( header[5] == 'B' )
        bBigEndian = true;
    else if ( header[5] == 'L' )
        bBigEndian = false;
    else
    {
        ERR_REPORT(ES_Error, "Invalid endian specifier in chunk file header");
        return;
    }

    if ( bBigEndian )
    {
        ERR_UNIMPLEMENTED();
    }

    m_bValid = true;
}

errResult stoChunkFileReader::EnterChunk()
{
    StackEntry chunk;

    if ( m_mode == kBinary )
    {
        m_result |= m_file.Read(chunk.m_header);
        if ( !ISOK(m_result) )
            return ER_Failure;
        chunk.m_chunkEnd = m_file.Tell() + chunk.m_header.m_size;
    }
    else
    {
        string token;
        m_result |= GetToken(token);
        ERR_TEST(m_result, "Error finding chunk token");

        if ( strcasecmp(token.c_str(), "CHUNK") != 0 )
        {
            m_result = ER_Failure;
            ERR_RETURNV(ES_Error, ("Expected \"CHUNK\" keyword but got \"%s\"", token.c_str()));
        }

        m_result |= GetStringToken(token);
        ERR_TESTV(m_result, ("Invalid chunk tag: \"%s\"", token.c_str()));

        if ( token.length() != 4 )
        {
            m_result = ER_Failure;
            ERR_RETURNV(ES_Error, ("Chunk tag not FOURCC: \"%s\"", token.c_str()));
        }

        TAG4 tag = *reinterpret_cast<const TAG4 *>(token.c_str());

#if CORE_BYTEORDER_LITTLE
        // char[4] to long literal is backward on Little Endian machines
        stoSwapByteOrder(tag);
#endif
        chunk.m_header.m_name = tag;

        m_result |= Get(chunk.m_header.m_version);
        ERR_TEST(m_result, "Could not read chunk version");

        m_result |= GetToken(token);
        ERR_TEST(m_result, "Error reading text block marker");

        if ( token != "{" )
        {
            m_result = ER_Failure;
            ERR_RETURNV(ES_Error, ("Expected block marker \"{\" but got: \"%s\"", token.c_str()));
        }
    }
    m_stack.push(chunk);

    return ER_Success;
}

errResult stoChunkFileReader::ExitChunk()
{
    m_result = ER_Success;  // clear error to allow recovery

    if ( m_mode == kBinary )
    {
        m_result |= m_file.Seek(m_stack.top().m_chunkEnd, stoFile::kSeekBegin);
    }
    else
    {
        string token;
        int nLevel = 1;
        while ( nLevel != 0 )
        {
            m_result |= GetToken(token, true);
            ERR_TEST(m_result, "Could not find end of text chunk");

            if ( token == "{" )
                ++nLevel;

            if ( token == "}" )
                --nLevel;

        };
    }
    m_stack.pop();

    return m_result;
}


/**
    Indicates whether the reader is positioned at the end of the current chunk.
    For binary files, this is simply a test of the current file position. For
    text files, we must peek at the next token to see if it is a "}".
**/
bool stoChunkFileReader::EndOfChunk()
{
    switch ( m_mode )
    {
        case kText:
        {
            return PeekNextToken() == "}";
        }
        case kBinary:
        {
            return m_file.Tell() == (int)(m_stack.top().m_chunkEnd);
        }
    }

    ERR_REPORT(ES_Error, "Invalid file mode");
    return true;
}


TAG4 stoChunkFileReader::PeekNextChunkTag()
{
    TAG4 tag = 0;

    // save the current file position and next token
    int nCurrentPos = m_file.Tell();
    string nextToken = m_nextToken;

    switch ( m_mode )
    {
        case kText:
        {
            string token;
            if ( !ISOK(GetToken(token, true)) )
                break;

            if ( token != "CHUNK" )
                break;

            if ( !ISOK(GetStringToken(token)) )
                break;

            if ( token.length() != 4 )
                break;

            tag = *reinterpret_cast<const TAG4 *>(token.c_str());
            stoSwapByteOrder(tag);
            break;
        }
        case kBinary:
            if ( ISOK(EnterChunk()) )
            {
                tag = GetCurrentChunkHeader().m_name;
                m_stack.pop();
            }
            break;
    }

    // restore the file position and next token
    m_file.Seek(nCurrentPos);
    m_nextToken = nextToken;

    return tag;
}


errResult stoChunkFileReader::GetToken( string &s, bool bReadRaw )
{
    if ( !bReadRaw && !ISOK(m_result) )
        return ER_Failure;

    if ( !m_nextToken.empty() )
    {
        s = m_nextToken;
        m_nextToken = "";
    }
    else
    {
        s.resize(0);
        bool inString = false;
        bool inToken = false;
        char delim = '"';
        while ( true )
        {
            char c;
            if ( !ISOK(m_file.Read(c)) )
            {
                // eof reached
                if ( inToken )  
                    break;

                m_result = ER_Failure;
                return m_result;
            }

            if ( inString )
            {
                s += c;
                if ( c == delim )
                    inString = false;
            }
            else
            {
                if ( c == '\'' || c == '"' )
                {
                    delim = c;
                    inString = true;
                    inToken = true;
                }
                else if ( isspace(c) )
                {
                    if ( inToken )
                        break;

                    // ignore whitespace up until the start of the token
                    continue;
                }
                else
                    inToken = true;

                s += c;
            }
        }
    }

    if ( s == "}" && !bReadRaw )
    {
        ERR_REPORT(ES_Error, "Attempt to read past end of chunk. Ignoring reads until chunk exit");
        m_result = ER_Failure;
        m_nextToken = s;
        s = "";
        return ER_Failure;
    }

    return ER_Success;
}


const string& stoChunkFileReader::PeekNextToken()
{
    if ( m_nextToken.empty() )
    {
        GetToken(m_nextToken, true);
    }
    return m_nextToken;
}


// read a string token, removing enclosing "s
errResult stoChunkFileReader::GetStringToken( string &s )
{
    errResult er = GetToken(s);
    if ( !ISOK(er) )
        return ER_Failure;

    if ( s.length() < 2 )
        return ER_Failure;

    if ( s[0] != '"' || s[s.length() - 1] != '"' )
        return ER_Failure;

    s = s.substr(1, s.length() - 2);
    return ER_Success;
}


/**
    Get the current file position to allow restoring later. This function
    added to support stoChunkFrameReader::Rewind().
    
    @return file offset from stoFileBuffer interface
**/
int stoChunkFileReader::GetPos() const
{
    return m_file.Tell(); 
}


errResult stoChunkFileReader::GetBlock( OUT void *pBlock, 
                                        IN const stoChunk::eBlockDataType dataTypes[], 
                                        IN int nCount )
{
    using namespace stoChunk;
    switch ( m_mode )
    {
        case kText:
        {
            byte *pBlockBytes = (byte *)pBlock;
            while ( nCount-- )
            {
                for ( const eBlockDataType *pDataType = &dataTypes[0]; *pDataType != BT_END; ++pDataType )
                {
                    switch ( *pDataType )
                    {
                        case BT_char:
                            m_result |= ReadText(*(char *)pBlockBytes);
                            pBlockBytes += 1;
                            break;

                        case BT_byte:
                            m_result |= ReadText(*(byte *)pBlockBytes);
                            pBlockBytes += 1;
                            break;

                        case BT_uint16:
                            m_result |= ReadText(*(uint16 *)pBlockBytes);
                            pBlockBytes += 2;
                            break;

                        case BT_int16:
                            m_result |= ReadText(*(int16 *)pBlockBytes);
                            pBlockBytes += 2;
                            break;

                        case BT_uint32:
                            m_result |= ReadText(*(uint32 *)pBlockBytes);
                            pBlockBytes += 4;
                            break;

                        case BT_int32:
                            m_result |= ReadText(*(int32 *)pBlockBytes);
                            pBlockBytes += 4;
                            break;

                        case BT_float32:
                            m_result |= ReadText(*(float32 *)pBlockBytes);
                            pBlockBytes += 4;
                            break;

                        case BT_uint64:
                            m_result |= ReadText(*(uint64 *)pBlockBytes);
                            pBlockBytes += 8;
                            break;

                        case BT_int64:
                            m_result |= ReadText(*(int64 *)pBlockBytes);
                            pBlockBytes += 8;
                            break;

                        case BT_float64:
                            m_result |= ReadText(*(float64 *)pBlockBytes);
                            pBlockBytes += 8;
                            break;

                        default:
                            ERR_RETURN(ES_Error, "Unhandled block data type");
                    }
                }
            }
            break;
        }

        case kBinary:
        {
            // determine stride
            int nStride = 0;
            for ( const eBlockDataType *pDataType = &dataTypes[0]; *pDataType != BT_END; ++pDataType )
            {
                nStride += *pDataType & 0xFF;   // data width in lower byte
            }

            // read as a large block
            m_result |= m_file.Read(pBlock, nStride * nCount);
            if ( !ISOK(m_result) )
                return m_result;

            if ( CHUNK_BYTE_SWAP_REQUIRED )
            {
                // byte swap anything larger than a byte
                byte *pBlockBytes = (byte *)pBlock;
                while ( nCount-- )
                {
                    for ( const eBlockDataType *pDataType = &dataTypes[0]; *pDataType != BT_END; ++pDataType )
                    {
                        switch ( *pDataType )
                        {
                            case BT_char:
                            case BT_byte:
                                pBlockBytes += 1;
                                break;

                            case BT_uint16:
                            case BT_int16:
                                stoSwapByteOrder16(pBlockBytes);
                                pBlockBytes += 2;
                                break;

                            case BT_uint32:
                            case BT_int32:
                            case BT_float32:
                                stoSwapByteOrder32(pBlockBytes);
                                pBlockBytes += 4;
                                break;

                            case BT_uint64:
                            case BT_int64:
                            case BT_float64:
                                stoSwapByteOrder64(pBlockBytes);
                                pBlockBytes += 8;
                                break;

                            default:
                                ERR_RETURN(ES_Error, "Unhandled block data type");
                        }
                    }
                }
            }
            return m_result;
        }
    }
    return ER_Failure;
}


/**
    Set the current file position. This function added to support stoChunkFrameReader::Rewind().
    @param  offset file position
    
    @return <edit>
**/
errResult stoChunkFileReader::SetPos( int offset )
{
    m_nextToken = "";

    // verify that the position is within the current chunk
    if ( m_mode == kBinary )
    {
        errAssert(!m_stack.empty());
        StackEntry &chunk = m_stack.top();
        int chunkStart = chunk.m_chunkEnd - chunk.m_header.m_size;
        if ( offset < chunkStart || offset > chunk.m_chunkEnd )
        {
            ERR_RETURN(ES_Error, "Cannot seek outside current chunk frame");
        }
    }

    return m_file.Seek(offset, stoFile::kSeekBegin);
}


// picky string to integer conversion with lots of error checking
errResult stoChunkFileReader::ConvertToInteger( const string &s, void *pValue, int nBytes, eSignType signType )
{
    errAssert(pValue);

    bool bNegative = false;
    uint64 result = 0;

    const char *pChar = s.c_str();
    if ( !*pChar )
        return ER_Failure;

    if ( *pChar == '-' )
    {
        bNegative = true;
        if ( signType == kUnsigned )
            return ER_Failure;
        ++pChar;
    }
    else if ( *pChar == '+' )
    {
        // ignore leading +
    }

    while ( *pChar )
    {
        if ( !isdigit(*pChar) )
            return ER_Failure;

        result = result * 10 + *pChar - '0';
        ++pChar;
    }

    // check for overflow
    switch ( signType )
    {
        case kUnsigned:
            switch ( nBytes )
            {
                case 1:
                    if ( (result & MAKEUINT64(0xFFFFFFFFFFFFFF00)) != 0 )
                        return ER_Failure;
                    break;
                case 2:
                    if ( (result & MAKEUINT64(0xFFFFFFFFFFFF0000)) != 0 )
                        return ER_Failure;
                    break;
                case 4:
                    if ( (result & MAKEUINT64(0xFFFFFFFF00000000)) != 0 )
                        return ER_Failure;
                    break;
                case 8:
                    break;
                default:
                    ERR_RETURN(ES_Error, "Invalid integer length in ConvertToInteger()");
            }
            break;

        case kSigned:
            switch ( nBytes )
            {
                case 1:
                    if ( (result & MAKEUINT64(0xFFFFFFFFFFFFFF80)) != 0 )
                        return ER_Failure;
                    break;
                case 2:
                    if ( (result & MAKEUINT64(0xFFFFFFFFFFFF8000)) != 0 )
                        return ER_Failure;
                    break;
                case 4:
                    if ( (result & MAKEUINT64(0xFFFFFFFF80000000)) != 0 )
                        return ER_Failure;
                    break;
                case 8:
                    if ( (result & MAKEUINT64(0x8000000000000000)) != 0 )
                        return ER_Failure;
                    break;
                default:
                    ERR_RETURN(ES_Error, "Invalid integer length in ConvertToInteger()");
            }

            if ( bNegative )
                result = (uint64)-(int64)result;    // damn, this is ugly
            break;
    }

    // little endian assumption here
#if CORE_BYTEORDER_BIG
    memcpy(pValue, (uint8*)&result + (8 - nBytes), nBytes);
#else
    memcpy(pValue, &result, nBytes);
#endif

    return ER_Success;
}

// Text mode reading methods

errResult stoChunkFileReader::ReadText( OUT bool &t )
{
    string s;
    if ( ISOK(GetToken(s)) && ISOK(ConvertToInteger(s, &t, sizeof(t), kUnsigned)) )
        return ER_Success;

    ERR_RETURNV(ES_Error, ("Expected bool but got \"%s\" in chunk \"%s\"", s.c_str(), GetCurrentChunkName().c_str()));
}

errResult stoChunkFileReader::ReadText( OUT byte &t )
{
    string s;
    if ( ISOK(GetToken(s)) && ISOK(ConvertToInteger(s, &t, sizeof(t), kUnsigned)) )
        return ER_Success;

    ERR_RETURNV(ES_Error, ("Expected byte but got \"%s\" in chunk \"%s\"", s.c_str(), GetCurrentChunkName().c_str()));
}


errResult stoChunkFileReader::ReadText( OUT char &t )
{
    string s;
    if ( ISOK(GetToken(s)) && ISOK(ConvertToInteger(s, &t, sizeof(t), kUnsigned)) )
        return ER_Success;

    ERR_RETURNV(ES_Error, ("Expected char but got \"%s\" in chunk \"%s\"", s.c_str(), GetCurrentChunkName().c_str()));
}

errResult stoChunkFileReader::ReadText( OUT uint16 &t )
{
    string s;
    if ( ISOK(GetToken(s)) && ISOK(ConvertToInteger(s, &t, sizeof(t), kUnsigned)) )
        return ER_Success;

    ERR_RETURNV(ES_Error, ("Expected uint16 but got \"%s\" in chunk \"%s\"", s.c_str(), GetCurrentChunkName().c_str()));
}

errResult stoChunkFileReader::ReadText( OUT int16 &t )
{
    string s;
    if ( ISOK(GetToken(s)) && ISOK(ConvertToInteger(s, &t, sizeof(t), kSigned)) )
        return ER_Success;

    ERR_RETURNV(ES_Error, ("Expected int16 but got \"%s\" in chunk \"%s\"", s.c_str(), GetCurrentChunkName().c_str()));
}

errResult stoChunkFileReader::ReadText( OUT uint32 &t )
{
    string s;
    if ( ISOK(GetToken(s)) && ISOK(ConvertToInteger(s, &t, sizeof(t), kUnsigned)) )
        return ER_Success;

    ERR_RETURNV(ES_Error, ("Expected uint32 but got \"%s\" in chunk \"%s\"", s.c_str(), GetCurrentChunkName().c_str()));
}

errResult stoChunkFileReader::ReadText( OUT int32 &t )
{
    string s;
    if ( ISOK(GetToken(s)) && ISOK(ConvertToInteger(s, &t, sizeof(t), kSigned)) )
        return ER_Success;

    ERR_RETURNV(ES_Error, ("Expected int32 but got \"%s\" in chunk \"%s\"", s.c_str(), GetCurrentChunkName().c_str()));
}

errResult stoChunkFileReader::ReadText( OUT uint64 &t )
{
    string s;
    if ( ISOK(GetToken(s)) && ISOK(ConvertToInteger(s, &t, sizeof(t), kUnsigned)) )
        return ER_Success;

    ERR_RETURNV(ES_Error, ("Expected uint64 but got \"%s\" in chunk \"%s\"", s.c_str(), GetCurrentChunkName().c_str()));
}

errResult stoChunkFileReader::ReadText( OUT int64 &t )
{
    string s;
    if ( ISOK(GetToken(s)) && ISOK(ConvertToInteger(s, &t, sizeof(t), kSigned)) )
        return ER_Success;

    ERR_RETURNV(ES_Error, ("Expected int64 but got \"%s\" in chunk \"%s\"", s.c_str(), GetCurrentChunkName().c_str()));
}

errResult stoChunkFileReader::ReadText( OUT float32 &f )
{
    string s;
    errResult er = GetToken(s);
    if ( ISOK(er) )
    {
        if ( sscanf(s.c_str(), "%f", &f) == 1)
        {
            return ER_Success;
        }
    }
    ERR_RETURNV(ES_Error, ("Expected float32 but got \"%s\" in chunk \"%s\"", s.c_str(), GetCurrentChunkName().c_str()));
}

errResult stoChunkFileReader::ReadText( OUT float64 &f )
{
    string s;
    errResult er = GetToken(s);
    if ( ISOK(er) )
    {
        if ( sscanf(s.c_str(), "%lf", &f) == 1)
        {
            return ER_Success;
        }
    }
    ERR_RETURNV(ES_Error, ("Expected float64 but got \"%s\" in chunk \"%s\"", s.c_str(), GetCurrentChunkName().c_str()));
}

errResult stoChunkFileReader::ReadText( OUT string &s )
{
    if ( ISOK(GetStringToken(s)) )
        return ER_Success;

    ERR_RETURNV(ES_Error, ("Expected string but got \"%s\" in chunk \"%s\"", s.c_str(), GetCurrentChunkName().c_str()));
}

