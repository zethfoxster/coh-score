/**
 *  property system
 *
 *  @author Sean Riley
 *
 *  @date 1/13/2003
 *
 *  @file
**/
#include "arda2/core/corFirst.h"
#include "stoFirst.h"

#include "stoObjectChunk.h"
#include "stoObjectReaderChunk.h"

#include "arda2/properties/proClassRegistry.h"
#include "arda2/properties/proObject.h"
#include "arda2/properties/proProperty.h"
#include "arda2/properties/proPropertyOwner.h"
#include "arda2/properties/proPropertyNative.h"

#include "arda2/storage/stoChunk.h"

using namespace std;

PRO_REGISTER_CLASS(stoObjectReaderChunk, proObjectReader)

stoObjectReaderChunk::stoObjectReaderChunk( stoChunkFileReader* inReader ) :
    proObjectReader( 0 ),
    m_reader( inReader ),
    m_parsingDynamicProperty( false )
{
}

stoObjectReaderChunk::~stoObjectReaderChunk()
{
}

proObject* stoObjectReaderChunk::ReadObject(OUT DP_OWN_VALUE &owner)
{
    if (!m_reader) return NULL;
    // parse the frame
    stoChunkFrameReader frameReader( *m_reader );

    // make sure the tag matches
    if ( frameReader.GetTag() != STO_CHUNK_OBJECT_TAG )
    {
        ERR_REPORTV( ES_Error, ( "Bad object tag" ) );

        return 0;
    }

    // save out the version (not sure why)
    if ( frameReader.GetVersion() != stoChunkObjectVersion )
    {
        ERR_REPORTV( ES_Error, ( "Invalid Frame version <%x>", frameReader.GetVersion() ) );

        return 0;
    }

    // read the object tag
    uint tag;
    m_reader->Get( tag );
    if ( ( uint )kObjectTag == tag )
    {
        // read in the classname
        if ( !ISOK( m_reader->Get( m_currentClassName ) ) || m_currentClassName == "" )
        {
            ERR_REPORTV( ES_Error, ( "Unable to read object's classname." ) );
            return 0;
        }

        // build the new class
        proObject* newObject = proClassRegistry::NewInstance( m_currentClassName );

        // add it to the reference list before we create it,
        // so that we safely resolve any circular references
        m_objectReferences.insert( RefMap::value_type(
            ( uint )m_objectReferences.size(), newObject ) );

        // then parse in the object
        proObjectReader::InternalReadObject( newObject );

        // done!
        owner = DP_ownValue;
        return newObject;
    }
    else if ( ( uint )kObjectReferenceTag == tag )
    {
        // parse in the reference
        uint index;
        if ( ISOK( m_reader->Get( index ) ) )
        {
            // and find it in the list of references
            RefMap::const_iterator target = m_objectReferences.find( index );
            if ( target == m_objectReferences.end() )
            {
                ERR_REPORTV( ES_Error, ( "Object reference not found." ) );
                return 0;
            }

            owner = DP_refValue;
            return target->second;
        }
        else
        {
            // error reading the object
            ERR_REPORTV( ES_Error, ( "Unable to serialize object reference." ) );
            return 0;
        }
    }

    ERR_REPORTV( ES_Error, ( "Unknown tag type <%x>", tag ) );
    return 0;
}

