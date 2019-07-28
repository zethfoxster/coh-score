/*****************************************************************************
    created:    2002/04/01
    copyright:  2002, NCSoft. All Rights Reserved
    author(s):  Peter M. Freese
    
    purpose:    
*****************************************************************************/

#ifndef   INCLUDED_stoChunk_h
#define   INCLUDED_stoChunk_h
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "arda2/core/corFirst.h"
#include "arda2/core/corStlStack.h"
#include "arda2/core/corCast.h"
#include "arda2/core/corStlVector.h"
#include "arda2/storage/stoFile.h"
#include "arda2/storage/stoFileBuffer.h"
#include <stdarg.h>

#define CHUNK_BYTE_SWAP_REQUIRED CORE_BYTEORDER_BIG

#if CHUNK_BYTE_SWAP_REQUIRED
#define CONDITIONALLY_SWAP_BYTE_ORDER(x) stoSwapByteOrder(x)
#else
#define CONDITIONALLY_SWAP_BYTE_ORDER(x) ((void)(x))
#endif


/*
union TAG4
{
    uint32 dword;
    char   chars[4];
};
*/
typedef uint32 TAG4;


inline std::string TagToString(TAG4 tag)
{
    char c[5], *pTagBytes = (char *)&tag;

    c[0] = pTagBytes[3];
    c[1] = pTagBytes[2];
    c[2] = pTagBytes[1];
    c[3] = pTagBytes[0];
    c[4] = '\0';
    std::string s(c);
    errAssert(s.length() == 4);
    return s;
}


// simple byte order conversion functions
inline void stoSwapByteOrder16( IN byte *b )
{
    std::swap(b[0], b[1]);
}
inline void stoSwapByteOrder32( IN byte *b )
{
    std::swap(b[0], b[3]);
    std::swap(b[1], b[2]);
}
inline void stoSwapByteOrder64( IN byte *b )
{
    std::swap(b[0], b[7]);
    std::swap(b[1], b[6]);
    std::swap(b[2], b[5]);
    std::swap(b[3], b[4]);
}

inline void stoSwapByteOrder( IN uint16 &t )
{
    stoSwapByteOrder16(reinterpret_cast<byte *>(&t));
}
inline void stoSwapByteOrder( IN int16 &t )
{
    stoSwapByteOrder16(reinterpret_cast<byte *>(&t));
}
inline void stoSwapByteOrder( IN uint32 &t )
{
    stoSwapByteOrder32(reinterpret_cast<byte *>(&t));
}
inline void stoSwapByteOrder( IN int32 &t )
{
    stoSwapByteOrder32(reinterpret_cast<byte *>(&t));
}
inline void stoSwapByteOrder( IN float32 &t )
{
    stoSwapByteOrder32(reinterpret_cast<byte *>(&t));
}
inline void stoSwapByteOrder( IN uint64 &t )
{
    stoSwapByteOrder64(reinterpret_cast<byte *>(&t));
}
inline void stoSwapByteOrder( IN int64 &t )
{
    stoSwapByteOrder64(reinterpret_cast<byte *>(&t));
}
inline void stoSwapByteOrder( IN float64 &t )
{
    stoSwapByteOrder64(reinterpret_cast<byte *>(&t));
}


namespace stoChunk
{
    struct Header
    {
        TAG4    m_name;
        int32   m_size;     // size of the chunk, not including the header
        uint32  m_version;
        int32   m_reserved; // possible codec?
    };

    enum eBlockDataType
    {
        // upper byte is unique, lower byte is size
        BT_END      = 0x0000,
        BT_byte     = 0x0101,
        BT_uint16   = 0x0202,
        BT_int16    = 0x0302,
        BT_uint32   = 0x0404,
        BT_int32    = 0x0504,
        BT_uint64   = 0x0608,
        BT_int64    = 0x0708,
        BT_float32  = 0x0804,
        BT_float64  = 0x0908,
        BT_char     = 0x0a01,
    };

    enum eEndianMode
    {
        EM_LittleEndian,
        EM_BigEndian,
    };
};


class stoChunkFileWriter
{
public:
    enum eSerializeMode
    {
        SM_Binary,
        SM_Text,

        // deprecated defines
        kBinary = SM_Binary,
        kText = SM_Text,
    };

