#include "arda2/core/corFirst.h"
#include "arda2/properties/proClassNative.h"
#include "arda2/properties/proPropertyNative.h"
#include "arda2/properties/proPropertyOwner.h"
#include "arda2/properties/proUndoList.h"
#include "arda2/properties/proUndoCmds.h"

PRO_REGISTER_CLASS(proUndoList, proObject)

proUndoList::proUndoList() : 
    m_userData(NULL),
    m_currentCmd(0),
    m_count(1)
{
}

proUndoList::~proUndoList()
{
}

/**
 * add a new command at the current position - may invalidate other cmds.
**/
int proUndoList::AddCommand(proUndoCmdBase *action)
{
    // clear all commands after for current position
    while (GetPropertyCount() > m_currentCmd)
    {
        proProperty *pro = GetPropertyAt(GetPropertyCount()-1);
        this->RemoveInstanceProperty(pro);
    }
    char buf[256];
    sprintf(buf, "%d-%s", m_count++, action->GetName().c_str() );
    action->SetName(buf);
    action->Initialize(this);

    AddChild(action, buf);
    m_currentCmd = GetPropertyCount();
    return m_currentCmd;
}

void proUndoList::RemoveCommand(proUndoCmdBase *action)
{
    // find the position of the action
    int pos = 0;
    for (; pos != GetPropertyCount(); pos++)
    {
        proProperty *prop = GetPropertyAt(pos);
        proObject *found = proConvUtils::AsObject(this, prop);
        if (found && found == action)
        {
            if (m_currentCmd >= pos)
                m_currentCmd--;
            RemoveChild(action);
            return;
        }
    }
}

/**
 * clear all of the existing commands
**/
void proUndoList::ClearCommands()
{
    FlushChildren();
    m_currentCmd = 0;
}

/**
 * Undo the current command and move the position back
**/
int proUndoList::UndoCommand()
{
    if (m_currentCmd <= 0)
        return -1;

    proUndoCmdBase *command = GetCommandAt(m_currentCmd-1);
    if (!command)
        return -1;

    //ERR_REPORTV(ES_Info, ("Undo: %s", command->GetName().c_str()));
    if (ISOK(command->Apply(kUndo, m_userData)) )
    {
        m_currentCmd -= 1;
    }
    else 
    {
        return -1;
    }

    return m_currentCmd;
}

/**
 * Redo the next command and move the position forward
**/
int proUndoList::RedoCommand()
{
    if (m_currentCmd < 0)
        return -1;

    proUndoCmdBase *command = GetCommandAt(m_currentCmd);
    if (!command)
        return -1;

    //ERR_REPORTV(ES_Info, ("Redo: %s", command->GetName().c_str()));
    if (ISOK(command->Apply(kRedo, m_userData)) )
    {
        m_currentCmd += 1;
    }
    else
    {
        return -1;
    }
    return m_currentCmd;
}
/**
 * get the current command
**/
proUndoCmdBase *proUndoList::GetCurrentCommand()
{
    if (m_currentCmd < 1)
        return NULL;

    return GetCommandAt(m_currentCmd-1);
}

/**
 * get the next command
**/
proUndoCmdBase *proUndoList::GetNextCommand()
{
    return GetCommandAt(m_currentCmd);
}

proUndoCmdBase *proUndoList::GetCommandAt(int index)
{
    proProperty *pro = GetPropertyAt(index);
    if (!pro)
    {
        return NULL;
    }
    proObject *obj = proConvUtils::AsObject(this, pro);
    if (!proClassRegistry::InstanceOf(obj->GetClass(),"proUndoCmdBase"))
    {
        return NULL;
    }
    proUndoCmdBase *command = static_cast<proUndoCmdBase*>(obj);
    return command;
}

int proUndoList::GetNumCommands() const
{
    return GetPropertyCount();
}

/**
 * Replaces all references to an object in undo command items
 * with a clone of that object.
 *
 * Must also replace references to children of the replaced object
 */
