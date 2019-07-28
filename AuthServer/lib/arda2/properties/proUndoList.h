/**
 *  undo functionality
 *
 *  @author Sean Riley
 *
 *  @date 1/13/2003
 *
 *  @file
**/

#ifndef __proUndoList_H__
#define __proUndoList_H__

#include "arda2/properties/proObject.h"

class proUndoCmdBase;

enum eUndoDirection
{
    kUndo,
    kRedo
};

/**
 *  stack of undo commands.
 *
 * The list has a "current" position that moves as undo/redo are
 * called.
**/
class proUndoList : public proObject
{
public:
    PRO_DECLARE_OBJECT

    proUndoList();
    virtual ~proUndoList();

    void SetUserData(void *data) { m_userData = data; }

    // clear all of the existing commands
    void ClearCommands();

    // Undo the current command and move the position back
    int UndoCommand();

    // Redo the next command and move the position forward
    int RedoCommand();
    
    proUndoCmdBase *GetCurrentCommand();
    proUndoCmdBase *GetNextCommand();
    proUndoCmdBase *GetCommandAt(int index);
    int             GetNumCommands() const;

    // helper methods for common operations
    void StorePropChanged(proObject *target, const std::string &propName, const std::string &before, const std::string &after);
    void StoreProObjectChanged(proObject *target, const std::string &propName, proObject *before, proObject *after);

    // replace the old object with the clone in all commands (except "ignore")
    void ReplaceObject(proObject *old, proObject *clone, proUndoCmdBase *ignore);

    // add a new command at the current position - will cause commands after that position
    // to be deleted. this moves the current position the new command
    int AddCommand(proUndoCmdBase *action);

    void RemoveCommand(proUndoCmdBase *action);

protected:
    void *m_userData;   // is passed to each command on undo/redo
    int m_currentCmd;   // position of current command
    int m_count;        // used for unique name generation
};

#endif  // __proUndoList_H__
