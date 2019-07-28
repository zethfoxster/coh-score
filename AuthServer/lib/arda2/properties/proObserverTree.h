/**
 * observers of a tree of property objects
 *
 *  @author Sean Riley
 *
 *  @date 1/13/2003
 *
 *  @file
**/

#ifndef __proObserverTree_h__
#define __proObserverTree_h__

#include "arda2/core/corStlHashmap.h"
#include "arda2/properties/proObserver.h"

class proObserverTreeItem;


/**
 * observe a hierachy of objects. only one hierarchy at a time!
 *
**/
class proObserverTree : public proObserver
{
public:
    proObserverTree();
    virtual ~proObserverTree();
    
    // receive a signal observable that an event has occurred
    virtual void Signal(proObject *obj, const int token, const int eventType, const std::string &propName);

    // set the root object of the tree to be observed
    virtual void SetTarget(proObject* target);

    // the number of unique objects being observed
    size_t GetNumItems() const { return m_unique.size(); }

protected:

    friend class proObserverTreeItem;

    // allows filtering when objects are being added
    virtual bool IgnoreObject(proObject *parent, proObject *object, const std::string &propName);

    // remove all observer items
    void FlushItems();

    // observe a new child object - called after FilterObject returns true
    proObserverTreeItem* AddItem(proObject* child);

    // un-observe an object
    bool RemoveItem(proObserverTreeItem *item);

    proObject *GetTarget() { return m_target; }
private:

    // begin observing an object
    virtual int Observe(proObject *obj);

    // stop observing an object - pass the token returned by Observe()
    virtual void Unobserve(proObject *obj, int token);

    typedef HashMap<proObject*, proObserverTreeItem*, corHashTraitsPointer<proObject> > UniqueMap;

    int                   m_nextToken;
    proObject*            m_target;
    UniqueMap             m_unique;
};


#endif // __proObserverTree_h__