proProperty *stoObjectReaderChunk::ReadDynamicProperty(proObject *object)
{
    // we will now find either a dynamic object tag, or a null tag
    uint tag;
    m_reader->Get( tag );
    errAssert( tag == ( uint )kNULLTag ||
        tag == ( uint )kSubObjectTag ||
        tag == ( uint )kSubObjectReferenceTag ||
        ( tag & ( uint )kChunkBase ) == ( uint )kChunkBase );

    // if the object is a simple type (boolean, etc, it's tag will
    // also be its type tag
    if ( ( tag & ( uint )kChunkBase ) == ( uint )kChunkBase )
    {
        // read the object's name
        string propertyName;
        if ( !ISOK( m_reader->Get( propertyName ) ) )
        {
            ERR_REPORTV( ES_Error, ( "Unable to read dynamic property's name." ) );
            return 0;
        }

        // switch this flag on
        m_parsingDynamicProperty = true;

        // build an object property and read in the data for it
        proProperty* returnValue = 0;
        switch ( tag )
        {
        case ( uint )kChunkBool:
            returnValue = new proPropertyBoolOwner();
            break;
        case ( uint )kChunkFloat:
            returnValue = new proPropertyFloatOwner();
            break;
        case ( uint )kChunkInt:
            returnValue = new proPropertyIntOwner();
            break;
        case ( uint )kChunkObject:
            ERR_REPORTV( ES_Error, ( "Found an unexpected object chunk" ) );
            break;
        case ( uint )kChunkString:
            returnValue = new proPropertyStringOwner();
            break;
        case ( uint )kChunkUint:
            returnValue = new proPropertyUintOwner();
            break;
        case ( uint )kChunkWstring:
        default:
            ERR_REPORTV( ES_Error, ( "Unable to read unsupported tag type <%x>", tag ) );
            break;
        }

        // build the new property
        if ( returnValue )
        {
            returnValue->SetName( propertyName );
            returnValue->Read( this, object );
        }

        // and turn it of, j.i.c.
        m_parsingDynamicProperty = false;

        return returnValue;
    }
    else if ( tag == ( uint )kSubObjectTag )
    {
        // read in the type tag.  Now this tag is really
        // important, because it will tell us what type
        // of property we need to construct
        uint typeTag;
        if ( !ISOK( m_reader->Get( typeTag ) ) )
        {
            ERR_REPORTV( ES_Error, ( "Unable to read dynamic property's type." ) );
            return 0;
        }

        // read the object's name
        string propertyName;
        if ( !ISOK( m_reader->Get( propertyName ) ) )
        {
            ERR_REPORTV( ES_Error, ( "Unable to read dynamic property's name." ) );
            return 0;
        }

        // valid type type
        errAssert( typeTag == ( uint )kChunkObject );
        if ( typeTag != ( uint )kChunkObject )
        {
            return 0;
        }

        // switch this flag on (tells subsequent read calls that
        // the object is being loaded dynamically, which alters
        // what data must be parsed)
        m_parsingDynamicProperty = true;

        // build an object property and read in the data for it
        proPropertyObjectOwner* newProperty =
            new proPropertyObjectOwner();
        newProperty->SetName( propertyName );
        newProperty->Read( this, object );

        // and turn it of, j.i.c.
        m_parsingDynamicProperty = false;

        return newProperty;
    }
    else if ( tag == ( uint )kSubObjectReferenceTag )
    {
        // we'd better find an object chunk tag
        uint chunktag;
        if ( !ISOK( m_reader->Get( chunktag ) ) || chunktag != ( uint )kChunkObject )
        {
            return 0;
        }

        // read the object's name
        string propertyName;
        if ( !ISOK( m_reader->Get( propertyName ) ) )
        {
            ERR_REPORTV(ES_Error, ("Unable to read dynamic property's name."));
            return 0;
        }

        // next we expect to find a frame
        stoChunkFrameReader frameReader( *m_reader );
        if ( frameReader.GetTag() != STO_CHUNK_OBJECT_TAG )
        {
            ERR_REPORTV(ES_Error, ("Did not find object tag for <%s>.", propertyName.c_str() ) );
            return 0;
        }
        if ( frameReader.GetVersion() != stoChunkObjectVersion )
        {
            ERR_REPORTV( ES_Error, ( "Object <%s> has an unexpected version number.", propertyName.c_str()));
            return 0;
        }

        // parse the reference data
        uint tag;
        if ( !ISOK( m_reader->Get( tag ) ) )
        {
            ERR_REPORTV( ES_Error, ( "Unable to read dynamic property's sub-tag." ) );
            return 0;
        }
        errAssert( tag == ( uint )kObjectReferenceTag );
        uint index;
        if ( !ISOK( m_reader->Get( index ) ) )
        {
            ERR_REPORTV( ES_Error, ( "Unable to read dynamic property's reference index." ) );
            return 0;
        }

        // pass back the reference by taking the object and
        // stuffing into the propert type of property
        RefMap::const_iterator foundIt = m_objectReferences.find( index ); 
        if ( foundIt == m_objectReferences.end() )
        {
            ERR_REPORTV( ES_Error, ( "Unknown dynamic property reference." ) );
            return 0;
        }
        proObject* propertyValue = foundIt->second;
        proPropertyObjectOwner* newProperty = new proPropertyObjectOwner();
        if ( !newProperty )
        {
            ERR_REPORTV( ES_Error, ( "Out of memory (allocating dynamic property)." ) );
            return 0;
        }
        newProperty->SetName( propertyName );
        newProperty->SetValue( object, propertyValue );

        return newProperty;
    }

    return 0;
}

