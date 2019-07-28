#ifndef __PRO_CHILD_SORT_H__
#define __PRO_CHILD_SORT_H__

// sorting utilities for children of proObject classes

class proObject;
class proProperty;

class proChildSortFunctor
{
public:
    proChildSortFunctor() { };
    proChildSortFunctor( const proChildSortFunctor& ) { };
    virtual ~proChildSortFunctor() { };
    proChildSortFunctor& operator=( const proChildSortFunctor& ) { return *this; };

    virtual bool ShouldSwap( proObject* inObject, proProperty* inA, proProperty* inB ) = 0;

protected:

private:

};

// an example of a child sort functor
class proChildSortByName : public proChildSortFunctor
{
public:
    proChildSortByName();
    virtual ~proChildSortByName();

    virtual bool ShouldSwap( proObject* inObject, proProperty* inA, proProperty* inB );
};

// an example of a child sort functor
class proChildISortByName : public proChildSortFunctor
{
public:
    proChildISortByName();
    virtual ~proChildISortByName();

    virtual bool ShouldSwap( proObject* inObject, proProperty* inA, proProperty* inB );
};

#endif // __PRO_CHILD_SORT_H__