void proUndoList::ReplaceObject(proObject *old, proObject *clone, proUndoCmdBase *ignore)
{
    // replace the object itself
    for (int i=0; i != GetPropertyCount(); i++)
    {
        proProperty *prop = GetPropertyAt(i);
        proObject *cmd = proConvUtils::AsObject(this, prop);
        if (cmd)
        {
            if (proClassRegistry::InstanceOf(cmd->GetClass(), "proUndoCmdBase"))
            {
                proUndoCmdBase *cmdProp = (proUndoCmdBase*)cmd;
                if (cmdProp != ignore)
                {
                    cmdProp->ReplaceObject(old, clone, ignore);
                }
            }
        }
    }
    // replace children recursively
    for (int j=0; j != old->GetPropertyCount(); j++)
    {
        proProperty *propOld = old->GetPropertyAt(j);
        proProperty *propClone = clone->GetPropertyAt(j);

        proObject *objOld   = proConvUtils::AsObject(old, propOld);
        proObject *objClone = proConvUtils::AsObject(clone, propClone);

        if (objOld && objClone)
            ReplaceObject(objOld, objClone, ignore);
    }
}


void proUndoList::StorePropChanged(proObject *target, const std::string &propName, const std::string &before, const std::string &after)
{
    proUndoCmdPropertyChanged *newCmd =
        proMakeUndoCmdPropertyChanged(
        target, propName, before, after );

    AddCommand(newCmd);
}

void proUndoList::StoreProObjectChanged(proObject *target, const std::string &propName, proObject *before, proObject *after)
{
    proUndoCmdPropertyObjectChanged *newCmd =
        proMakeUndoCmdPropertyObjectChanged(
        target, propName, before, after );

    AddCommand(newCmd);
}
















/*
 *  UNIT TESTS
 */
#if BUILD_UNIT_TESTS

#include "arda2/test/tstUnit.h"
#include "arda2/properties/proClassNative.h"
#include "arda2/properties/proPropertyNative.h"

static const int defaultInt       = 7711;
static const uint defaultUint       = 11234;
static const float defaultFloat   = 0.55113f;
static const std::string defaultString = "default";
static const bool defaultBool     = true;

static const std::string newName("foobar");
static const int newX = 51;
static const uint newSize = 99999;
static const float newRatio = 0.4f;

/**
 * test class for proObject tests
*/
class proTestUndoClass : public proObject
{
    PRO_DECLARE_OBJECT
public:
    proTestUndoClass() : 
        m_x(defaultInt), 
        m_size(defaultUint), 
        m_ratio(defaultFloat),
        m_name(defaultString), 
        m_flag(defaultBool),
        m_number(0),
        m_object(NULL)
     {}

    virtual ~proTestUndoClass() 
    {
        SafeDelete(m_object);
    }

    int GetX() const { return m_x;}
    uint GetSize() const { return m_size;}
    float GetRatio() const { return m_ratio;}
    std::string GetName() const { return m_name;}
    bool GetFlag() const { return m_flag;}
    uint GetNumber() const { return m_number;}
    proObject *GetObject() const { return (proObject*)m_object; }

    void SetX(int value) { m_x = value; }
    void SetSize(int value) { m_size = value; }
    void SetRatio(float value) { m_ratio = value; }
    void SetName(const std::string &value) { m_name = value; }
    void SetFlag(bool value) { m_flag = value; }
    void SetNumber(uint value) { m_number = value; }
    void SetObject(proObject *value) { SafeDelete(m_object); m_object = value; }

protected:
    int m_x;
    uint m_size;
    float m_ratio;
    std::string m_name;
    bool m_flag;
    uint m_number;
    proObject *m_object;

};

PRO_REGISTER_CLASS(proTestUndoClass, proObject)
REG_PROPERTY_INT(proTestUndoClass, X)
REG_PROPERTY_UINT(proTestUndoClass, Size)
REG_PROPERTY_FLOAT(proTestUndoClass, Ratio)
REG_PROPERTY_STRING(proTestUndoClass, Name)
REG_PROPERTY_BOOL(proTestUndoClass, Flag)
REG_PROPERTY_ENUM(proTestUndoClass, Number, "zero,0,one,1,two,2,three,3,four,4,five,5")
REG_PROPERTY_OBJECT(proTestUndoClass, Object, proObject)


class proUndoListTests : public tstUnit
{
private:
    proTestUndoClass *m_object;
    proUndoList      *m_undoList;

public:
    proUndoListTests()
    {
    }

    virtual void Register()
    {
        SetName("proUndoList");
        proClassRegistry::Init();
        AddTestCase("testBasicTypes", &proUndoListTests::testBasicTypes);
        AddTestCase("testSimpleObject1", &proUndoListTests::testSimpleObject1);
        AddTestCase("testSimpleObject2", &proUndoListTests::testSimpleObject2);
        AddTestCase("testReplaceObject", &proUndoListTests::testReplaceObject);
        AddTestCase("testComplexObject", &proUndoListTests::testComplexObject);
        AddTestCase("testReplaceParent", &proUndoListTests::testReplaceParent);
    };