// invoked from within proObject or derived classes
errResult stoObjectReaderChunk::BeginObject(const string &className, int &version)
{
    errAssert( m_reader );

    // make sure the class names match
    if ( m_currentClassName != className )
    {
        ERR_REPORT( ES_Error, "Class names don't match" );

        return ER_Failure;
    }

    // parse the object's version number
    if ( !ISOK( m_reader->Get( version ) ) )
    {
        ERR_REPORT( ES_Error, "Unable to parse object version number" );

        return ER_Failure;
    }

    // done!
    return ER_Success;
}

void stoObjectReaderChunk::GetPropValue(const string &name, int &value)
{
    errAssert( m_reader );

    // when parsing a dynamic property, we can skip reading in
    // the tag and name, since we've already done that
    if ( !m_parsingDynamicProperty )
    {
        // read in the integer tag label
        uint tag;
        if ( !ISOK( m_reader->Get( tag ) ) || tag != ( uint )kChunkInt )
        {
            ERR_REPORT( ES_Error, "Unable to find int Chunk tag." );
            return;
        }

        string readName;
        if ( !ISOK( m_reader->Get( readName ) || readName != name ) )
        {
            ERR_REPORT( ES_Error, "Unable to find int property name." );
            return;
        }
    }

    if ( !ISOK( m_reader->Get( value ) ) )
    {
        ERR_REPORT( ES_Error, "Unable to read int value." );
        return;
    }
}

void stoObjectReaderChunk::GetPropValue(const string &name, bool &value)
{
    errAssert( m_reader );

    // when parsing a dynamic property, we can skip reading in
    // the tag and name, since we've already done that
    if ( !m_parsingDynamicProperty )
    {
        // read in the Boolean tag label
        uint tag;
        if ( !ISOK( m_reader->Get( tag ) ) || tag != ( uint )kChunkBool )
        {
            ERR_REPORT( ES_Error, "Unable to find Boolean Chunk tag." );
            return;
        }

        string readName;
        if ( !ISOK( m_reader->Get( readName ) || readName != name ) )
        {
            ERR_REPORT( ES_Error, "Unable to find Boolean property name." );
            return;
        }
    }

    if ( !ISOK( m_reader->Get( value ) ) )
    {
        ERR_REPORT( ES_Error, "Unable to read Boolean value." );
        return;
    }
}

void stoObjectReaderChunk::GetPropValue(const string &name, string &value)
{
    errAssert( m_reader );

    // when parsing a dynamic property, we can skip reading in
    // the tag and name, since we've already done that
    if ( !m_parsingDynamicProperty )
    {
        // read in the string tag label
        uint tag;
        if ( !ISOK( m_reader->Get( tag ) ) || tag != ( uint )kChunkString )
        {
            ERR_REPORT( ES_Error, "Unable to find string Chunk tag." );
            return;
        }

        string readName;
        if ( !ISOK( m_reader->Get( readName ) || readName != name ) )
        {
            ERR_REPORT( ES_Error, "Unable to find string property name." );
            return;
        }
    }

    if ( !ISOK( m_reader->Get( value ) ) )
    {
        ERR_REPORT( ES_Error, "Unable to read string value." );
        return;
    }
}

