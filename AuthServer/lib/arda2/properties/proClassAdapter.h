/**
 *  property class
 *
 *  @author Sean Riley
 *
 *  @date 1/13/2003
 *
 *  @file
**/

#ifndef _proClassAdapter_h__
#define _proClassAdapter_h__

#include "arda2/properties/proFirst.h"

#include "arda2/core/corStdString.h"
#include "arda2/core/corStlVector.h"

class proProperty;

#include "arda2/properties/proClassRegistry.h"
#include "arda2/properties/proObject.h"
#include "arda2/properties/proAdapter.h"

/** 
 * adapter for classes. used to allow classes to be displayed in the
 * tree editor. Regular proClass objects cannot be used in the tree editor.
 *
 */
class proClassAdapter : public proAdapter<proClass>
{
    PRO_DECLARE_OBJECT

public:
    proClassAdapter() {}
    virtual ~proClassAdapter() {}

    void SetClassname(const std::string &value);
    std::string GetClassname() const;

    void SetParentClassname(const std::string &value);
    std::string GetParentClassname() const;

protected:
    void Initialize();

};


#endif // _proClassAdapter_h__
