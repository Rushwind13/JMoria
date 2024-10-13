/////
//
// JVector.h
// declaration of class JVector, a 2D vector class
#ifndef __JVECTOR_H__
#define __JVECTOR_H__
//#include "JMDefs.h"
#include "assert.h"

#include "DungeonConstants.h"

#define VEC_EXPAND(v) (v).x, (v).y

template<class T>class TVector2
{
// Member variables
public:
	T x;
	T y;
	// Member functions
public:
	TVector2() {};
	TVector2( const T inx, const T iny ):x(inx), y(iny) {};
	TVector2( const TVector2 &in ):x(in.x), y(in.y) {};
	virtual ~TVector2() {};	
	
	// Array indexing
	const T                 &operator [] (unsigned int i)
	{
		assert (i<2);
		return *(&x+i);
	}

	// Array indexing
	const T           &operator [] (unsigned int i) const
	{
		assert (i<2);
		return *(&x+i);
	}
	
	// Add another JVector to this one
	TVector2 &operator += ( const TVector2 &in )
	{
		x += in.x;
		y += in.y;

		return *this;
	}

	// subtract another JVector from this one
	TVector2 &operator -= ( const TVector2 &in )
	{
		x -= in.x;		
		y -= in.y;

		return *this;
	}

	// multiply vector be int scalar
	TVector2 &operator *= ( T &i )
	{
		x *= i;		
		y *= i;

		return *this;
	}	

	// multiply vector be int scalar
	TVector2 &operator *= ( T i )
	{
		x *= i;		
		y *= i;

		return *this;
	}	

	// integer divide vector by scalar
	TVector2 &operator /= ( T &i )
	{
		assert( i != 0 );
		x /= i;		
		y /= i;

		return *this;
	}

	// negate this vector
	friend TVector2 operator - ( const TVector2 &in )
	{
		return( JVector(-in.x, -in.y) );
	}

	// subtract 2 vectors
	friend TVector2 operator - ( const TVector2 &a, const TVector2 &b )
	{
		TVector2 vReturn(a);
		vReturn -= b;
		return(vReturn);
	}

	// subtract 2 vectors
	friend TVector2 operator + ( const TVector2 &a, const TVector2 &b )
	{
		TVector2 vReturn(a);
		vReturn += b;
		return(vReturn);
	}

	// multiply vector by scalar
	friend TVector2 operator * ( const TVector2 &a, T &i )
	{
		return(JVector(a.x * i, a.y * i));
	}

	// multiply vector by scalar
	friend TVector2 operator * ( T &i, const TVector2 &a )
	{
		return(JVector(a.x * i, a.y * i));
	}

	// multiply vector by scalar
	friend TVector2 operator / ( const TVector2 &a, int &i )
	{
		assert( i != 0 );
		return(TVector2(a.x / i, a.y / i));
	}

	// test equality
	friend bool operator == ( const TVector2 &a, const TVector2 &b )
	{
		return((a.x == b.x) && (a.y == b.y));
	}

	// test inequality
	friend bool operator != ( const TVector2 &a, const TVector2 &b )
	{
		return((a.x != b.x) || (a.y != b.y));
	}

	bool IsInWorld()
	{
		return( x > 0 && x < ( DUNG_WIDTH - 1 )
			 && y > 0 && y < ( DUNG_HEIGHT - 1 ));
	}

	void Init( T inx=0, T iny=0 )
	{
		x = inx;
		y = iny;
	}

	bool IsZero()
	{
		return((x == 0) && (y == 0));
	}

	float Length()
	{
		return( (x*x) + (y*y) );
	}

	void Norm()
	{
		float len = Length();
		if( len != 0.0f )
		{
			x /= len;
			y /= len;
		}
	}
	void printvec( const std::string label )
	{
		std::cout << label << ": " << x << " " << y << "  ";
	}
};

typedef TVector2<int> JIVector;
typedef TVector2<float> JVector;
typedef TVector2<float> JFVector;
#endif //__JVECTOR_H__
