#include "arda2/core/corFirst.h"
#include "arda2/properties/proClassNative.h"
#include "arda2/properties/proPropertyNative.h"
#include "arda2/properties/proPropertyOwner.h"
#include "arda2/properties/proUndoCmds.h"

using namespace std;

/************** base Undo Command ****************/

PRO_REGISTER_ABSTRACT_CLASS(proUndoCmdBase, proObject)
REG_PROPERTY_STRING(proUndoCmdBase,  Name)
REG_PROPERTY_STRING(proUndoCmdBase,  PropertyName)

proUndoCmdBase::~proUndoCmdBase()
{
    if (m_object)
        Unobserve(m_object, m_objectToken);
}

void proUndoCmdBase::SetObject(proObject *val)
{ 
    m_object = val; 
    m_objectToken = Observe(m_object);
}

/**
 * called by the undo list when the command is added to it.
 *
 */
void proUndoCmdBase::Initialize(proUndoList *undoList)
{
    m_undoList = undoList;
}


/**
 * Called by the undolist to replace objects that may be deleted.
 * Implementors of this must check ALL of their proObject*'s, not
 * just the m_object member variable.
 */
void proUndoCmdBase::ReplaceObject(
    proObject *old, proObject *clone, proUndoCmdBase* ignore )
{
    UNREF( ignore );

    if (m_object == old)
    {
        Unobserve(m_object, m_objectToken);
        m_object = clone;
        m_objectToken = Observe(m_object);
    }
}


void proUndoCmdBase::Signal(proObject *obj, const int token, const int eventType, const string &)
{
    switch (eventType)
    {
    case kDeleted:
        {
            Unobserve(obj, token);
            if (obj == m_object)
            {
                m_object = NULL;

                if ( m_undoList )
                {
                    m_undoList->RemoveCommand(this);
                }
            }
        }
        return;

    default:
        return;
    }
}

/************** Property Changed Command ****************/

PRO_REGISTER_CLASS(proUndoCmdPropertyChanged, proUndoCmdBase)
REG_PROPERTY_STRING(proUndoCmdPropertyChanged,  Before)
REG_PROPERTY_STRING(proUndoCmdPropertyChanged,  After)

proUndoCmdPropertyChanged::proUndoCmdPropertyChanged() :
    proUndoCmdBase()
{
    m_name = "Changed";
}

proUndoCmdPropertyChanged::~proUndoCmdPropertyChanged()
{
}

void proUndoCmdPropertyChanged::SetBefore(const string &val)
{
    m_before = val;
}
void proUndoCmdPropertyChanged::SetAfter(const string &val)
{
    m_after = val;
}


string proUndoCmdPropertyChanged::GetBefore() const
{
    return m_before;
}
string proUndoCmdPropertyChanged::GetAfter() const
{
    return m_after;
}

errResult proUndoCmdPropertyChanged::Apply(eUndoDirection action, void *)
{
    if (!m_object)
        return ER_Failure;

    if (action == kUndo)
    {
        m_object->SetValue(m_propertyName.c_str(), m_before.c_str());
    }
    else
    {
        m_object->SetValue(m_propertyName.c_str(), m_after.c_str());
    }
    return ER_Success;
}

/************** Property Changed Command ****************/

PRO_REGISTER_CLASS(proUndoCmdPropertyObjectChanged, proUndoCmdBase)

proUndoCmdPropertyObjectChanged::proUndoCmdPropertyObjectChanged() :
    proUndoCmdBase(),
    m_before(NULL),
    m_beforeClone(NULL),
    m_after(NULL),
    m_afterClone(NULL),
    m_beforeCloneToken(-1),
    m_afterCloneToken(-1)
{
    m_name = "ObjectChanged";
}

proUndoCmdPropertyObjectChanged::~proUndoCmdPropertyObjectChanged()
{
    if (m_beforeClone)
    {
        Unobserve(m_beforeClone, m_beforeCloneToken);
        SafeDelete(m_beforeClone);
    }

    if (m_afterClone)
    {
        Unobserve(m_afterClone, m_afterCloneToken);
        SafeDelete(m_afterClone);
    }
}

void proUndoCmdPropertyObjectChanged::SetBefore(proObject *val)
{
    m_before = val;
}
void proUndoCmdPropertyObjectChanged::SetAfter(proObject *val)
{
    m_after = val;
}

proObject *proUndoCmdPropertyObjectChanged::GetBefore() const
{
    return m_before;
}
proObject *proUndoCmdPropertyObjectChanged::GetAfter() const
{
    return m_after;
}

void proUndoCmdPropertyObjectChanged::SetBeforeClone(proObject *clone)
{
    if (m_beforeClone)
    {
        Unobserve(m_beforeClone, m_beforeCloneToken);
    }
    m_beforeClone = clone;
    m_beforeCloneToken = Observe(m_beforeClone);
}

void proUndoCmdPropertyObjectChanged::SetAfterClone(proObject *clone)
{
    if (m_afterClone)
    {
        Unobserve(m_afterClone, m_afterCloneToken);
    }
    m_afterClone = clone;
    m_afterCloneToken = Observe(m_afterClone);
}


void proUndoCmdPropertyObjectChanged::Initialize(proUndoList *undoList)
{
    proUndoCmdBase::Initialize(undoList);

    // Clone the before object and replace references to old pointer.
    // The old "before" object may be deleted when this action occurs.
    if (m_before && !m_beforeClone)
    {
        SetBeforeClone(m_before->Clone());
        if ( m_undoList )
        {
            m_undoList->ReplaceObject(m_before, m_beforeClone, this);
        }
        m_before = NULL;
    }
}