    enum eHeaderMode
    {
        HM_WriteHeader,
        HM_NoHeader
    };

    stoChunkFileWriter( stoFile &file, eSerializeMode mode = SM_Binary, 
        stoChunk::eEndianMode endianMode = stoChunk::EM_LittleEndian, 
        eHeaderMode headerMode = HM_WriteHeader );

    // put methods
    template <typename T>
    errResult Put( IN const T &t )
    {
        if ( m_mode == kBinary )
            m_result |= WriteBinary(t);
        else
        {
            m_result |= WriteText(t);
            m_result |= EndTextLine();
        }
        return m_result;
    }

    template <typename T>
    errResult PutArray( IN const std::vector<T>& vec )
    {
        m_result |= Put(std::size32(vec));
        if(vec.size() > 0)
            m_result |= PutArray(&vec[0], std::size32(vec));
        return m_result;
    }

    template <typename T>
    errResult PutArray( IN const T *t, int nCount ) 
    {
        if ( m_mode == kBinary )
        {
            if ( CHUNK_BYTE_SWAP_REQUIRED && sizeof(T) > 1 )
            {
                while ( --nCount >= 0 )
                {
                    m_result |= WriteBinary(*t++);
                }
            }
            else
            {
                return m_file.Write(t, sizeof(T) * nCount);
            }
        }
        else
        {
            while ( --nCount >= 0 )
            {
                m_result |= WriteText(*t++);
            }
            m_result |= EndTextLine();
        }
        return ER_Success;
    }
    errResult Put( IN const void *p, size_t size )
    {
        return PutArray(static_cast<const byte *>(p), (int)size);
    }

    errResult PutBlock( IN const void *pBlock, IN const stoChunk::eBlockDataType dataTypes[], 
        IN int nCount );

    errResult GetResult() const { return m_result; }

    errResult EnterChunk( TAG4 tag, int nVersion);
    errResult ExitChunk();

protected:
    // Print-like output
    errResult WriteFormattedText( const char *fmt, ... );

    errResult WriteIndent();
    errResult EndTextLine();

    errResult WriteText( bool u );
    errResult WriteText( byte b );
    errResult WriteText( uint16 u );
    errResult WriteText( int16 i );
    errResult WriteText( uint32 u );
    errResult WriteText( int32 i );
    errResult WriteText( uint64 u );
    errResult WriteText( int64 i );
    errResult WriteText( float32 f );
    errResult WriteText( float64 f );
    errResult WriteText( const char* s );
    errResult WriteText( const std::string& s );
    errResult WriteBinary( IN bool t )
    {
        return m_file.Write((byte)t);
    }
    errResult WriteBinary( IN byte t )
    {
        return m_file.Write(t);
    }
    errResult WriteBinary( IN char t )
    {
        return m_file.Write(t);
    }
    errResult WriteBinary( IN uint16 t )
    {
        CONDITIONALLY_SWAP_BYTE_ORDER(t);
        return m_file.Write(t);
    }
    errResult WriteBinary( IN int16 t )
    {
        CONDITIONALLY_SWAP_BYTE_ORDER(t);
        return m_file.Write(t);
    }
    errResult WriteBinary( IN uint32 t )
    {
        CONDITIONALLY_SWAP_BYTE_ORDER(t);
        return m_file.Write(t);
    }
    errResult WriteBinary( IN int32 t )
    {
        CONDITIONALLY_SWAP_BYTE_ORDER(t);
        return m_file.Write(t);
    }
    errResult WriteBinary( IN uint64 t )
    {
        CONDITIONALLY_SWAP_BYTE_ORDER(t);
        return m_file.Write(t);
    }
    errResult WriteBinary( IN int64 t )
    {
        CONDITIONALLY_SWAP_BYTE_ORDER(t);
        return m_file.Write(t);
    }
    errResult WriteBinary( IN float32 t )
    {
        CONDITIONALLY_SWAP_BYTE_ORDER(t);
        return m_file.Write(t);
    }
    errResult WriteBinary( IN float64 t )
    {
        CONDITIONALLY_SWAP_BYTE_ORDER(t);
        return m_file.Write(t);
    }
    errResult WriteBinary( IN const char *s )
    {
        errAssert(s != NULL);
        return m_file.WriteString(s);
    }
    errResult WriteBinary( IN const std::string &s )
    {
        return m_file.WriteString(s);
    }

private:
    stoChunkFileWriter( const stoChunkFileWriter& );
    stoChunkFileWriter& operator=( const stoChunkFileWriter& );

