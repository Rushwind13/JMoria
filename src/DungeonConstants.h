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

#define DUNG_ROOM_MINWIDTH 6
#define DUNG_ROOM_MINHEIGHT 6
// #define DUNG_ROOM_MAXWIDTH	SCREEN_MIN_XOFF + SCREEN_MAX_XOFF - 2
// #define DUNG_ROOM_MAXHEIGHT	SCREEN_MIN_YOFF + SCREEN_MAX_YOFF - 2
#define DUNG_ROOM_MAXWIDTH 20
#define DUNG_ROOM_MAXHEIGHT 20

// Dungeon generation algorithm constants
#define MAX_TRIES 10 // Maximum placement attempts for rooms/hallways before giving up
#define HALLWAY_LEADS_TO_ROOM_PERCENT 80 // 80% chance hallway leads to room, 20% to more hallways

// Tile placement offsets
#define DOOR_OFFSET 1 // Distance from room edge to place door
#define WALL_OFFSET 2 // Distance from room edge to start hallway/room connection

#define DUNG_MAXDEPTH 1000 // in levels * 50ft

#endif // __DUNGEONCONSTANTS_H__