errResult proUndoCmdPropertyObjectChanged::Apply(eUndoDirection action, void *)
{
    if (!m_object)
        return ER_Failure;

    proPropertyObjectOwner *prop = (proPropertyObjectOwner*)m_object->GetPropertyByName(m_propertyName);

    if (action == kUndo)
    {
        // Clone the "after" object the first time we are undone.
        // The "after" object may be delete by the undo.
        if (m_after && !m_afterClone)
        {
            SetAfterClone(m_after->Clone());
            m_undoList->ReplaceObject(m_after, m_afterClone, this);
            m_after = NULL;
        }
        else if (m_afterClone)
        {
            // when we undo, the after object may be deleted, so re-clone it here.
            proObject *newAfter = m_afterClone->Clone();
            m_undoList->ReplaceObject(m_afterClone, newAfter, this);
            SetAfterClone(newAfter);
        }
        prop->SetValue(m_object, m_beforeClone);
    }
    else
    {
        // when we redo, the before object may be deleted, so re-clone it here.
        if (m_beforeClone)
        {
            proObject *newBefore = m_beforeClone->Clone();
            m_undoList->ReplaceObject(m_beforeClone, newBefore, this);
            SetBeforeClone(newBefore);
        }
        prop->SetValue(m_object, m_afterClone);
    }
    return ER_Success;
}


void proUndoCmdPropertyObjectChanged::ReplaceObject(
    proObject *old, proObject *clone, proUndoCmdBase* ignore )
{
    proUndoCmdBase::ReplaceObject(old, clone, ignore );

    // also replace my local object pointers if needed
    if (m_before == old)
    {
        m_before = clone;
    }
    else if (m_after == old)
    {
        m_after = clone;
    }
    if (m_beforeClone == old)
    {
        SetBeforeClone(clone);
    }
    else if (m_afterClone == old)
    {
        SetAfterClone(clone);
    }
}

void proUndoCmdPropertyObjectChanged::Signal(proObject *obj, const int token, const int eventType, const string &)
{
    switch (eventType)
    {
    case kDeleted:
        {
            Unobserve(obj, token);

            if (obj == m_beforeClone)
            {
                m_beforeClone = NULL;
            }
            if (obj == m_afterClone)
            {
                m_afterClone = NULL;
            }
            if (obj == m_object)
            {
                m_object = NULL;
                m_undoList->RemoveChild(this);
            }
        }
        return;

    default:
        return;
    }
}

/************** Group Operation ****************/
PRO_REGISTER_CLASS( proUndoCmdGroupOperation, proUndoCmdBase )

proUndoCmdGroupOperation::proUndoCmdGroupOperation()
{
    m_name = "Group Operation";
}

proUndoCmdGroupOperation::~proUndoCmdGroupOperation()
{
}

// apply this command
errResult proUndoCmdGroupOperation::Apply(
    eUndoDirection inAction, void* inUserData )
{
    // walk the list of children, calling apply for
    // each subchild that is a proUncoCmdBase
    for ( int i = 0, stop = GetPropertyCount(); i < stop; ++i )
    {
        proUndoCmdBase* target = PRO_AS_TYPED_OBJECT( proUndoCmdBase,
            this, GetPropertyAt( i ) );
        if ( target )
        {
            if ( !ISOK( target->Apply( inAction, inUserData ) ) )
            {
                return ER_Failure;
            }
        }
    }

    return ER_Success;
}

// called by the undo list once the object is fully populated
void proUndoCmdGroupOperation::Initialize( proUndoList* inUndoList )
{
    proUndoCmdBase::Initialize( inUndoList );

    // initialize each of the children
    for ( int i = 0, stop = GetPropertyCount(); i < stop; ++i )
    {
        proUndoCmdBase* target = PRO_AS_TYPED_OBJECT( proUndoCmdBase,
            this, GetPropertyAt( i ) );
        if ( target )
        {
            target->Initialize( inUndoList );
        }
    }
}

// replace an object with it's clone
void proUndoCmdGroupOperation::ReplaceObject(
    proObject* inOld, proObject* inClone, proUndoCmdBase* ignore )
{
    for ( int i = 0, stop = GetPropertyCount(); i < stop; ++i )
    {
        proUndoCmdBase* target = PRO_AS_TYPED_OBJECT( proUndoCmdBase,
            this, GetPropertyAt( i ) );
        if ( target && target != ignore )
        {
            target->ReplaceObject( inOld, inClone, ignore );
        }
    }

    proUndoCmdBase::ReplaceObject( inOld, inClone, ignore );
}

/************** Helper Functions ****************/

// Helper method to build a property changed undo command
proUndoCmdPropertyChanged* proMakeUndoCmdPropertyChanged(
    proObject *target, const string &propName, const string &before, const string &after )
{
    proUndoCmdPropertyChanged *newCmd = new proUndoCmdPropertyChanged();
    newCmd->SetObject(target);
    newCmd->SetPropertyName(propName);
    newCmd->SetBefore(before);
    newCmd->SetAfter(after);

    return newCmd;
}

// Helper method to build a property changed changed undo command
proUndoCmdPropertyObjectChanged* proMakeUndoCmdPropertyObjectChanged(
    proObject *target, const string &propName, proObject *before, proObject *after )
{
    proUndoCmdPropertyObjectChanged *newCmd = new proUndoCmdPropertyObjectChanged();
    newCmd->SetObject(target);
    newCmd->SetPropertyName(propName);
    newCmd->SetBefore(before);
    newCmd->SetAfter(after);

    return newCmd;
}
