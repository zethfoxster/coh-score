
#ifndef   INCLUDED_matAdapters_h
#define   INCLUDED_matAdapters_h
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "arda2/properties/proFirst.h"

#include "arda2/core/corStdString.h"
#include "arda2/math/matVector3.h"
#include "arda2/math/matColor.h"

#include "arda2/properties/proObject.h"
#include "arda2/properties/proAdapter.h"

#include "arda2/math/matTransform.h"


/**
 * property adapter for mat transform. 
**/
class matTransformAdapter : public proAdapter<matTransform>
{
    PRO_DECLARE_OBJECT
public:
    matTransformAdapter();
    virtual ~matTransformAdapter(){}

    // property mutators
    bool SetPosition(const matVector3 &val)  { m_target->SetPosition(val); return true;}
    bool SetScale(float val)           { m_target->SetScale(val);  return true;}
    void SetYaw(float val)             { UNREF(val); }
    void SetPitch(float val)           { UNREF(val); }
    void SetRoll(float val)            { UNREF(val); }

    // property accessors
    matVector3 GetPosition() const      { return m_target->GetPosition(); }
    float GetScale() const              { return m_target->GetScale(); }
    float GetYaw() const                { return 1.0f; }
    float GetPitch() const              { return 1.0f; }
    float GetRoll() const               { return 1.0f; }
};



/**
 * property adapter for mat transform. 
**/
class matVectorAdapter : public proAdapter<matVector3>
{
    PRO_DECLARE_OBJECT
public:
    matVectorAdapter();
    virtual ~matVectorAdapter(){}

    // property mutators
    void SetX(float val)             { m_target->x = val; }
    void SetY(float val)             { m_target->y = val; }
    void SetZ(float val)             { m_target->z = val; }

    // property accessors
    float GetX() const               { return m_target->x; }
    float GetY() const               { return m_target->y; }
    float GetZ() const               { return m_target->z; }
};


/**
 * property adapter for mat transform. 
**/
class matColorAdapter : public proAdapter<matColor>
{
    PRO_DECLARE_OBJECT
public:
    matColorAdapter();
    virtual ~matColorAdapter(){}

    // property mutators
    void SetR(float val)             { m_target->SetR(val); }
    void SetG(float val)             { m_target->SetG(val); }
    void SetB(float val)             { m_target->SetB(val); }
    void SetA(float val)             { m_target->SetA(val); }

    // property accessors
    float GetR() const               { return m_target->GetR(); }
    float GetG() const               { return m_target->GetG(); }
    float GetB() const               { return m_target->GetB(); }
    float GetA() const               { return m_target->GetA(); }
};

#endif // INCLUDED_matAdapters_h
