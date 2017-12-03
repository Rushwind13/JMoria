#ifndef __PLAYER_H__
#define __PLAYER_H__
#include "JMDefs.h"

class CTileset;

class CPlayer
{
public:
	CPlayer(){};
	~CPlayer() {};

	void Init();
	void Term() {};
	bool Update( float fCurTime );
	void PreDraw();
	void Draw();
	void PostDraw();
	JVector m_vPos;
	CTileset *m_TileSet;
protected:
	JResult SpawnPlayer();
};
#endif // __PLAYER_H__