#include "arda2/core/corFirst.h"

#include "arda2/properties/proFirst.h"
#include "arda2/properties/proObject.h"
#include "arda2/properties/proObserverTree.h"
#include "arda2/properties/proStringUtils.h"

using namespace std;

/**
 * an item used by the tree observer to observe an object.
 * maintains a ref count when objects are observed multiple times.
 */
class proObserverTreeItem : public proObserver
{
public:
    proObserverTreeItem(proObserverTree *tree, int token);
    virtual ~proObserverTreeItem();
    
    // receive a signal observable that an event has occurred
    virtual void Signal(proObject *obj, const int token, const int eventType, const string &propName);

    // begin observing an object & its children
    virtual int Observe(proObject *obj);

    // stop observing an object & its children
    virtual void Unobserve(proObject *obj, int token);

    int GetToken() const { return m_token; }
    proObject* GetObject() const { return m_target; }

    int IncRef() { return ++m_refCount; }
    int DecRef() { return --m_refCount; }

protected:

    proObject*       m_target;
    int              m_token;
    proObserverTree* m_tree;
    int              m_refCount;
};




/***************** proObserverTree implementation *********/

proObserverTree::proObserverTree() :
   m_nextToken(0),
   m_target(NULL)
{
}


proObserverTree::~proObserverTree()
{
    FlushItems();
}

void proObserverTree::FlushItems()
{
    UniqueMap::iterator it = m_unique.begin();
    UniqueMap::iterator end = m_unique.end();

    for (; it != end; ++it)
    {
        proObserverTreeItem* item = it->second;
        delete item; // un-observes implicitly
    }
    m_unique.clear();
}

/**
 * set the target root object of the tree
 */
void proObserverTree::SetTarget(proObject* target)
{
    if (m_target)
    {
        FlushItems();
    }

    m_target = target;
    if (!m_target)
        return;

    // don't filter the root object?
    AddItem(target);
}

proObserverTreeItem* proObserverTree::AddItem(proObject* child)
{
    UniqueMap::iterator uit = m_unique.find(child);
    if (uit == m_unique.end())
    {
        // new reference to this object. create a new item
        int token = m_nextToken++;
        proObserverTreeItem* newItem = new proObserverTreeItem(this, token);

        newItem->Observe(child);
        m_unique[child] = newItem;

        //ERR_REPORTV(ES_Info, ("AddItem: %d, %s, %u, %u", token, child->GetClass()->GetName().c_str(), newItem, child));

        vector<proObject*> objs;
        child->GetChildren(objs);

        vector<proObject*>::iterator it = objs.begin();
        vector<proObject*>::iterator end = objs.end();
        for (; it != end; ++it)
        {
            proObject *item = *it;
            AddItem(item);
        }
        return newItem;
    }
    else
    {
        // duplicate reference to this object.
        proObserverTreeItem* item = uit->second;
        item->IncRef();

        //ERR_REPORTV(ES_Info, ("IncRef: %d, %s, %u, %u", item->GetToken(), child->GetClass()->GetName().c_str(), item, child));
        return item;
    }

    
}

/**
 * return true if item was actually removed, not just decrefed
 */
bool proObserverTree::RemoveItem(proObserverTreeItem *item)
{
    UniqueMap::iterator uit = m_unique.find(item->GetObject());
    if (uit == m_unique.end())
    {
        ERR_REPORTV(ES_Warning, ("Removing item not found."));
        return false;
    }

    int num = item->DecRef();
    if (num == 0)
    {
        //ERR_REPORTV(ES_Info, ("RemoveItem: %d, %u", item->GetToken(), item->GetObject()));
        if (item->GetObject() == m_target)
        {
            m_target = NULL;
        }

        // all refs to this item gone. remove it.
        m_unique.erase(uit);
        delete item;
        return true;
    }
    else
    {
        //ERR_REPORTV(ES_Info, ("DecRef: %d, %u (%d)", item->GetToken(), item->GetObject(), num));
        return false;
    }
}

// begin observing an object
int proObserverTree::Observe(proObject *obj)
{
    return proObserver::Observe(obj);
}

// stop observing an object - pass the token returned by Observe()
void proObserverTree::Unobserve(proObject *obj, int token)
{
    proObserver::Unobserve(obj, token);
}



/**
 * Allow filtering of objects added to the tree.
 * This is called before objects are observed to allow certain objects to be filtered
 * out of the set of objects being observed
 *
 * @return true to allow the object, false to no-observe it.
 */
bool proObserverTree::IgnoreObject(proObject *parent, proObject *object, const string &propName)
{
    UNREF(parent);
    UNREF(object);
    UNREF(propName);

    // by default, all objects are observed
    return false;
}


void proObserverTree::Signal(proObject *obj, const int token, const int eventType, const string &propName)
{
    UNREF(obj);
    UNREF(token);
    UNREF(eventType);
    UNREF(propName);
}


/***************** proObserverTreeItem implementation *********/

