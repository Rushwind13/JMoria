/////
//
// DungeonTile.h
// declaration of classes:
// CDungeonTile - defines the terrain of a single dungeon square
// CDungeonTileDef - Base class for *Def classes, these define each type of terrain
// CWallDef - Attributes of walls
// CFloorDef - Attributes of floors
// CRubbleDef - Attributes of rubble
// CDoorDef - Attributes of doors

#ifndef __DUNGEONTILE_H__
#define __DUNGEONTILE_H__

#include "JMDefs.h"

class CMonster;
class CItem;

class CDungeonTileDef;
class CWallDef;
class CFloorDef;
class CRubbleDef;
// class CDoorDef;

class CDungeonTile;
class CDungeonTileHP;

class CDungeonTile
{
    // Member Variables
public:
    CDungeonTile() : m_dtd( NULL ), m_pCurMonster( NULL ), m_pCurItem( NULL ), m_dwFlags( 0 ) {};

    JVector m_vPos;
    CDungeonTileDef *m_dtd;
    CMonster *m_pCurMonster;
    CItem *m_pCurItem;
    int m_dwFlags; // 32 bits to do whatever you like with.
protected:
private:
    // Member Functions
public:
protected:
private:
};

class CDungeonTileHP : public CDungeonTile
{
    // Member Variables
public:
    int m_dwCurHP; // how far have you busted through the wall, etc.
protected:
private:
    // Member Functions
public:
protected:
private:
};

class CDungeonTileDef
{
    // Member Variables
public:
    CDungeonTileDef() {}
    CDungeonTileDef( const CDungeonTileDef &in )
        : m_dwFlags( in.m_dwFlags ),
          m_dwType( in.m_dwType ),
          m_dwBaseHP( in.m_dwBaseHP )
    {
    }
    int m_dwFlags;
    int m_dwType;         // DTD_WALL, DTD_FLOOR, etc.
    int m_dwModifiedType; // DOOR->OPENDOOR, etc.
    int m_dwIndex;        // index into tileset for this type
    int m_dwBaseHP;       // for busting down walls, disarming traps, etc.
    JColor m_Color;       // add a little color to the world
protected:
private:
    // Member Functions
public:
    virtual int HandleModify( /*cmd?*/ ) { return 0; } // how do you deal with a modify cmd?
    // ... need one of these per state (virt in base, defined in subclasses)?
protected:
private:
};

class CWallDef : public CDungeonTileDef
{
    // Member Variables
public:
protected:
private:
    // Member Functions
public:
    int HandleModify( /* cmd */ )
    {
        /*if( cmd != CMD_TUNNEL )
        {
        Error();
        }
                DoTunnel();/**/

        return 0;
    }

protected:
private:
};

class CFloorDef : public CDungeonTileDef
{
    // Member Variables
public:
    CMonster *m_CurMonster;
    CItem *m_CurItem;

protected:
private:
    // Member Functions
public:
protected:
private:
};

#endif // __DUNGEONTILE_H__