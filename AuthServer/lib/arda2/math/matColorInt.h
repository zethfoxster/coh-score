/*****************************************************************************
	created:	2001/10/18
	copyright:	2001, NCSoft. All Rights Reserved
	author(s):	Peter M. Freese

	purpose:	A color structure used for containing integer color
				values with each component in the range 0-255.
*****************************************************************************/

#ifndef   INCLUDED_matColorInt
#define   INCLUDED_matColorInt
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// prevent warning about anonymous union/struct
#if CORE_COMPILER_MSVC
#pragma warning( push )
#pragma warning( disable: 4201 )
#endif

class matColor;
class matColorInt;
class matVector4;

// we have to forward declare these for VC++
bool operator==( const matColorInt&, const matColorInt& );
bool operator!=( const matColorInt&, const matColorInt& );

class matColorInt
{
public:
	matColorInt( int r, int g, int b, int a = 255 );
	explicit matColorInt( DWORD dwColor );
	matColorInt();
	
	explicit matColorInt( const matColor &color );
	
	// NOTE: implicit copy constructor and assignment operator ok
	
	void Set( int r, int g, int b, int a = 255 );
	void Set( const matColorInt &color );
	void Set( const matColor &color );
	void Set( const matVector4& colorvec );

	void SetR( int r ) { m_r = static_cast<uint8>(r); }
	void SetG( int g ) { m_g = static_cast<uint8>(g); }
	void SetB( int b ) { m_b = static_cast<uint8>(b); }
	void SetA( int a ) { m_a = static_cast<uint8>(a); }
	
	int GetR() const { return m_r; }
	int GetG() const { return m_g; }
	int GetB() const { return m_b; }
	int GetA() const { return m_a; }
	
	void SetDWORD( uint dword ) { m_dword = dword; }
	uint GetDWORD() const { return m_dword; }
	matVector4 GetVec() const;
	
	void Invert();

	void Interpolate(	float fInterpolant, 
						const matColorInt color0, 
						const matColorInt color1 );
	// catmullrom interpolation
	void Interpolate(	float fInterpolant, 
						const matColorInt color0, 
						const matColorInt color1, 
						const matColorInt color2, 
						const matColorInt color3 );

	// equality operators
	friend bool operator==( const matColorInt &lhs, const matColorInt &rhs )  
	{ return lhs.m_dword == rhs.m_dword; }
	
	friend bool operator!=( const matColorInt &lhs, const matColorInt &rhs )  
	{ return lhs.m_dword != rhs.m_dword; }

protected:
	void SetFloat( float r, float g, float b, float a );
	
private:  
	union
	{
#if CORE_SYSTEM_XENON
        struct
        {
            uint8 m_a;
            uint8 m_r;
            uint8 m_g;
            uint8 m_b;
        };
#else
        struct
		{
			uint8 m_b;
			uint8 m_g;
			uint8 m_r;
			uint8 m_a;
		};
#endif
        uint m_dword;
	};
	
public:
	static const matColorInt Black;
	static const matColorInt Blue;
	static const matColorInt Cyan;
	static const matColorInt Green;
	static const matColorInt Grey;
	static const matColorInt Magenta;
	static const matColorInt Red;
	static const matColorInt White;
	static const matColorInt Yellow;
};


//
// Constructor.
//
inline matColorInt::matColorInt()
{
	Set(255,255,255,255);
}


inline matColorInt::matColorInt( DWORD dwColor )
{
	SetDWORD(dwColor);
}


inline matColorInt::matColorInt( const matColor &color )
{
	Set(color);
}


//
// Construct the color components with integer [0,255] values
//
inline matColorInt::matColorInt( int r, int g, int b, int a )
{
	Set(r, g, b, a);
}


//
// Set the color components with integer [0,255] values
//
inline void matColorInt::Set( int r, int g, int b, int a )
{
	m_r = static_cast<uint8>(r);
	m_g = static_cast<uint8>(g);
	m_b = static_cast<uint8>(b);
	m_a = static_cast<uint8>(a);
}


//
// Sets the color.
//
// Parameters:
//      color: Some color.
//
inline void matColorInt::Set(const matColorInt &color)
{
	m_dword = color.m_dword;
}


inline void matColorInt::Invert()
{
	m_dword = ~m_dword;
}



// restore suppressed warning(s)
#if CORE_COMPILER_MSVC
#pragma warning( pop )
#endif



#endif // INCLUDED_matColorInt

