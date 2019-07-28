/**
 *  property system
 *
 *  @author Sean Riley
 *
 *  @date 1/13/2003
 *
 *  @file
**/

#ifndef __CHUNK_OBJECT_READER_H__
#define __CHUNK_OBJECT_READER_H__

#include "stoFirst.h"
#include "arda2/core/corStdString.h"
#include "arda2/core/corStlMap.h"
#include "arda2/properties/proObjectReader.h"

class proObject;
class proProperty;
class stoChunkFileReader;

/**
 * ObjectReader is for unserializing property objects.
**/
class stoObjectReaderChunk : public proObjectReader
{
    PRO_DECLARE_OBJECT
public:
    stoObjectReaderChunk() : proObjectReader(), m_reader(NULL) {}
    stoObjectReaderChunk( stoChunkFileReader* );
    virtual ~stoObjectReaderChunk();

    virtual proObject *ReadObject(OUT DP_OWN_VALUE &owner);

    virtual proProperty *ReadDynamicProperty(proObject *object);

    // invoked from within proObject or derived classes
    virtual errResult BeginObject(const std::string &className, int &version);
    virtual void GetPropValue(const std::string &name, int &value);
    virtual void GetPropValue(const std::string &name, bool &value);
    virtual void GetPropValue(const std::string &name, std::string &value);
    virtual void GetPropValue(const std::string &name, std::wstring &value);
    virtual void GetPropValue(const std::string &name, float &value);
    virtual void GetPropValue(const std::string &name, uint &value);
    virtual void GetPropValue(const std::string &name,
        proObject *&value, OUT DP_OWN_VALUE &owner);
    virtual errResult EndObject();

protected:
    // helper methods to read in dynamic property types
    proProperty* ReadDynamicObjectProperty( proObject* );

private:
    stoChunkFileReader* m_reader;

    typedef std::map< uint, proObject* > RefMap;
    RefMap m_objectReferences;

    // used to validate the class type of read objects
    std::string m_currentClassName;

    // a flag used to communicate when a dynamic property
    // is being parsed (some parsing steps needed to be
    // omitted in this case)
    bool m_parsingDynamicProperty;
};

#endif

