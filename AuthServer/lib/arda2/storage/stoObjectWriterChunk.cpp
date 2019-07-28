#include "arda2/core/corFirst.h"
#include "stoFirst.h"

#include "stoObjectChunk.h"
#include "stoObjectWriterChunk.h"

#include "arda2/core/corStdString.h"
#include "arda2/properties/proObject.h"
#include "arda2/properties/proProperty.h"
#include "arda2/properties/proPropertyNative.h"

#include "arda2/storage/stoChunk.h"
#include "arda2/core/corAssert.h"

using namespace std;

PRO_REGISTER_CLASS(stoObjectWriterChunk, proObjectWriter)


stoObjectWriterChunk::stoObjectWriterChunk( stoChunkFileWriter* inWriter ) :
    proObjectWriter( 0 ),
    m_writer( inWriter )
{
}

stoObjectWriterChunk::~stoObjectWriterChunk()
{
}

// invoked externally to write a object out
void stoObjectWriterChunk::WriteObject(const proObject *obj)
{
    if ( obj && m_writer)
    {
        // build a frame for this object
        stoChunkFrameWriter frameWriter( *m_writer,
            STO_CHUNK_OBJECT_TAG, stoChunkObjectVersion );

        // and then write it out
        proObjectWriter::InternalWriteObject( obj );
    }
}

// invoked from within proObject or derived classes
bool stoObjectWriterChunk::CheckReference(const proObject *obj)
{
    RefMap::const_iterator foundIt =
        m_objectReferences.find( obj );
    if ( foundIt == m_objectReferences.end() )
    {
        // it's a new object
        m_objectReferences.insert( RefMap::value_type(
            obj, ( uint )m_objectReferences.size() ) );

        // start out with a tag before the frame, so that
        // the object can properly recognize what is being
        // serialized
        m_writer->Put( ( uint )kObjectTag );

        // force the caller to serialize the object themselves
        return false;
    }

    // add a tag to let the reader know this is an object reference
    m_writer->Put( ( uint )kObjectReferenceTag );

    // just serialize an index value
    m_writer->Put( foundIt->second );

    return true;
}

// invoked when NOT a reference
void stoObjectWriterChunk::BeginObject(const string &className, const int version)
{
    errAssert( m_writer );

    // write out the classname, so the reader will know what to build
    m_writer->Put( className );

    // write out the version number for comparison (this should
    // go in the frame, but it's already been written)
    m_writer->Put( version );
}

void stoObjectWriterChunk::PutPropValue(const string &name, const int value)
{
    errAssert( m_writer );

    // label this as an integer so that dynamic
    // properties will parse safely
    m_writer->Put( ( uint )kChunkInt );

    m_writer->Put( name );
    m_writer->Put( value );
}

void stoObjectWriterChunk::PutPropValue(const string &name, const bool value)
{
    errAssert( m_writer );

    // label this as a Boolean so that dynamic
    // properties will parse safely
    m_writer->Put( ( uint )kChunkBool );

    m_writer->Put( name );
    m_writer->Put( value );
}

void stoObjectWriterChunk::PutPropValue(const string &name, const string &value)
{
    errAssert( m_writer );

    // label this as a string so that dynamic
    // properties will parse safely
    m_writer->Put( ( uint )kChunkString );

    m_writer->Put( name );
    m_writer->Put( value );
}

void stoObjectWriterChunk::PutPropValue(const string &name, const wstring &value)
{
    errAssert( m_writer );

    ERR_REPORTV(ES_Info, ("Wide strings are not supported <%s, %S>", name.c_str(), (char*)value.c_str()));
}

void stoObjectWriterChunk::PutPropValue(const string &name, const float value)
{
    errAssert( m_writer );

    // label this as a floating point number so that dynamic
    // properties will parse safely
    m_writer->Put( ( uint )kChunkFloat );

    m_writer->Put( name );
    m_writer->Put( value );
}

void stoObjectWriterChunk::PutPropValue(const string &name, const uint value)
{
    errAssert( m_writer );

    // label this as an unsigned integer so that dynamic
    // properties will parse safely
    m_writer->Put( ( uint )kChunkUint );

    m_writer->Put( name );
    m_writer->Put( value );
}

