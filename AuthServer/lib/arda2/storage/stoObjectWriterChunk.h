/**
 *  property system
 *
 *  @author Sean Riley
 *
 *  @date 1/13/2003
 *
 *  @file
**/

#ifndef __CHUNK_OBJECT_WRITER_H__
#define __CHUNK_OBJECT_WRITER_H__

#include "stoFirst.h"

#include "arda2/core/corStdString.h"
#include "arda2/core/corStlMap.h"
#include "arda2/properties/proObjectWriter.h"

class proObject;
class proProperty;
class stoChunkFileWriter;
class stoChunkFrameWriter;


/**
 * This ObjectWriter is for serializing property objects to a chunk file.
 *
 * Includes support for writing references to objects that
 * are included in the stream multiple times.
**/
class stoObjectWriterChunk : public proObjectWriter
{
    PRO_DECLARE_OBJECT
public:
    stoObjectWriterChunk() : proObjectWriter(), m_writer(NULL) {}
    stoObjectWriterChunk( stoChunkFileWriter* inWriter );
    virtual ~stoObjectWriterChunk();

    // invoked externally to write a object out
    virtual void WriteObject(const proObject *obj);

    // invoked from within proObject or derived classes
    virtual bool CheckReference(const proObject *obj); 

    // invoked when NOT a reference
    virtual void BeginObject(const std::string &className, const int version);
    virtual void PutPropValue(const std::string &name, const int value);
    virtual void PutPropValue(const std::string &name, const bool value);
    virtual void PutPropValue(const std::string &name, const std::string &value);
    virtual void PutPropValue(const std::string &name, const std::wstring &value);
    virtual void PutPropValue(const std::string &name, const float value);
    virtual void PutPropValue(const std::string &name, const uint value);
    virtual void PutPropValue(const std::string &name, const proObject *obj);

    // used to write out additional types as strings
    virtual void PutPropTyped(const std::string &name,
        const std::string &value, const std::string &typeName);

    virtual void EndObject();

protected:

private:
    stoChunkFileWriter*  m_writer;

    // used to save off object references properly
    typedef std::map< const proObject*, uint > RefMap;
    RefMap m_objectReferences; // uint is the token to write into the file
};



#endif

