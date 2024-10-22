/////
//
// JVector.h
// declaration of class JVector, a 2D vector class
#ifndef __JVECTOR_H__
#define __JVECTOR_H__
// #include "JMDefs.h"
#include "assert.h"

#include "DungeonConstants.h"

#define VEC_EXPAND( v ) ( v ).x, ( v ).y

template <class T> class TVector2
{
    // Member variables
public:
    T x;
    T y;
    // Member functions
public:
    TVector2<T>() {}
    TVector2<T>( const T inx, const T iny ) : x( inx ), y( iny ) {}
    TVector2<T>( const TVector2<T> &in ) : x( in.x ), y( in.y ) {}
    virtual ~TVector2<T>() {}

    // Array indexing
    const T &operator[]( unsigned int i )
    {
        assert( i < 2 );
        return *( &x + i );
    }

    // Array indexing
    const T &operator[]( unsigned int i ) const
    {
        assert( i < 2 );
        return *( &x + i );
    }

    // Add another JVector to this one
    TVector2<T> &operator+=( const TVector2<T> &in )
    {
        x += in.x;
        y += in.y;

        return *this;
    }

    // subtract another JVector from this one
    TVector2<T> &operator-=( const TVector2<T> &in )
    {
        x -= in.x;
        y -= in.y;

        return *this;
    }

    // multiply vector be int scalar
    TVector2<T> &operator*=( T &i )
    {
        x *= i;
        y *= i;

        return *this;
    }

    // multiply vector be int scalar
    TVector2<T> &operator*=( T i )
    {
        x *= i;
        y *= i;

        return *this;
    }

    // integer divide vector by scalar
    TVector2<T> &operator/=( T &i )
    {
        assert( i != 0 );
        x /= i;
        y /= i;

        return *this;
    }

    // negate this vector
    friend TVector2<T> operator-( const TVector2 &a )
    {
        TVector2<T> vReturn( -a.x, -a.y );
        return ( vReturn );
    }

    // subtract 2 vectors
    friend TVector2<T> operator-( const TVector2<T> &a, const TVector2<T> &b )
    {
        TVector2<T> vReturn( a );
        vReturn -= b;
        return ( vReturn );
    }

    // subtract 2 vectors
    friend TVector2<T> operator+( const TVector2<T> &a, const TVector2<T> &b )
    {
        TVector2<T> vReturn( a );
        vReturn += b;
        return ( vReturn );
    }

    // multiply vector by scalar
    friend TVector2<T> operator*( const TVector2<T> &a, const T &i )
    {
        TVector2<T> vReturn( a );
        vReturn *= i;
        return ( vReturn );
    }

    // multiply vector by scalar
    friend TVector2<T> operator*( const T &i, const TVector2<T> &a )
    {
        TVector2<T> vReturn( a );
        vReturn *= i;
        return ( vReturn );
    }

    // multiply vector by scalar
    friend TVector2<T> operator/( const TVector2<T> &a, const int &i )
    {
        assert( i != 0 );
        TVector2<T> vReturn( a );
        vReturn /= i;
        return ( vReturn );
    }

    // test equality
    friend bool operator==( const TVector2<T> &a, const TVector2<T> &b )
    {
        return ( ( a.x == b.x ) && ( a.y == b.y ) );
    }

    // test inequality
    friend bool operator!=( const TVector2<T> &a, const TVector2<T> &b )
    {
        return ( ( a.x != b.x ) || ( a.y != b.y ) );
    }

    bool IsInWorld() { return ( x >= 0 && x < ( DUNG_WIDTH ) && y >= 0 && y < ( DUNG_HEIGHT ) ); }

    void Init( T inx = 0, T iny = 0 )
    {
        x = inx;
        y = iny;
    }

    bool IsZero() { return ( ( x == 0 ) && ( y == 0 ) ); }

    float Length() { return ( ( x * x ) + ( y * y ) ); }

    void Norm()
    {
        float len = Length();
        if( len != 0.0f )
        {
            x /= len;
            y /= len;
        }
    }
    void printvec( const char *label ) { printf( "%s: %f %f ", label, x, y ); }
};

typedef TVector2<int> JIVector;
typedef TVector2<float> JVector;
typedef TVector2<float> JFVector;
#endif // __JVECTOR_H__
