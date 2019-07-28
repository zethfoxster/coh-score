/**
 * interface for observers of property objects
 *
 *  @author Sean Riley
 *
 *  @date 1/13/2003
 *
 *  @file
**/

#ifndef __proObserver_h__
#define __proObserver_h__

#include "arda2/core/corStdString.h"

class proObject;

/**
 * Types of signals that can be sent to observers from property objects.
 * It is possible that applications may define new types of observer
 * signals.
**/
enum ePropertyObserverEvents
{
    kPropertyChanged,
    kPropertyAdded,
    kPropertyRemoved,
    kPropertyNameChanged,
    kDeleted,
    kPropertiesReordered,
    kPropertyValueInvalid,
};

#define proTokenUndefined -1

/**
 * Interface for observers to implement to receive signal calls when
 * the state of observed objects changes.
 *
**/
class proObserver
{
public:
    proObserver() : m_nextToken(0)
    {
    };
    virtual ~proObserver() 
    {
    };
    
    // receive a signal observable that an event has occurred
    virtual void Signal(proObject *obj, const int token, const int eventType, const std::string &propName) = 0;

    // begin observing an object
    virtual int Observe(proObject *obj);

    // stop observing an object - pass the token returned by Observe()
    virtual void Unobserve(proObject *obj, int token);

protected:
    int m_nextToken;
};

// a proObject reference object that relies on
// the observer system to detect when the referenced
// object is deleted
class proObjectReference : public proObserver
{
public:
    proObjectReference();
    proObjectReference( proObject* inObject );
    proObjectReference( const proObjectReference& inSource );

    proObjectReference& operator=( const proObjectReference& inSource );
    proObject* operator=( proObject* inObject );
    proObject* operator*( void );

    void Signal( proObject* inObject, const int inToken,
        const int inEventType, const std::string& inPropertyName );

protected:
    void ReleaseObject( void );

    void SetObject( proObject* inObject );

private:
    proObject* m_object;
    int   m_token;
};

#endif // __proObserver_h__