    void WriteHeader();

    stoFile                 &m_file;
    errResult               m_result;

    struct StackEntry
    {
        stoChunk::Header    m_header;
        int                 m_chunkStart;   // offset of chunk header in file
    };
    std::stack<StackEntry>       m_stack;

    eSerializeMode          m_mode;

    // text mode state information
    static const int        kChunkIndentation = 4;
    static const int        kMaxLineWidth = 120;
    int                     m_column;
};


class stoChunkFileReader
{
public:
    enum eSerializeMode
    {
        SM_Binary,
        SM_Text,

        // deprecated defines
        kBinary = SM_Binary,
        kText = SM_Text,
    };

    stoChunkFileReader( stoFile &file );

    // special constructor for when not using a headers
    stoChunkFileReader( stoFile &file, eSerializeMode mode, stoChunk::eEndianMode endianMode );

    bool IsValid() const { return m_bValid; }

    // get methods
    template <typename T>
        errResult Get( OUT T &t )
    {
        if ( m_mode == kBinary )
            return ReadBinary(t);
        else
        {
            return ReadText(t);
        }
        return ER_Success;
    }

    template <typename T>
    errResult GetArray( OUT std::vector<T>& vec )
    {
        uint size;
        m_result |= Get(size);
        if(size > 0)
        {
            vec.resize(size);
            m_result |= GetArray(&vec[0], size);
        }
        return m_result;
    }

    template <typename T>
        errResult GetArray( OUT T *t, int nCount ) 
    {
        if ( m_mode == kBinary )
        {
            if ( (CHUNK_BYTE_SWAP_REQUIRED && sizeof(T) > 1) || (sizeof(T) == 4 && nCount < 16) )
            {
                while ( --nCount >= 0 )
                {
                    ReadBinary(*t++);
                }
            }
            else
            {
                return m_file.Read(t, sizeof(T) * nCount);
            }
        }
        else
        {
            while ( --nCount >= 0 )
            {
                ReadText(*t++);
            }
        }
        return ER_Success;
    }

    errResult GetBlock( OUT void *pBlock, IN const stoChunk::eBlockDataType dataTypes[], 
        IN int nCount );

    bool ReadError() const { return !ISOK(m_result); }
    errResult GetResult() const { return m_result; }

    errResult EnterChunk();
    errResult ExitChunk();

    bool EndOfChunk();

    const stoChunk::Header& GetCurrentChunkHeader() const
    {
        errAssert(!m_stack.empty());
        return m_stack.top().m_header;
    }
    std::string GetCurrentChunkName()
    {
        return TagToString(GetCurrentChunkHeader().m_name);
    }

    TAG4 PeekNextChunkTag();

protected:
    // fetch the next token with error and chunk frame checking
    errResult GetToken( std::string &s, bool bReadRaw = false );

    const std::string& PeekNextToken();

    // read a string token, removing enclosing "s
    errResult GetStringToken( std::string &s );

    // these functions support Rewind operation on chunks (see stoChunkFrameReader)
    int GetPos() const;
    errResult SetPos( int offset );
    friend class stoChunkFrameReader;

    enum eSignType
    {
        kUnsigned = 0,
        kSigned = 1,
    };
    errResult ConvertToInteger( const std::string &s, void *pValue, int nBytes, eSignType signType );

    // text mode reading methods
    errResult ReadText( OUT bool &t );
    errResult ReadText( OUT byte &t );
    errResult ReadText( OUT char &t );
    errResult ReadText( OUT uint16 &t );
    errResult ReadText( OUT int16 &t );
    errResult ReadText( OUT uint32 &t );
    errResult ReadText( OUT int32 &t );
    errResult ReadText( OUT uint64 &t );
    errResult ReadText( OUT int64 &t );
    errResult ReadText( OUT float32 &f );
    errResult ReadText( OUT float64 &f );
    errResult ReadText( OUT std::string &s );