    virtual void TestCaseSetup()
    {
        m_object = new proTestUndoClass();
        m_undoList = new proUndoList();
    }

    virtual void TestCaseTeardown()
    {
        SafeDelete(m_object);
        SafeDelete(m_undoList);
    }

    void testBasicTypes() const
    {
        int num;

        proTestUndoClass *object = new proTestUndoClass();
        proUndoList* undoList = new proUndoList();

        object->SetName("one");
        undoList->StorePropChanged(object, "Name", defaultString, "one");

        object->SetName("two");
        undoList->StorePropChanged(object, "Name", "one", "two");

        object->SetName("three");
        undoList->StorePropChanged(object, "Name", "two", "three");
        
        object->SetX(100);
        undoList->StorePropChanged(object, "X", "7711", "100");

        // got right number of commands
        TESTASSERT(undoList->GetNumCommands() == 4);

        // test undoing
        num = undoList->UndoCommand();
        TESTASSERT(object->GetX() == 7711 && num == 3);

        num = undoList->UndoCommand();
        TESTASSERT(object->GetName() == "two" && num == 2);

        num = undoList->UndoCommand();
        TESTASSERT(object->GetName() == "one" && num == 1);

        num = undoList->UndoCommand();
        TESTASSERT(object->GetName() == "default" && num == 0);

        num = undoList->UndoCommand();
        TESTASSERT(object->GetName() == "default" && num == -1);

        // mess with it (equal numbers of both)
        undoList->RedoCommand();
        undoList->RedoCommand();
        undoList->UndoCommand();
        undoList->RedoCommand();
        undoList->UndoCommand();
        undoList->UndoCommand();

        // test re-doing
        num = undoList->RedoCommand();
        TESTASSERT(object->GetName() == "one" && num == 1);

        num = undoList->RedoCommand();
        TESTASSERT(object->GetName() == "two" && num == 2);

        num = undoList->RedoCommand();
        TESTASSERT(object->GetName() == "three" && num == 3);

        num = undoList->RedoCommand();
        TESTASSERT(object->GetX() == 100 && num == 4);

        num = undoList->RedoCommand();
        TESTASSERT(object->GetX() == 100 && num == -1);

        SafeDelete(object);
        SafeDelete(undoList);
    }

    /**
     * change from null to a value and undo & redo it
     */
    void testSimpleObject1() const
    {
        int num;

        proTestUndoClass *object = new proTestUndoClass();
        proTestUndoClass *child = new proTestUndoClass();
        proUndoList* undoList = new proUndoList();

        child->SetName("CHILD");

        // 1
        undoList->StoreProObjectChanged(object, "Object", NULL, child);
        object->SetObject(child);

        // 2
        num = undoList->UndoCommand();
        TESTASSERT(object->GetObject() == NULL && num == 0);
        //SafeDelete(child);

        // 3
        num = undoList->RedoCommand();
        TESTASSERT(object->GetObject() != NULL && num == 1);

        // object should be a clone of child
        TESTASSERT(object->GetObject() != child);
        TESTASSERT( ((proTestUndoClass*)object->GetObject())->GetName() == "CHILD");

        SafeDelete(object);
        SafeDelete(undoList);
    }

    /**
     * change from a value to null and undo & redo it
     */
    void testSimpleObject2() const
    {
        int num;

        proTestUndoClass *object = new proTestUndoClass();
        proTestUndoClass *child = new proTestUndoClass();
        proUndoList* undoList = new proUndoList();

        child->SetName("CHILD");
        object->SetObject(child);

        // 1
        undoList->StoreProObjectChanged(object, "Object", child, NULL);
        object->SetObject(NULL);
        //SafeDelete(child);

        // 2
        num = undoList->UndoCommand();
        // object should be a clone of child
        TESTASSERT(object->GetObject() != child);
        TESTASSERT( ((proTestUndoClass*)object->GetObject())->GetName() == "CHILD");
        child = (proTestUndoClass*)object->GetObject();

        // 3
        num = undoList->RedoCommand();
        TESTASSERT(object->GetObject() == NULL && num == 1);
        //SafeDelete(child);

        SafeDelete(object);
        SafeDelete(undoList);
    }


