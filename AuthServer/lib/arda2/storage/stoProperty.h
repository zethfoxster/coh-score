

#ifndef   INCLUDED_stoProperties_h
#define   INCLUDED_stoProperties_h
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "arda2/core/corStdString.h"

#include "arda2/properties/proFirst.h"

#include "arda2/properties/proPropertyNative.h"
#include "arda2/properties/proPropertyOwner.h"

class stoConfigFile;

void proRegisterStorageTypes();

////// ------------------ base classes for storage properties ----------------------

/*
 * a filename property without path information
*/
class stoPropertyFilename : public proPropertyString
{
    PRO_DECLARE_OBJECT
public:
   typedef std::string MyType;

    stoPropertyFilename() : proPropertyString() {}
    virtual ~stoPropertyFilename() {}

    virtual proDataType GetDataType() const { return DT_string; }

    virtual void        FromString(proObject* o, const std::string& aValue );
    virtual std::string      ToString(const proObject* o) const { return GetValue(o); }

};

/*
 * a directory property
*/
class stoPropertyDirectory : public proPropertyString
{
    PRO_DECLARE_OBJECT
public:
   typedef std::string MyType;

    stoPropertyDirectory() : proPropertyString() {}
    virtual ~stoPropertyDirectory() {}

    virtual proDataType GetDataType() const { return DT_string; }

    virtual void        FromString(proObject* o, const std::string& aValue ) { SetValue(o, aValue); };
    virtual std::string      ToString(const proObject* o) const { return GetValue(o); }

};

/*
 * a full path including filename and directory
*/
class stoPropertyFullpath : public proPropertyString
{
    PRO_DECLARE_OBJECT
public:
   typedef std::string MyType;

    stoPropertyFullpath() : proPropertyString() {}
    virtual ~stoPropertyFullpath() {}

    virtual proDataType GetDataType() const { return DT_string; }

    virtual void        FromString(proObject* o, const std::string& aValue ) { SetValue(o, aValue); };
    virtual std::string      ToString(const proObject* o) const { return GetValue(o); }

};


////// ------------------ classes for native storage properties ----------------------

/*
*/
class stoPropertyFilenameNative : public stoPropertyFilename
{
    PRO_DECLARE_OBJECT
public:
    typedef std::string   MyType;
    virtual std::string       GetValue(const proObject *target) const = 0;
    virtual void         SetValue(proObject *target, const std::string &value) = 0;
    virtual ~stoPropertyFilenameNative() {};
};

/*
*/
class stoPropertyDirectoryNative : public stoPropertyDirectory
{
    PRO_DECLARE_OBJECT
public:
    typedef std::string   MyType;
    virtual std::string       GetValue(const proObject *target) const = 0;
    virtual void         SetValue(proObject *target, const std::string &value) = 0;
    virtual ~stoPropertyDirectoryNative() {};
};
/*
*/
class stoPropertyFullpathNative : public stoPropertyFullpath
{
    PRO_DECLARE_OBJECT
public:
    typedef std::string   MyType;
    virtual std::string       GetValue(const proObject *target) const = 0;
    virtual void         SetValue(proObject *target, const std::string &value) = 0;
    virtual ~stoPropertyFullpathNative() {};
};

// macros to register native storage properties

#define REG_PROPERTY_FILENAME( c, p ) \
    REG_PROPERTY_INTERNAL(std::string, const std::string&, sto, Filename, c, p, GET_CODE, SET_CODE, NULL)

#define REG_PROPERTY_DIRECTORY(c,p) \
    REG_PROPERTY_INTERNAL(std::string, const std::string&, sto, Directory, c, p, GET_CODE, SET_CODE, NULL)

#define REG_PROPERTY_FULLPATH(c,p) \
    REG_PROPERTY_INTERNAL(std::string, const std::string&, sto, Fullpath, c, p, GET_CODE, SET_CODE, NULL)

#define REG_PROPERTY_FILENAME_ANNO( c, p, annotations ) \
    REG_PROPERTY_INTERNAL(std::string, const std::string&, sto, Filename, c, p, GET_CODE, SET_CODE, annotations)

#define REG_PROPERTY_DIRECTORY_ANNO(c,p, annotations) \
    REG_PROPERTY_INTERNAL(std::string, const std::string&, sto, Directory, c, p, GET_CODE, SET_CODE, annotations)

#define REG_PROPERTY_FULLPATH_ANNO(c,p, annotations) \
    REG_PROPERTY_INTERNAL(std::string, const std::string&, sto, Fullpath, c, p, GET_CODE, SET_CODE, annotations)


////// ------------------ classes for owner storage properties ----------------------


/*
*/
class stoPropertyFilenameOwner : public proPropertyOwner<std::string, const std::string &, stoPropertyFilename>
{
    PRO_DECLARE_OBJECT
public:
    typedef std::string MyType;

    stoPropertyFilenameOwner() { }
    virtual ~stoPropertyFilenameOwner() {};
};

/*
*/
class stoPropertyFullpathOwner : public proPropertyOwner<std::string, const std::string &, stoPropertyFullpath>
{
    PRO_DECLARE_OBJECT
public:
    typedef std::string MyType;

    stoPropertyFullpathOwner() {}
    virtual ~stoPropertyFullpathOwner() {};
};

/*
*/
class stoPropertyDirectoryOwner : public proPropertyOwner<std::string, const std::string &, stoPropertyDirectory>
{
    PRO_DECLARE_OBJECT
public:
    typedef std::string MyType;

    stoPropertyDirectoryOwner() {}
    virtual ~stoPropertyDirectoryOwner() {};
};


// functions for loading annotation data from ini files
errResult stoLoadAnnotations(const std::string &filename);
errResult stoLoadAnnotations(stoConfigFile &config);
errResult stoMergeAnnotations(stoConfigFile &source, stoConfigFile &destination);
errResult stoSaveAnnotationsForClass(stoConfigFile &config, proClass *cls);
errResult stoSaveAllClasses(stoConfigFile &config,const std::string &className);

#endif // INCLUDED_matProperties_h