proObserverTreeItem::proObserverTreeItem(proObserverTree *tree, int token) :
  m_target(NULL),
  m_token(token),
  m_tree(tree),
  m_refCount(0)
{
}

proObserverTreeItem::~proObserverTreeItem()
{
    if (m_target)
    {
        proObserver::Unobserve(m_target, 0);
        m_target = NULL;
    }
}
    
// receive a signal observable that an event has occurred
void proObserverTreeItem::Signal(proObject *obj, const int token, const int eventType, const string &propName)
{
    switch (eventType)
    {
      case kPropertyChanged:
      {
          proProperty *property = obj->GetPropertyByName(propName);
          proObject *child = proConvUtils::AsObject(obj, property);
          if (child)
          {
              // new value for an object property - observe it.
              if (!m_tree->IgnoreObject(obj, child, propName))
                m_tree->AddItem(child);
          }
          m_tree->Signal(obj, token, eventType, propName);
      }
      break;

      case kPropertyAdded:
      {
          proProperty *property = obj->GetPropertyByName(propName);
          proObject *child = proConvUtils::AsObject(obj, property);
          if (child)
          {
              // new object property - observe it.
              if (!m_tree->IgnoreObject(obj, child, propName))
                m_tree->AddItem(child);
          }
          m_tree->Signal(obj, token, eventType, propName);
      }
      break;

      case kPropertyRemoved:
      {
          m_tree->Signal(obj, token, eventType, propName);
      }
      break;

      case kDeleted:
      {
          m_tree->Signal(obj, token, eventType, propName);
          m_tree->RemoveItem(this);
      }
      break;
    }
}

/**
 *  Observes the target object and immediate children. 
 *  This function is recursive for children of children.
 */
int proObserverTreeItem::Observe(proObject *obj)
{
    proObserver::Observe(obj);
    m_target = obj;
    m_refCount = 1;
    return m_token;
}

// stop observing an object & its children
void proObserverTreeItem::Unobserve(proObject *obj, int)
{
    proObserver::Unobserve(obj, 0);
}

#if BUILD_UNIT_TESTS

#include "arda2/test/tstUnit.h"

#include "proPropertyOwner.h"
#include "proClassNative.h"
#include "proPropertyNative.h"

typedef pair<proObject*,int> proObjectToken;

static const int defaultInt       = 7711;
static const uint defaultUint       = 11234;
static const float defaultFloat   = 0.55113f;
static const string defaultString = "default";
static const bool defaultBool     = true;

/**
 * test class for proObject tests
*/
class proObserverTargetTestTree : public proObject
{
PRO_DECLARE_OBJECT

public:
    proObserverTargetTestTree() : 
        m_x(defaultInt), 
        m_size(defaultUint), 
        m_ratio(defaultFloat),
        m_name(defaultString), 
        m_flag(defaultBool),
        m_number(0)
     {
     }

    virtual ~proObserverTargetTestTree() {}

    int GetX() const { return m_x;}
    uint GetSize() const { return m_size;}
    float GetRatio() const { return m_ratio;}
    string GetName() const { return m_name;}
    bool GetFlag() const { return m_flag;}
    uint GetNumber() const { return m_number;}

    void SetX(int value) { m_x = value; }
    void SetSize(int value) { m_size = value; }
    void SetRatio(float value) { m_ratio = value; }
    void SetName(const string &value) { m_name = value; }
    void SetFlag(bool value) { m_flag = value; }
    void SetNumber(uint value) { m_number = value; }

protected:
    int m_x;
    uint m_size;
    float m_ratio;
    string m_name;
    bool m_flag;
    uint m_number;

};

PRO_REGISTER_CLASS(proObserverTargetTestTree, proObject)
REG_PROPERTY_INT(proObserverTargetTestTree, X)
REG_PROPERTY_UINT(proObserverTargetTestTree, Size)
REG_PROPERTY_FLOAT(proObserverTargetTestTree, Ratio)
REG_PROPERTY_STRING(proObserverTargetTestTree, Name)
REG_PROPERTY_BOOL(proObserverTargetTestTree, Flag)
REG_PROPERTY_ENUM(proObserverTargetTestTree, Number, "zero,0,one,1,two,2,three,3,four,4,five,5")



class proObserverTreeTests : public tstUnit
{
private:
    proObserverTargetTestTree *m_object;
    proObserverTree *m_observer;

public:
    proObserverTreeTests()
    {
    }

    virtual void Register()
    {
        SetName("proObserverTree");
        AddDependency("proObserver");

        AddTestCase("testPropChanged", &proObserverTreeTests::testPropChanged);
        AddTestCase("testStaticChilden", &proObserverTreeTests::testStaticChilden);
        AddTestCase("testDynamicChilden", &proObserverTreeTests::testDynamicChilden);
        AddTestCase("testUnobserve", &proObserverTreeTests::testUnobserve);
        AddTestCase("testUnobserveWithChildren", &proObserverTreeTests::testUnobserveWithChildren);
        AddTestCase("testSwitch", &proObserverTreeTests::testSwitch);

    };