    /**
     * change from a value to different value and undo & redo it
     */
    void testReplaceObject() const
    {
        int num;

        proTestUndoClass *object = new proTestUndoClass();
        proTestUndoClass *child1 = new proTestUndoClass();
        proTestUndoClass *child2 = new proTestUndoClass();
        proUndoList* undoList = new proUndoList();

        child1->SetName("CHILD1");
        child2->SetName("CHILD2");
        object->SetObject(child1);

        // 1
        undoList->StoreProObjectChanged(object, "Object", child1, child2);
        object->SetObject(child2);
        //SafeDelete(child1);

        // 2
        num = undoList->UndoCommand();
        // object should be a clone of child1
        TESTASSERT(object->GetObject() != NULL && num == 0);
        TESTASSERT( ((proTestUndoClass*)object->GetObject())->GetName() == "CHILD1");

        // 3
        num = undoList->RedoCommand();
        TESTASSERT(object->GetObject() != NULL && num == 1);
        TESTASSERT( ((proTestUndoClass*)object->GetObject())->GetName() == "CHILD2");

        // mess with it
        num = undoList->UndoCommand();
        num = undoList->RedoCommand();
        num = undoList->UndoCommand();
        num = undoList->RedoCommand();
        TESTASSERT( ((proTestUndoClass*)object->GetObject())->GetName() == "CHILD2");
        num = undoList->UndoCommand();
        TESTASSERT( ((proTestUndoClass*)object->GetObject())->GetName() == "CHILD1");

        SafeDelete(object);
        SafeDelete(undoList);
    }