    // binary mode reading methods
    errResult ReadBinary( OUT bool &t )
    {
        // sizeof(bool) is undefined, so we must choose a serialization size
        byte b;
        errResult er = m_file.Read(b);
        t = force_cast<bool>(b);
        return er;
    }
    errResult ReadBinary( OUT byte &t )
    {
        return m_file.Read(t);
    }
    errResult ReadBinary( OUT char &t )
    {
        return m_file.Read(t);
    }
    errResult ReadBinary( OUT uint16 &t )
    {
        errResult er = m_file.Read(t);
        CONDITIONALLY_SWAP_BYTE_ORDER(t);
        return er;
    }
    errResult ReadBinary( OUT int16 &t )
    {
        errResult er = m_file.Read(t);
        CONDITIONALLY_SWAP_BYTE_ORDER(t);
        return er;
    }
    errResult ReadBinary( OUT uint32 &t )
    {
        errResult er = m_file.Read(t);
        CONDITIONALLY_SWAP_BYTE_ORDER(t);
        return er;
    }
    errResult ReadBinary( OUT int32 &t )
    {
        errResult er = m_file.Read(t);
        CONDITIONALLY_SWAP_BYTE_ORDER(t);
        return er;
    }
    errResult ReadBinary( OUT uint64 &t )
    {
        errResult er = m_file.Read(t);
        CONDITIONALLY_SWAP_BYTE_ORDER(t);
        return er;
    }
    errResult ReadBinary( OUT int64 &t )
    {
        errResult er = m_file.Read(t);
        CONDITIONALLY_SWAP_BYTE_ORDER(t);
        return er;
    }
    errResult ReadBinary( OUT float32 &t )
    {
        errResult er = m_file.Read(t);
        CONDITIONALLY_SWAP_BYTE_ORDER(t);
        return er;
    }
    errResult ReadBinary( OUT float64 &t )
    {
        errResult er = m_file.Read(t);
        CONDITIONALLY_SWAP_BYTE_ORDER(t);
        return er;
    }
    errResult ReadBinary( OUT std::string &s )
    {
        return m_file.ReadString(s);
    }

private:
    stoChunkFileReader( const stoChunkFileReader& );
    stoChunkFileReader& operator=( const stoChunkFileReader& );

    void ReadHeader();

    stoFileBuffer           m_file;
    errResult               m_result;

    struct StackEntry
    {
        stoChunk::Header    m_header;
        int                 m_chunkEnd; // offset of end of chunk in file
    };
    std::stack<StackEntry>       m_stack;

    eSerializeMode          m_mode;
    bool                    m_bValid;
    std::string                  m_nextToken;
};


class stoChunkFrameWriter
{
public:
    explicit stoChunkFrameWriter( stoChunkFileWriter &writer, TAG4 tag, uint nVersion ) :
        m_writer(writer)
    {
        m_writer.EnterChunk(tag, nVersion);
    }

    ~stoChunkFrameWriter()
    {
        m_writer.ExitChunk();
    }

private:
    stoChunkFrameWriter( const stoChunkFrameWriter& );
    stoChunkFrameWriter& operator=( const stoChunkFrameWriter& );
    stoChunkFileWriter  &m_writer;
};


class stoChunkFrameReader
{
public:
    explicit stoChunkFrameReader( stoChunkFileReader &reader ) :
        m_reader(reader)
    {
        m_valid = ISOK(m_reader.EnterChunk());
        m_frameDataPos = m_reader.GetPos();
        m_tag = m_reader.GetCurrentChunkHeader().m_name;
        m_version = m_reader.GetCurrentChunkHeader().m_version;
    }

    ~stoChunkFrameReader()
    {
        if ( m_valid )
            m_reader.ExitChunk();
    }

    TAG4 GetTag() const { return m_tag; }
    uint GetVersion() const { return m_version; }

    // Rewind to beginning of frame
    errResult Rewind() const
    {
        return m_reader.SetPos(m_frameDataPos);
    }

private:
    stoChunkFrameReader( const stoChunkFrameReader& );
    stoChunkFrameReader& operator=( const stoChunkFrameReader& );

    stoChunkFileReader  &m_reader;
    int                 m_frameDataPos;
    bool                m_valid;
    TAG4                m_tag;
    uint                m_version;
};


#endif // INCLUDED_stoChunk_h