    virtual void TestCaseSetup()
    {
        m_object = new proObserverTargetTestTree();
        m_observer = new proObserverTree();


    }
    virtual void TestCaseTeardown()
    {
        SafeDelete(m_object);
        SafeDelete(m_observer);
    }

    void testPropChanged() const
    {
        // is removed on deletion
        m_observer->SetTarget(m_object);


        proProperty *pro = NULL;
        
        pro = m_object->GetPropertyByName("X");
        pro->FromString(m_object, "1000");

        pro = m_object->GetPropertyByName("Name");
        pro->FromString(m_object, "test");

        TESTASSERT(m_observer->GetNumItems() == 1);
    }
    

    void testStaticChilden() const
    {
        proObserverTargetTestTree *child1 = new proObserverTargetTestTree();
        proObserverTargetTestTree *child2 = new proObserverTargetTestTree();
        proObserverTargetTestTree *child3 = new proObserverTargetTestTree();

        m_object->AddChild(child1, "Child1");
        m_object->AddChild(child2, "Child2");
        child2->AddChild(child3, "foo");

        m_observer->SetTarget(m_object);
        TESTASSERT(m_observer->GetNumItems() == 4);
    }

    
    void testDynamicChilden() const
    {
        m_observer->SetTarget(m_object);

        proObserverTargetTestTree *child1 = new proObserverTargetTestTree();
        proObserverTargetTestTree *child2 = new proObserverTargetTestTree();
        proObserverTargetTestTree *child3 = new proObserverTargetTestTree();

        m_object->AddChild(child1, "Child1");
        m_object->AddChild(child2, "Child2");
        child2->AddChild(child3, "foo");

        TESTASSERT(m_observer->GetNumItems() == 4);

        child2->RemoveChild(child3);
        m_object->RemoveChild(child2);
        m_object->RemoveChild(child1);

        TESTASSERT(m_observer->GetNumItems() == 1);
    }

    void testUnobserve() const
    {
        m_observer->SetTarget(m_object);
        m_observer->SetTarget(NULL);

        TESTASSERT(m_observer->GetNumItems() == 0);
    }

    void testUnobserveWithChildren() const
    {
        m_observer->SetTarget(m_object);

        proObserverTargetTestTree *child1 = new proObserverTargetTestTree();
        proObserverTargetTestTree *child2 = new proObserverTargetTestTree();
        proObserverTargetTestTree *child3 = new proObserverTargetTestTree();

        m_object->AddChild(child1, "Child1");
        m_object->AddChild(child2, "Child2");
        child2->AddChild(child3, "foo");

        TESTASSERT(m_observer->GetNumItems() == 4);

        m_observer->SetTarget(NULL);

        TESTASSERT(m_observer->GetNumItems() == 0);
    }


    void testSwitch() const
    {
        // first set of objects
        proObserverTargetTestTree *parenta = new proObserverTargetTestTree();
        proObserverTargetTestTree *child1a = new proObserverTargetTestTree();
        proObserverTargetTestTree *child2a = new proObserverTargetTestTree();
        proObserverTargetTestTree *child3a = new proObserverTargetTestTree();
        proObserverTargetTestTree *child4a = new proObserverTargetTestTree();

        parenta->AddChild(child1a, "Child1");
        parenta->AddChild(child2a, "Child2");
        child2a->AddChild(child3a, "foo");
        child1a->AddChild(child4a, "foo");


        // second set of objects
        proObserverTargetTestTree *parentb = new proObserverTargetTestTree();
        proObserverTargetTestTree *child1b = new proObserverTargetTestTree();
        proObserverTargetTestTree *child2b = new proObserverTargetTestTree();
        proObserverTargetTestTree *child3b = new proObserverTargetTestTree();
        proObserverTargetTestTree *child4b = new proObserverTargetTestTree();

        parentb->AddChild(child1b, "Child1");
        child1b->AddChild(child2b, "Child2");
        child2b->AddChild(child3b, "foo");
        child3b->AddChild(child4b, "foo");

        child1b->AddChild(child2b, "Child_ref1", DP_refValue);
        child1b->AddChild(child3b, "Child_ref2", DP_refValue);
        child1b->AddChild(child4b, "Child_ref3", DP_refValue);

        m_observer->SetTarget(parenta);
        TESTASSERT(m_observer->GetNumItems() == 5);

        m_observer->SetTarget(parentb);
        TESTASSERT(m_observer->GetNumItems() == 5);

        m_observer->SetTarget(NULL);
        TESTASSERT(m_observer->GetNumItems() == 0);

        m_observer->SetTarget(parenta);
        TESTASSERT(m_observer->GetNumItems() == 5);

        m_observer->SetTarget(NULL);
        TESTASSERT(m_observer->GetNumItems() == 0);

        SafeDelete(parenta);
        SafeDelete(parentb);
    }

};

EXPORTUNITTESTOBJECT(proObserverTreeTests);


#endif

