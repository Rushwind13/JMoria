#ifndef __MONSTER_H__
#define __MONSTER_H__

#include "JMDefs.h"
#include "JLinkList.h"
#include "AIMgr.h"

class CAttack
{
public:
	CAttack():m_dwType(-1),m_dwEffect(-1),m_szDamage(NULL){};
	~CAttack(){};
	int m_dwType;
	int m_dwEffect;
	char *m_szDamage;
};

// There will be 1 instance of CMonsterTileDef for each line in Monsters.dat
class CMonsterDef
{
	// Member Variables
public:
	CMonsterDef()
	:m_dwFlags(0),
	m_dwType(0),
	m_dwIndex(0),
	m_fBaseHP(0.0f),
	m_fBaseAC(0.0f),
	m_szName(NULL)
	{
		m_llAttacks = new JLinkList<CAttack>;
	};
	~CMonsterDef()
	{
		if(m_szName)
		{
			delete [] m_szName;
			m_szName = NULL;
		}
	}
	int m_dwFlags;
	int m_dwType; // MT_KOBOLD, MT_DRAGON, etc.
	int m_dwIndex; // index into the monster tileset for this monster type
	float m_fBaseHP; // how many HP does this monster start with?
	float m_fBaseAC; // what AC does this monster start with?
	char *m_szName; // What do we call this thing?
	JColor	m_Color; // What color do we draw this thing? (Make it appropriate to the monster name...)
	JLinkList<JColor> *m_Colors; // for multi-hued
	float m_fSpeed; // speed of monster (affects update rate)
    int m_dwMoveType; // movement type of monster
	char *m_szPlural; // how to refer to more than one of this monster
	char *m_szHD; // NdM form of this monster's hit dice.
	JLinkList <CAttack> *m_llAttacks; // this monster's attacks
	int	m_dwLevel; // earliest dungeon level to place this monster
	int m_dwExpValue; // how much XP do you get for killing this monster
protected:
private:
	
	// Member Functions
public:
protected:
private:
};

// There will be 1 instance of CMonster for each monster on the current dungeon level
class CMonster
{
	// Member Variables
public:
	float m_fCurHP;
	float m_fCurAC;
//	JVector m_vPos;
	CMonsterDef *m_md;
	CLink <CMonster> *m_pllLink;
	CAIBrain *m_pBrain; // this is the place to get info for the AI.

	char *GetName()
	{
		if( !m_md )
		{
			return NULL;
		}

		return m_md->m_szName;
	};

	bool	Hit( float &fRoll );
	int		Damage( float fDamageMult );

	JVector GetPos()
	{
		if( m_pBrain != NULL )
		{
			return m_pBrain->m_vPos;
		}

		return JVector(-9999,-9999);
	}

	void SetPos(JVector vNewPos)
	{
		if( m_pBrain != NULL )
		{
			m_pBrain->m_vPos = vNewPos;
		}
	}
protected:
    void SetColor();
private:
	
	// Member Functions
public:
	CMonster();
	~CMonster();
	
	JResult Init(CMonsterDef *pmd);	
	JResult SpawnMonster();
	void PreDraw();
	void Draw();
	void PostDraw();
protected:
private:
};


#endif // __MONSTER_H__
