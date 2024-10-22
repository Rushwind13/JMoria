#ifndef __DUNGEONCONSTANTS_H__
#define __DUNGEONCONSTANTS_H__

// Some maximums for dungeon size (per level)
// dungeon width and height
// #define FIXED_DUNGEON
#ifdef FIXED_DUNGEON
#define DUNG_WIDTH 16
#define DUNG_HEIGHT 16
#else
#define DUNG_WIDTH 100
#define DUNG_HEIGHT 100
#endif

// #define SCREEN_MIN_XOFF 8
// #define SCREEN_MAX_XOFF 16
// #define SCREEN_MIN_YOFF 12
// #define SCREEN_MAX_YOFF 14

#define DUNG_ROOM_MINWIDTH 3
#define DUNG_ROOM_MINHEIGHT 3
// #define DUNG_ROOM_MAXWIDTH	SCREEN_MIN_XOFF + SCREEN_MAX_XOFF - 2
// #define DUNG_ROOM_MAXHEIGHT	SCREEN_MIN_YOFF + SCREEN_MAX_YOFF - 2
#define DUNG_ROOM_MAXWIDTH 12
#define DUNG_ROOM_MAXHEIGHT 12

#define DUNG_MAXDEPTH 1000 // in levels * 50ft

#endif // __DUNGEONCONSTANTS_H__