void stoObjectWriterChunk::PutPropValue(const string &name, const proObject *obj)
{
    errAssert( m_writer );

    // object properties are written out differently (tag,
    // then name, then object stuff) in order to help
    // with analysis when loading them back in

    // tag
    if ( obj )
    {
        RefMap::const_iterator foundIt =
            m_objectReferences.find( obj );
        if ( foundIt == m_objectReferences.end() )
        {
            m_writer->Put( ( uint )kSubObjectTag );
        }
        else
        {
            m_writer->Put( ( uint )kSubObjectReferenceTag );
        }
    }
    else
    {
        m_writer->Put( ( uint )kNULLTag );
    }


    // label this as an object so that dynamic
    // properties will parse safely
    m_writer->Put( ( uint )kChunkObject );

    // put name
    m_writer->Put( name );

    // put object
    if ( obj )
    {
        WriteObject( obj );
    }
}

// used to write out additional types as strings
void stoObjectWriterChunk::PutPropTyped(const string &name,
    const string &value, const string &typeName)
{
    errAssert( m_writer );

    m_writer->Put( name );
    m_writer->Put( value );
    m_writer->Put( typeName );
}


void stoObjectWriterChunk::EndObject()
{
    // end the object with a NULL tag,
    // so that the reader will know where
    // the dynamic properties end
    m_writer->Put( kNULLTag );
}


