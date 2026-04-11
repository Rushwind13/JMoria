// RenderNull.h
//
// No-op renderer for headless/test use — no ncurses, no SDL, no OpenGL.
//
#ifndef __RENDERNULL_H__
#define __RENDERNULL_H__

#include "RenderBase.h"

class CRenderNull : public IRenderBackend
{
public:
    JResult Init( int, int, int ) override { return JSUCCESS; }
    void Term() override {}

    void PreDraw() override {}
    void PostDraw() override {}
    void SwapBuffers() override {}

    void PreDrawTile() override {}
    void PostDrawTile() override {}
    void SetTileColor( JColor ) override {}
    bool DrawTile( const JFVector &, JVector &, JIVector &, JFVector & ) override { return true; }
    bool DrawTile( const JFVector &, JVector &, JIVector & ) override { return true; }

    void PreDrawObjects( JRect, uint32, bool, bool, JFVector * ) override {}
    void PostDrawObjects() override {}

    void DrawTextBoundingBox( JRect, JColor ) override {}
    bool DrawChar( const JFVector &, JVector &, char ) override { return true; }

    int GetScreenWidth() const override { return 80; }
    int GetScreenHeight() const override { return 24; }
};

#endif // __RENDERNULL_H__
