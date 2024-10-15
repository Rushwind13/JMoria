//
// JRect.h
// 
// Definition for JRect, a class to define a screen rectangle
//

#ifndef __JRECT_H__
#define __JRECT_H__

#include "JVector.h"

#define RECT_EXPAND(r) (r).left, (r).top, (r).right, (r).bottom
#define VIEWRECT_EXPAND(r) (r).left, (r).right, (r).top, (r).bottom

class JRect
{
public:
	int left;
	int top;
	int right;
	int bottom;

	JRect():left(0), top(0), right(0), bottom(0) {};
	JRect(int l, int t, int r, int b):left(l), top(t), right(r), bottom(b) {};
	JRect(const JRect &in):left(in.left), top(in.top), right(in.right), bottom(in.bottom) {};
	JRect(const JIVector vPos, int width, int height):left(vPos.x), top(vPos.y), right(vPos.x+width), bottom(vPos.y+height) {};

	void Init( int l, int t, int r, int b ) {left = l; top = t; right = r; bottom = b; };
    void Init( JIVector vPos, int width, int height ) {left = vPos.x; top = vPos.y; right = vPos.x+width; bottom = vPos.y+height; };
    void Init( JRect &rcIn ) {left = rcIn.left; top = rcIn.top; right = rcIn.right; bottom = rcIn.bottom; };

	int Left()	{ return left; };
	int Top()	{ return top; };
	int Right() { return right; };
	int Bottom(){ return bottom; };
	int Width()	{ return Right()-Left(); };
	int Height(){ return Bottom()-Top(); };
    
    void SetWidth( int dwWidth, bool setFromLeft=true )
    {
        if( setFromLeft ) right = left + dwWidth;
        else left = right - dwWidth;
    };
    void SetHeight( int dwHeight, bool setFromTop=true )
    {
        if( setFromTop ) bottom = top + dwHeight;
        else top = bottom - dwHeight;
    };

	bool IsInWorld()
    {
        if( left < 0 || right > DUNG_WIDTH
           || top < 0 || bottom > DUNG_HEIGHT )
        {
            return false;
        }
        if( left > DUNG_WIDTH || right < 0
           || top > DUNG_HEIGHT || bottom < 0  )
        {
            printf("JRect has bad memory allocation: <%d %d, %d %d>\n", left, top, right, bottom);
            return false;
        }

		return true;
	}
};

#endif // __JRECT_H__
