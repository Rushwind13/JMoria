#ifndef __PLAYER_H__
#define __PLAYER_H__
#include "JMDefs.h"
#include "JLinkList.h"

class CTileset;
class CItem;

class CPlayer
{
public:
    CPlayer():m_bHasSpawned(false){};
	~CPlayer() {};

	void Init();
	void Term() {};
	bool Update( float fCurTime );
	void PreDraw();
	void Draw();
	void PostDraw();
	JVector m_vPos;
	CTileset *m_TileSet;
    bool m_bHasSpawned;
protected:
	JResult SpawnPlayer();
    
    JLinkList<CItem> *m_pllInventory;
    JLinkList<CItem> *m_pllEquipment;
};
#endif // __PLAYER_H__