    void testComplexObject() const
    {
        proTestUndoClass *object = new proTestUndoClass();
        proTestUndoClass *child1 = new proTestUndoClass();
        proTestUndoClass *child2 = new proTestUndoClass();
        proTestUndoClass *child3 = new proTestUndoClass();
        proUndoList* undoList = new proUndoList();

        /**
         * 1 - attach a child1 object
         * 2 - modify values on child1 object
         * 3 - replace child1 with new child2
         * 4 - modify values on child2
         * 5 - NULL out child2
         * 6 - replace NULL  with new child3
         * 7 - modify values of child3
         */

        // 1
        undoList->StoreProObjectChanged(object, "Object", NULL, child1);
        object->SetObject(child1);

        // 2 
        undoList->StorePropChanged(child1, "Name", "default", "child1");
        child1->SetName("child1");
        undoList->StorePropChanged(child1, "Number", "0", "1000");
        child1->SetNumber(1000);

        // 3
        undoList->StoreProObjectChanged(object, "Object", child1, child2);
        object->SetObject(child2);

        // 4
        undoList->StorePropChanged(child2, "Name", "default", "child2");
        child2->SetName("child2");
        undoList->StorePropChanged(child2, "Number", "0", "69");
        child2->SetNumber(69);

        // 5
        undoList->StoreProObjectChanged(object, "Object", child2, NULL);
        object->SetObject(NULL);

        // 6
        undoList->StoreProObjectChanged(object, "Object", NULL, child3);
        object->SetObject(child3);

        // 7
        undoList->StorePropChanged(child3, "Name", "default", "child3");
        child3->SetName("child3");
        undoList->StorePropChanged(child3, "Number", "0", "2000");
        child3->SetNumber(2000);
        TESTASSERT(undoList->GetNumCommands() == 10);
        TESTASSERT( ((proTestUndoClass*)object->GetObject())->GetName() == "child3");
        TESTASSERT( ((proTestUndoClass*)object->GetObject())->GetNumber() == 2000);

        // undo 7
        undoList->UndoCommand();
        undoList->UndoCommand();
        TESTASSERT( ((proTestUndoClass*)object->GetObject())->GetName() == "default");
        TESTASSERT( ((proTestUndoClass*)object->GetObject())->GetNumber() == 0);

        // undo 6
        undoList->UndoCommand();
        TESTASSERT( object->GetObject() == NULL);

        // undo 5
        undoList->UndoCommand();
        TESTASSERT( ((proTestUndoClass*)object->GetObject())->GetName() == "child2");
        TESTASSERT( ((proTestUndoClass*)object->GetObject())->GetNumber() == 69);

        // undo 4
        undoList->UndoCommand();
        undoList->UndoCommand();
        TESTASSERT( ((proTestUndoClass*)object->GetObject())->GetName() == "default");
        TESTASSERT( ((proTestUndoClass*)object->GetObject())->GetNumber() == 0);

        // undo 3
        undoList->UndoCommand();
        TESTASSERT( ((proTestUndoClass*)object->GetObject())->GetName() == "child1");
        TESTASSERT( ((proTestUndoClass*)object->GetObject())->GetNumber() == 1000);

        // undo 2
        undoList->UndoCommand();
        undoList->UndoCommand();
        TESTASSERT( ((proTestUndoClass*)object->GetObject())->GetName() == "default");
        TESTASSERT( ((proTestUndoClass*)object->GetObject())->GetNumber() == 0);

        // undo 1
        undoList->UndoCommand();
        TESTASSERT( object->GetObject() == NULL);

        //SafeDelete(child1);
        //SafeDelete(child2);
        //SafeDelete(child3);

        // redo 1
        undoList->RedoCommand();
        TESTASSERT( object->GetObject() != NULL);
        TESTASSERT( ((proTestUndoClass*)object->GetObject())->GetName() == "default");
        TESTASSERT( ((proTestUndoClass*)object->GetObject())->GetNumber() == 0);

        // redo 2
        undoList->RedoCommand();
        undoList->RedoCommand();
        TESTASSERT( ((proTestUndoClass*)object->GetObject())->GetName() == "child1");
        TESTASSERT( ((proTestUndoClass*)object->GetObject())->GetNumber() == 1000);

        // redo 3
        undoList->RedoCommand();
        TESTASSERT( object->GetObject() != NULL);
        TESTASSERT( ((proTestUndoClass*)object->GetObject())->GetName() == "default");
        TESTASSERT( ((proTestUndoClass*)object->GetObject())->GetNumber() == 0);

        // redo 4
        undoList->RedoCommand();
        undoList->RedoCommand();
        TESTASSERT( ((proTestUndoClass*)object->GetObject())->GetName() == "child2");
        TESTASSERT( ((proTestUndoClass*)object->GetObject())->GetNumber() == 69);

        // redo 5
        undoList->RedoCommand();
        TESTASSERT( object->GetObject() == NULL);

        // redo 6
        undoList->RedoCommand();
        TESTASSERT( object->GetObject() != NULL);
        TESTASSERT( ((proTestUndoClass*)object->GetObject())->GetName() == "default");
        TESTASSERT( ((proTestUndoClass*)object->GetObject())->GetNumber() == 0);

        // redo 7
        undoList->RedoCommand();
        undoList->RedoCommand();
        TESTASSERT( ((proTestUndoClass*)object->GetObject())->GetName() == "child3");
        TESTASSERT( ((proTestUndoClass*)object->GetObject())->GetNumber() == 2000);
        
        // undo & redo all twice!
        while (undoList->UndoCommand());
        while (undoList->RedoCommand() != -1);
        while (undoList->UndoCommand());
        while (undoList->RedoCommand() != -1);

        TESTASSERT(undoList->GetNumCommands() == 10);
        TESTASSERT( ((proTestUndoClass*)object->GetObject())->GetName() == "child3");
        
        SafeDelete(object);
        SafeDelete(undoList);

    }


    /**
     * Change from a value of a child of X, then replace X.
     * When an object (X) is replaced, pointers to its children must also be replaced.
     */
    void testReplaceParent() const
    {
        proTestUndoClass *object = new proTestUndoClass();
        proTestUndoClass *parent = new proTestUndoClass();
        proTestUndoClass *child = new proTestUndoClass();
        proTestUndoClass *newParent = new proTestUndoClass();
        proUndoList* undoList = new proUndoList();

        parent->SetName("child");
        child->SetName("child");
        newParent->SetName("newParent");
        object->SetObject(parent);
        parent->SetObject(child);

        // 1 - change value on child of parent
        undoList->StorePropChanged(child, "Number", "0", "5555");
        child->SetNumber(5555);

        // 2 - replace the parent
        undoList->StoreProObjectChanged(object, "Object", parent, newParent);
        object->SetObject(newParent);
        //SafeDelete(parent);

        // child is already deleted
        undoList->UndoCommand();

        undoList->UndoCommand();
        undoList->RedoCommand();
        undoList->RedoCommand();

        SafeDelete(object);
        SafeDelete(undoList);
    }

};

EXPORTUNITTESTOBJECT(proUndoListTests);


#endif
