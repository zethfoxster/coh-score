/**
 *  undo functionality
 *
 *  @author Sean Riley
 *
 *  @date 1/13/2003
 *
 *  @file
**/

#ifndef __proUndoCmds_H__
#define __proUndoCmds_H__

#include "arda2/properties/proObject.h"
#include "arda2/properties/proUndoList.h"
#include "arda2/properties/proObserver.h"

/**
 * base class for user-defined undo events
**/
class proUndoCmdBase : public proObject, public proObserver
{
    PRO_DECLARE_OBJECT

public:

    proUndoCmdBase() : m_object(NULL), m_undoList(NULL) {}
    virtual ~proUndoCmdBase();

    // called by the undo list once the object is fully populated
    virtual void Initialize(proUndoList *undoList);

    // property accessors
    void SetName(const std::string &val)         { m_name = val; }
    void SetPropertyName(const std::string &val) { m_propertyName = val; m_name += "-"; m_name += val; }
    void SetObject(proObject *val);

    // property mutators
    std::string GetName() const                  { return m_name; }
    std::string GetPropertyName() const          { return m_propertyName; }
    proObject *GetObject() const            { return m_object; }

    // apply this command
    virtual errResult Apply(eUndoDirection action, void *userData) = 0;

    // replace an object with it's clone
    virtual void ReplaceObject(proObject *old, proObject *clone, proUndoCmdBase* ignore );

    virtual void Signal(proObject *obj, const int token, const int eventType, const std::string &propName);

protected:

    std::string            m_name;
    std::string            m_propertyName;
    proObject        *m_object;
    int               m_objectToken;
    proUndoList      *m_undoList;
};


/**
 * undo command for a property changing its value 
**/
class proUndoCmdPropertyChanged : public proUndoCmdBase
{
PRO_DECLARE_OBJECT
public:
    proUndoCmdPropertyChanged();
    virtual ~proUndoCmdPropertyChanged();

    void SetBefore(const std::string &val);
    void SetAfter(const std::string &val);

    std::string GetBefore() const;
    std::string GetAfter() const;

    virtual errResult Apply(eUndoDirection action, void *data);

protected:
    std::string       m_before;
    std::string       m_after;
};


/**
 * undo command for an object property changing its value 
**/
class proUndoCmdPropertyObjectChanged : public proUndoCmdBase
{
PRO_DECLARE_OBJECT
public:
    proUndoCmdPropertyObjectChanged();
    virtual ~proUndoCmdPropertyObjectChanged();

    virtual void Initialize(proUndoList *undoList);

    void SetBefore(proObject *val);
    void SetAfter(proObject *val);

    proObject *GetBefore() const;
    proObject *GetAfter() const;

    virtual errResult Apply(eUndoDirection action, void *data);
    virtual void ReplaceObject(proObject *old, proObject *clone, proUndoCmdBase* ignore );

    virtual void Signal(proObject *obj, const int token, const int eventType, const std::string &propName);

protected:

    void SetBeforeClone(proObject *clone);
    void SetAfterClone(proObject *clone);

    proObject *m_before;        // the real before object
    proObject *m_beforeClone;   // the cloned before object
    proObject *m_after;         // the real after object
    proObject *m_afterClone;    // the cloned after object

    int m_beforeCloneToken;
    int m_afterCloneToken;
};


/**
  * A class to facilitate atomic undo operations for
  * multi-property commands (e.g., undoing the keyframe
  * manipulations by the user in the keyframe editor window)
  *
 **/
class proUndoCmdGroupOperation : public proUndoCmdBase
{
    PRO_DECLARE_OBJECT

public:
    proUndoCmdGroupOperation();
    ~proUndoCmdGroupOperation();

    // apply this command
    virtual errResult Apply( eUndoDirection inAction, void* inUserData );

    // called by the undo list once the object is fully populated
    virtual void Initialize(proUndoList *undoList);

    // replace an object with it's clone
    virtual void ReplaceObject( proObject* old, proObject* clone, proUndoCmdBase* ignore );

protected:

private:
};

// Helper method to build a property changed undo command
proUndoCmdPropertyChanged* proMakeUndoCmdPropertyChanged(
    proObject *target, const std::string &propName, const std::string &before, const std::string &after );
// Helper method to build a property changed changed undo command
proUndoCmdPropertyObjectChanged* proMakeUndoCmdPropertyObjectChanged(
    proObject *target, const std::string &propName, proObject *before, proObject *after );

#endif //__proUndoCmds_H__