// Unit Test Code //
#if BUILD_UNIT_TESTS
    #include "arda2/properties/proClassNative.h"
    #include "arda2/properties/proPropertyNative.h"
    #include "arda2/storage/stoObjectReaderChunk.h"
    #include "arda2/storage/stoFileOSFile.h"
    #include "arda2/test/tstUnit.h"

    // just your basic, testable object
    class tstWritableObject : public proObject
    {
        PRO_DECLARE_OBJECT
    public:
        tstWritableObject( void ) :
            m_bool( false ),
            m_float( 0 ),
            m_int( 0 ),
            m_object( 0 ),
            m_objectB( 0 ),
            m_string( "" )
        { }

        ~tstWritableObject()
        {
            SafeDelete( m_objectB ); // object B is owned by this class
        }

        bool GetBool( void ) const { return m_bool; }
        float GetFloat( void ) const { return m_float; }
        int GetInt( void ) const { return m_int; }
        proObject* GetObject( void ) const { return m_object; }
        proObject* GetObjectB( void ) const { return m_objectB; }
        string GetString( void ) const { return m_string; }

        void SetBool( bool inBool ) { m_bool = inBool; }
        void SetFloat( float inFloat ) { m_float = inFloat; }
        void SetInt( int inInt ) { m_int = inInt; }
        void SetObject( const proObject* inObject ) { m_object = const_cast< proObject* >( inObject ); }
        void SetObjectB( const proObject* inObject ) { m_objectB = const_cast< proObject* >( inObject ); }
        void SetString( const string& inString ) { m_string = inString; }

    protected:

    private:
        bool       m_bool;
        float      m_float;
        int        m_int;
        proObject* m_object;
        proObject* m_objectB; // this object is owned by this class
        string     m_string;
    };

    class tstWritableObjectB : public proObject
    {
        PRO_DECLARE_OBJECT
    public:
        tstWritableObjectB() { }

    protected:

    private:
    };

    PRO_REGISTER_CLASS( tstWritableObject, proObject )
    REG_PROPERTY_BOOL( tstWritableObject, Bool )
    REG_PROPERTY_FLOAT( tstWritableObject, Float )
    REG_PROPERTY_INT( tstWritableObject, Int )
    REG_PROPERTY_OBJECT( tstWritableObject, Object, proObject )
    REG_PROPERTY_OBJECT( tstWritableObject, ObjectB, proObject )
    REG_PROPERTY_STRING( tstWritableObject, String )

    PRO_REGISTER_CLASS( tstWritableObjectB, proObject )

    const bool testBool =     true;
    const float testFloat =   -14.763f;
    const int testInt =       42;
    const string testString = "Testing.  1, 2, 3.";

    const string child1Name = "TestChild";
    const string child2Name = "testChild2";

    class stoObjectWriterChunkTests : public tstUnit
    {
    public:
        stoObjectWriterChunkTests() { };

        void Register()
        {
            SetName("stoObjectWriterChunk");
            AddTestCase( "TestObjectWriteAndReadBinary", &stoObjectWriterChunkTests::TestObjectWriteAndReadBinary );
            AddTestCase( "TestObjectWriteAndReadText", &stoObjectWriterChunkTests::TestObjectWriteAndReadText );
        }

        void TestObjectWrite( stoChunkFileWriter* outFile ) const
        {
            // build a test object
            tstWritableObject testObject;
            testObject.SetBool( testBool );
            testObject.SetFloat( testFloat );
            testObject.SetInt( testInt );
            testObject.SetObject( &testObject );
            testObject.SetString( testString );

            tstWritableObjectB* testObject2 = new tstWritableObjectB();
            testObject.SetObjectB( testObject2 );
  
            tstWritableObjectB* testObject3 = new tstWritableObjectB();

            // it's also important to test dynamic children
            testObject.AddChild( testObject2, child1Name, DP_refValue );
            testObject.AddChild( testObject3, child2Name, DP_ownValue );

            // dynamic boolean
            AddInstanceProperty( &testObject, "InstancedBool", true );
            AddInstanceProperty( &testObject, "InstancedFloat", 97.6261f );
            AddInstanceProperty( &testObject, "InstancedInt", 1701 );
            string stringValue( "This is a string instance." );
            AddInstanceProperty( &testObject, "InstancedString", stringValue );
            AddInstanceProperty( &testObject, "InstancedUInt", ( uint )0x10101 );

            // write it out
            stoObjectWriterChunk testWriter( outFile );

            testWriter.WriteObject( &testObject );
        }

        void TestObjectRead( stoChunkFileReader* inFile ) const
        {
            // and then read it back in
            stoObjectReaderChunk testReader( inFile );

            // really, we need to make make this call with
            // the reader reading the object
            DP_OWN_VALUE ownValue;
            proObject* readObject = testReader.ReadObject( ownValue );

            TESTASSERT( proClassRegistry::InstanceOf(
                readObject->GetClass(), "tstWritableObject" ) );
            if ( proClassRegistry::InstanceOf( readObject->GetClass(), "tstWritableObject" ) )
            {
                tstWritableObject* actualObject =
                    ( tstWritableObject* )readObject;
                TESTASSERT( actualObject->GetBool() == testBool );
                TESTASSERT( actualObject->GetFloat() == testFloat );
                TESTASSERT( actualObject->GetInt() == testInt );
                TESTASSERT( actualObject->GetObject() == actualObject );
                TESTASSERT( actualObject->GetObjectB() != 0 );
                TESTASSERT( actualObject->GetObjectB() != actualObject );
                if ( actualObject->GetObjectB() )
                {
                    TESTASSERT( proClassRegistry::InstanceOf(
                        actualObject->GetObjectB()->GetClass(), "tstWritableObjectB" ) );
                }
                TESTASSERT( actualObject->GetString() == testString );

                vector< proObject* > myChildren;
                actualObject->GetChildren( myChildren );
                TESTASSERT( myChildren.size() == 4 );

                TESTASSERT( actualObject->GetPropertyAt( 0 )->GetName() == child1Name );
                TESTASSERT( actualObject->GetPropertyAt( 1 )->GetName() == child2Name );
            }
            TESTASSERT( ownValue == DP_ownValue );

            SafeDelete( readObject );
        }

        void TestObjectWriteAndReadBinary( void ) const
        {
            // write out the object
            stoFileOSFile outFile;
            TESTASSERT( ISOK( outFile.Open( "testObjectFile.bin",
                stoFileOSFile::kAccessCreate ) ) );
            stoChunkFileWriter testFileWriter( outFile );
            TestObjectWrite( &testFileWriter );
            outFile.Close();

            // read in the object
            stoFileOSFile inFile;
            TESTASSERT( ISOK( inFile.Open( "testObjectFile.bin",
                stoFileOSFile::kAccessRead ) ) );
            stoChunkFileReader testFileReader( inFile );
            TestObjectRead( &testFileReader );
            inFile.Close();
        }

        void TestObjectWriteAndReadText( void ) const
        {
            // write out the object
            stoFileOSFile outFile;
            TESTASSERT( ISOK( outFile.Open( "testObjectFile.txt",
                stoFileOSFile::kAccessCreate ) ) );
            stoChunkFileWriter testFileWriter( outFile, stoChunkFileWriter::SM_Text );
            TestObjectWrite( &testFileWriter );
            outFile.Close();

            // read in the object
            stoFileOSFile inFile;
            TESTASSERT( ISOK( inFile.Open( "testObjectFile.txt",
                stoFileOSFile::kAccessRead ) ) );
            //stoChunkFileReader testFileReader( inFile,
            //    stoChunkFileReader::SM_Text, stoChunk::EM_LittleEndian );
            stoChunkFileReader testFileReader( inFile );
            TestObjectRead( &testFileReader );
            inFile.Close();
        }

    protected:

    private:
    };

    EXPORTUNITTESTOBJECT( stoObjectWriterChunkTests );
#endif
