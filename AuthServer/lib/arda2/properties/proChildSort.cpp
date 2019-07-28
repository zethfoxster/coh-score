#include "arda2/core/corFirst.h"
#include "arda2/properties/proFirst.h"

#include "proChildSort.h"

#include "arda2/properties/proProperty.h"

proChildSortByName::proChildSortByName()
{
}

proChildSortByName::~proChildSortByName()
{
}

bool proChildSortByName::ShouldSwap( proObject*,
    proProperty* inA, proProperty* inB )
{
    return ( inA->GetName() > inB->GetName() );
}

proChildISortByName::proChildISortByName()
{
}

proChildISortByName::~proChildISortByName()
{
}

bool proChildISortByName::ShouldSwap( proObject*,
    proProperty* inA, proProperty* inB )
{
    return ( _stricmp( inA->GetName().c_str(), inB->GetName().c_str() ) > 0 );
}

