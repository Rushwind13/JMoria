//
// JRect.h
//
// Definition for JRect, a class to define a screen rectangle
//

#ifndef __JRECT_H__
#define __JRECT_H__

#include "JVector.h"

#define RECT_EXPAND( r ) ( r ).left, ( r ).top, ( r ).right, ( r ).bottom
#define VIEWRECT_EXPAND( r ) ( r ).left, ( r ).right, ( r ).top, ( r ).bottom

class JRect
{
public:
    int left;
    int top;
    int right;
    int bottom;

    JRect() : left( 0 ), top( 0 ), right( 0 ), bottom( 0 ) {}
    JRect( int l, int t, int r, int b ) : left( l ), top( t ), right( r ), bottom( b ) {}
    JRect( const JRect &in )
        : left( in.left ),
          top( in.top ),
          right( in.right ),
          bottom( in.bottom )
    {
    }
    JRect( const JIVector vPos, int width, int height )
        : left( vPos.x ),
          top( vPos.y ),
          right( vPos.x + width ),
          bottom( vPos.y + height )
    {
    }

    void Init( int l, int t, int r, int b )
    {
        left = l;
        top = t;
        right = r;
        bottom = b;
    }
    void Init( JIVector vPos, int width, int height )
    {
        left = vPos.x;
        top = vPos.y;
        right = vPos.x + width;
        bottom = vPos.y + height;
    }
    void Init( JRect &rcIn )
    {
        left = rcIn.left;
        top = rcIn.top;
        right = rcIn.right;
        bottom = rcIn.bottom;
    }

    int Left() { return left; }
    int Top() { return top; }
    int Right() { return right; }
    int Bottom() { return bottom; }
    int Width() { return Right() - Left(); }
    int Height() { return Bottom() - Top(); }

    void SetWidth( int dwWidth, bool setFromLeft = true )
    {
        if( setFromLeft )
            right = left + dwWidth;
        else
            left = right - dwWidth;
    };
    void SetHeight( int dwHeight, bool setFromTop = true )
    {
        if( setFromTop )
            bottom = top + dwHeight;
        else
            top = bottom - dwHeight;
    };

    const JRect Edges()
    {
        JRect edges( left - 1, top - 1, right + 1, bottom + 1 );
        return edges;
    };

    bool IsValidRect()
    {
        if( left > right || top > bottom )
        {
            JLog( LOG_LEVEL_ERROR, true, "JRect has bad memory allocation: <%d %d, %d %d>\n", left,
                  top, right, bottom );
            return false;
        }
        return true;
    }

    bool IsInWorld() { return IsInRect( JRect( 0, 0, DUNG_WIDTH, DUNG_HEIGHT ) ); }

    bool IsWithinWorld() { return IsWithinRect( JRect( 0, 0, DUNG_WIDTH, DUNG_HEIGHT ) ); }

    bool IsInRect( const JRect &rcIn )
    {
        if( left < rcIn.left || right >= rcIn.right || top < rcIn.top || bottom >= rcIn.bottom )
        {
            return false;
        }
        return IsValidRect();
    }

    bool IsWithinRect( const JRect &rcIn )
    {
        if( left < rcIn.left + 1 || right >= rcIn.right - 1 || top < rcIn.top + 1 ||
            bottom >= rcIn.bottom - 1 )
        {
            return false;
        }
        return IsValidRect();
    }

    bool Contains( const JIVector vTarget )
    {
        if( vTarget.x < left || vTarget.x >= right || vTarget.y < top || vTarget.y >= bottom )
        {
            return false;
        }
        return IsValidRect();
    }
};

#endif // __JRECT_H__
