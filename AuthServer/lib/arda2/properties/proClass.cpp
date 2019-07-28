#include "arda2/core/corFirst.h"

#include "proFirst.h"

#include "arda2/properties/proClass.h"
#include "arda2/properties/proObject.h"
#include "arda2/properties/proClassNative.h"
#include "arda2/properties/proPropertyNative.h"

PRO_REGISTER_ABSTRACT_CLASS(proClass, proObject)
//REG_PROPERTY_STRING(proClass, Name)

proClass::proClass() :
    proObject(),
    m_parent(NULL)
{
}

proClass::proClass(const std::string &name) :
    proObject(),
    m_name(name),
    m_parent(NULL)
{
}

proClass::~proClass()
{
}

bool proClass::Instanceof( const proClass* aClass ) const
{
    const proClass* c = this;

    do
    {
        if( c == aClass )
            return true;

        c = c->m_parent;
    }
    while( c != NULL );

    return false;
}