void stoObjectReaderChunk::GetPropValue(const string &name, wstring &value)
{
    errAssert( m_reader );
    UNREF(name);
    UNREF(value);
    ERR_REPORTV( ES_Info, ( "Unable to read wide string values." ) );
}

void stoObjectReaderChunk::GetPropValue(const string &name, float &value)
{
    errAssert( m_reader );

    // when parsing a dynamic property, we can skip reading in
    // the tag and name, since we've already done that
    if ( !m_parsingDynamicProperty )
    {
        // read in the floating point number tag label
        uint tag;
        if ( !ISOK( m_reader->Get( tag ) ) || tag != ( uint )kChunkFloat )
        {
            ERR_REPORT( ES_Error, "Unable to find floating point Chunk tag." );
            return;
        }

        string readName;
        if ( !ISOK( m_reader->Get( readName ) || readName != name ) )
        {
            ERR_REPORT( ES_Error, "Unable to find floating point property name." );
            return;
        }
    }

    if ( !ISOK( m_reader->Get( value ) ) )
    {
        ERR_REPORT( ES_Error, "Unable to read floating point value." );
        return;
    }
}

void stoObjectReaderChunk::GetPropValue(const string &name, uint &value)
{
    errAssert( m_reader );

    // when parsing a dynamic property, we can skip reading in
    // the tag and name, since we've already done that
    if ( !m_parsingDynamicProperty )
    {
        // read in the unsigned integer tag label
        uint tag;
        if ( !ISOK( m_reader->Get( tag ) ) || tag != ( uint )kChunkUint )
        {
            ERR_REPORTV( ES_Error, ( "Unable to read chunk tag for %s", name.c_str() ) );

            return;
        }

        string readName;
        if ( !ISOK( m_reader->Get( readName ) || readName != name ) )
        {
            ERR_REPORTV( ES_Error, ( "Unable to read name from %s", name.c_str() ) );

            return;
        }
    }

    if ( !ISOK( m_reader->Get( value ) ) )
    {
        ERR_REPORTV( ES_Error, ( "Unable to read unsigned integer value of %s", name.c_str() ) );

        return;
    }
}

void stoObjectReaderChunk::GetPropValue(const string &name,
    proObject *&value, OUT DP_OWN_VALUE &owner)
{
    errAssert( m_reader );

    // if we're parsing a dynamic property, we know it's
    // got to be a regular object so,
    if ( m_parsingDynamicProperty )
    {
        // disable the flag now, so that the subcall doesn't
        // think it's dynamic as well (this is the only type
        // of property where this action is required)
        m_parsingDynamicProperty = false;

        // read in the object
        value = ReadObject( owner );

        // done
        return;
    }

    // check the subtag (that allows for NULL pointers)
    uint objectType = ( uint )kNULLTag;
    if ( !ISOK( m_reader->Get( objectType ) ) )
    {
        ERR_REPORTV( ES_Error, ( "Unable to read data tag for %s", name.c_str() ) );

        value = 0;
        return;
    }


    // read in the object tag label
    uint tag;
    if ( !ISOK( m_reader->Get( tag ) ) || tag != ( uint )kChunkObject )
    {
        ERR_REPORTV( ES_Error, ( "Unable to read object tag for %s", name.c_str() ) );
        value = 0;

        return;
    }

    // then read in the name, and make sure it matches
    string readName;
    if ( !ISOK( m_reader->Get( readName ) || readName != name ) )
    {
        ERR_REPORTV( ES_Error, ( "Unable to read name of %s", name.c_str() ) );

        value = 0;

        return;
    }

    // finally, read the object as needed
    switch ( objectType )
    {
    case ( uint )kNULLTag:
        value = 0;
        owner = DP_ownValue;

        return;
    case ( uint )kSubObjectTag:
    case ( uint )kSubObjectReferenceTag:
        value = ReadObject( owner );

        return;
    default:
        ERR_REPORTV(ES_Error, ("Uknown tag of %x value for %s", objectType, name.c_str()));
        value = 0;

        return;
    }
}

errResult stoObjectReaderChunk::EndObject()
{
    // nothing much to do here, really
    return ER_Success;
}



