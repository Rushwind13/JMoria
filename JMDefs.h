#ifndef __JMDEFS_H__
#define __JMDEFS_H__
#ifdef __WIN32__
#include "windows.h"
#endif //__WIN32__
#include <stdio.h>
#include "Util.h"

#include "JRect.h"
#include "JVector.h"
#include "JColor.h"

typedef int JResult;
typedef unsigned int uint32;

#define nul '\0'

#define JSUCCESS		0
#define JQUITREQUEST	1
#define JRESETSTATE		2
#define JBOGUSKEY		-1

#define INVALID_LENGTH	-1

#define JERROR() printf("An error occurred: %s %d\n", __FILE__, __LINE__ );
#include "DungeonConstants.h"

#include "Constants.h"

#include "Game.h"

class CGame;
extern CGame *g_pGame;
extern JIVector g_vDirDelta[];
extern Constants g_Constants;

#define DUNG_COLL_NO_COLLISION (Sint8)(-1)
#define DUNG_COLL_MONSTER (Sint8)(-2)

// SDL sees the mouse wheel as buttons 4&5
// but has no constants for them.
#define MOUSE_WHEEL_UP		4
#define MOUSE_WHEEL_DOWN	5

// OpenGL needs this defined. vanilla SDL does not.
#define RENDER_TILESET_POSTLOAD_NEEDED
#endif // __JMDEFS_H__