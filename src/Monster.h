#ifndef __MONSTER_H__
#define __MONSTER_H__

#include "AIMgr.h"
#include "JLinkList.h"
#include "JMDefs.h"

#define COLOR_CHANGE_TIMEOUT 0.266f
#define BREED_TIMEOUT 0.75f

class CAttack
{
public:
    CAttack() : m_dwType( -1 ), m_dwEffect( -1 ), m_szDamage( NULL ) {}
    ~CAttack() {}
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
        : m_dwFlags( 0 ),
          m_dwType( 0 ),
          m_dwIndex( 0 ),
          m_dwLevel( 0 ),
          m_fExpValue( 0 ),
          m_dwMoveType( 0 ),
          m_fBaseHP( 0.0f ),
          m_fBaseAC( 0.0f ),
          m_fSpeed( 0.0f ),
          m_szName( NULL ),
          m_szPlural( NULL ),
          m_szAppear( NULL ),
          m_szHD( NULL )
    {
        m_llAttacks = new JLinkList<CAttack>;
        m_Colors = new JLinkList<JColor>;
        m_szAppear = new char[10];
        sprintf( m_szAppear, "1d1" );
    };
    ~CMonsterDef()
    {
        if( m_szName )
        {
            delete[] m_szName;
            m_szName = NULL;
        }
        if( m_szPlural )
        {
            delete[] m_szPlural;
            m_szPlural = NULL;
        }
        if( m_szAppear )
        {
            delete[] m_szAppear;
            m_szAppear = NULL;
        }
        if( m_szHD )
        {
            delete[] m_szHD;
            m_szHD = NULL;
        }
        if( m_llAttacks )
        {
            m_llAttacks->Terminate();
            m_llAttacks = NULL;
        }
        if( m_Colors )
        {
            m_Colors->Terminate();
            m_Colors = NULL;
        }
    }
    int m_dwFlags;
    int m_dwType;    // MT_KOBOLD, MT_DRAGON, etc.
    int m_dwIndex;   // index into the monster tileset for this monster type
    float m_fBaseHP; // how many HP does this monster start with?
    float m_fBaseAC; // what AC does this monster start with?
    char *m_szName;  // What do we call this thing?
    JColor
        m_Color; // What color do we draw this thing? (Make it appropriate to the monster name...)
    JLinkList<JColor> *m_Colors;     // for multi-hued
    float m_fSpeed;                  // speed of monster (affects update rate)
    int m_dwMoveType;                // movement type of monster
    char *m_szPlural;                // how to refer to more than one of this monster
    char *m_szAppear;                // how many copies of this monster show up at first
    char *m_szHD;                    // NdM form of this monster's hit dice.
    JLinkList<CAttack> *m_llAttacks; // this monster's attacks
    int m_dwLevel;                   // earliest dungeon level to place this monster
    float m_fExpValue;               // how much XP do you get for killing this monster
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
    float m_fHP;
    float m_fCurHP;
    float m_fLastHPTime;
    float m_fCurAC;
    // 	JVector m_vPos;
    float m_fColorChangeInterval;
    float m_fBreedInterval;
    CMonsterDef *m_md;
    CLink<CMonster> *m_pllLink;
    CAIBrain *m_pBrain; // this is the place to get info for the AI.

    char *GetName()
    {
        if( !m_md )
        {
            return NULL;
        }

        return m_md->m_szName;
    };

    float Attack();
    float Damage( float fDamageMult );

    bool Hit( float &fRoll );
    int TakeDamage( float fDamageMult );

    JVector GetPos()
    {
        if( m_pBrain != NULL )
        {
            return m_pBrain->m_vPos;
        }

        return JVector( -9999, -9999 );
    }

    void SetPos( JVector vNewPos )
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

    static JResult CreateMonster( CMonsterDef *pmd, JVector vNear = JVector(-1,-1) );
    void Breed();
    JResult SpawnMonster(JVector vSpawnPoint = JVector(-1,-1));
    bool SpawnAt(JVector vPos);
    void PreDraw();
    void Draw();
    void PostDraw();
#ifdef UNIT_TEST
public:
#else
protected:
#endif
    void Init( CMonsterDef *pmd );
    void InitBrain( CMonsterDef *pmd );

private:
};

#endif // __MONSTER_H__
