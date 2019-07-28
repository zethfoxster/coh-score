/*****************************************************************************
    created:    2001/10/18
    copyright:  2001, NCSoft. All Rights Reserved
    author(s):  Peter M. Freese

    purpose:    
*****************************************************************************/

#ifndef   INCLUDED_matColor
#define   INCLUDED_matColor
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "arda2/math/matVector4.h"

class matColorInt;

class matColor
{
public:
    matColor();
    // NOTE: implicit copy constructor and assignment operator okay
    matColor( float fR, float fG, float fB, float fA = 1.0f );
    explicit matColor( const matColorInt &color );
    
    void Set( float fR, float fG, float fB, float fA = 1.0f );
    void Set( const matColor &color );
    void Set( const matColorInt& color );
    void Set( const matVector4& colorvec );
    
    void SetR( float fR ) { m_r = fR; }
    void SetG( float fG ) { m_g = fG; }
    void SetB( float fB ) { m_b = fB; }
    void SetA( float fA ) { m_a = fA; }
    
    void SetR( int r );
    void SetG( int g );
    void SetB( int b );
    void SetA( int a );
    
    float GetR() const { return m_r; }
    float GetG() const { return m_g; }
    float GetB() const { return m_b; }
    float GetA() const { return m_a; }
    
    float GetColorMagnitudeSquared() const;
    float GetColorMagnitude() const;
    float GetColorSum() const;
    float GetColorWeight() const;

    float GetColorMax() const;  //* max(r,g,b)

    float GetLargestComponent() const;
    float GetSmallestComponent() const;

    matColor& ScaleRGB( float fScale );
    matColor& AddRGB( const matColor &other );
    
    void Interpolate(   float fInterpolant, 
                        const matColor& color0, 
                        const matColor& color1, 
                        const matColor& color2, 
                        const matColor& color3 );

    void Interpolate(   float fInterpolant, 
                        const matColor& color0, 
                        const matColor& color1 );

    matColor& operator +=( const matColor &rhs )
    {
        AddRGB(rhs);
        return *this;
    }

    matColor& operator *=(float fScale )
    {
        ScaleRGB(fScale);
        return *this;
    }

    matColor& operator -()
    {
        ScaleRGB(-1.0f);
        return *this;
    }

    matColor operator +( const matColor &rhs ) const
    {
        // use constructor form to take advantage of Return Value Optimization
        return matColor(m_r + rhs.m_r, m_g + rhs.m_g, m_b + rhs.m_g, m_a + rhs.m_a);
    }

    matColor operator -( const matColor &rhs ) const
    {
        // use constructor form to take advantage of Return Value Optimization
        return matColor(m_r - rhs.m_r, m_g - rhs.m_g, m_b - rhs.m_g, m_a - rhs.m_a);
    }

    matColor operator *( float fScale ) const
    {
        // use constructor form to take advantage of Return Value Optimization
        return matColor(m_r * fScale, m_g * fScale, m_b * fScale, m_a);
    }

private:
    float m_r;
    float m_g;
    float m_b;
    float m_a;
    
public:
    static const matColor Black;
    static const matColor White;
    static const matColor Green;
    
private:
    // prevent accidental misuse
    explicit matColor(int red, int green, int blue, int alpha );
    void Set(int red, int green, int blue, int alpha );
};


inline matColor operator *(float fScale, const matColor &rhs)
{
    return rhs * fScale;
}


inline bool operator ==(const matColor &lhs, const matColor &rhs )
{
  return (
    (lhs.GetR() == rhs.GetR()) &&
    (lhs.GetG() == rhs.GetG()) &&
    (lhs.GetB() == rhs.GetB()) &&
    (lhs.GetA() == rhs.GetA()));
}

inline bool operator !=(const matColor &lhs, const matColor &rhs)
{
  return !operator==(lhs, rhs);
}

//
// Constructor.
//
inline matColor::matColor()
{
    Set(0.0f, 0.0f, 0.0f, 1.0f);
}


//
// Construct the color components with float [0.0,1.0] values
//
inline matColor::matColor(float red, float green, float blue, float alpha)
{
    Set(red, green, blue, alpha);
}


inline matColor::matColor( const matColorInt &color )
{
    Set(color);
}


//
// Set the color components with float [0.0,1.0] values
//
inline void matColor::Set( float fR, float fG, float fB, float fA )
{
    m_r = fR;
    m_g = fG;
    m_b = fB;
    m_a = fA;
}

//
// Sets the color.
//
// Parameters:
//      color: Some color.
//
inline void matColor::Set(const matColor &color)
{
    m_r = color.m_r;
    m_g = color.m_g;
    m_b = color.m_b;
    m_a = color.m_a;
}

//
// Scale RGB components of color by given factor
//
inline matColor& matColor::ScaleRGB( float fScale )
{
    m_r *= fScale;
    m_g *= fScale;
    m_b *= fScale;
    return *this;
}

inline matColor& matColor::AddRGB( const matColor &other )
{
    m_r += other.m_r;
    m_g += other.m_g;
    m_b += other.m_b;
    return *this;
}

inline float matColor::GetColorMagnitudeSquared() const
{
    return m_r * m_r + m_g * m_g + m_b * m_b;
}

inline float matColor::GetColorMagnitude() const
{
    return sqrtf(GetColorMagnitudeSquared());
}

inline float matColor::GetColorSum() const
{
    return m_r + m_g + m_b;
}

/**
 * This algorithm is also very simple but the basis of it is much more complex. 
 * The formula looks like: .30 * R + .59 * G + .11 * B. The percentages here 
 * relate to how perceptive the eye is to a given color. 
 **/
inline float matColor::GetColorWeight() const
{
    return (.30f * fabsf(m_r)) + (.59f * fabsf(m_g)) + (.11f * fabsf(m_b));
}


inline float matColor::GetColorMax() const
{
    return Max(Max(m_r, m_g), m_b);
}


inline float matColor::GetLargestComponent() const
{
    float currMax = m_r;

    if (fabsf(m_g) > fabsf(currMax))
    {
        currMax = m_g;
    }
    if (fabsf(m_b) > fabsf(currMax))
    {
        currMax = m_b;
    }

    return currMax;
}


inline float matColor::GetSmallestComponent() const
{
    float currMin = m_r;

    if (fabsf(m_g) < fabsf(currMin))
    {
        currMin = m_g;
    }
    if (fabsf(m_b) < fabsf(currMin))
    {
        currMin = m_b;
    }

    return currMin;
}

#endif // INCLUDED_matColor
