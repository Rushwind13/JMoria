#ifndef __PLAYER_H__
#define __PLAYER_H__
#include "JMDefs.h"
#include "JLinkList.h"

class CTileset;
class CItem;
class CMonster;
#define PLAYER_BASE_DAMAGE "1d2"
#define CLASS_HD_WARRIOR "1d10"
#define PLAYER_MAX_LEVEL 11

class CClass
{
//Methods
public:
    CClass():
    m_szHD(NULL)
    {
        m_fExpNeeded[0]=20.0f;
        m_fExpNeeded[1]=40.0f;
        m_fExpNeeded[2]=80.0f;
        m_fExpNeeded[3]=180.0f;
        m_fExpNeeded[4]=350.0f;
        m_fExpNeeded[5]=700.0f;
        m_fExpNeeded[6]=1250.0f;
        m_fExpNeeded[7]=2500.0f;
        m_fExpNeeded[8]=5000.0f;
        m_fExpNeeded[9]=7500.0f;
        m_fExpNeeded[10]=10000.0f;

        m_szHD = new char[10];
        sprintf(m_szHD, CLASS_HD_WARRIOR);
        
        memset(m_szName, 0, MAX_STRING_LENGTH);
        strcpy(m_szName, "Warrior");
    };
    ~CClass()
    {
        if( m_szHD )
        {
            delete [] m_szHD;
            m_szHD = NULL;
        }
    };
protected:
private:
// Fields
public:
    float m_fExpNeeded[PLAYER_MAX_LEVEL];
    char *m_szHD;
    char m_szName[MAX_STRING_LENGTH];
protected:
private:
};

class CPlayer
{
public:
    CPlayer():
    m_bHasSpawned(false),
    m_szDamage(NULL),
    m_fDamageModifier(0.0f),
    m_fToHitModifier(0.0f),
    m_fArmorClass(1.0f),
    m_fHitPoints(0.0f),
    m_fCurHitPoints(0.0f),
    m_fExperience(0.0f),
    m_fLevel(1.0f),
    m_pClass(NULL)
    {
        memset(m_szName, 0, MAX_STRING_LENGTH);
        m_pClass = new CClass;
        m_llInventory = new JLinkList<CItem>;
        m_llEquipment = new JLinkList<CItem>;
        m_szDamage = new char[10];
        sprintf(m_szDamage, PLAYER_BASE_DAMAGE);

        m_fHitPoints = Util::Roll(m_pClass->m_szHD);
        m_fCurHitPoints = m_fHitPoints;
    };
    ~CPlayer() { Term(); };

	void Init();
	void Term()
    {
        if(m_pClass)
        {
            delete m_pClass;
            m_pClass = NULL;
        }
        if(m_llInventory)
        {
            m_llInventory->Terminate();
            m_llInventory = NULL;
        }
        if(m_llEquipment)
        {
            m_llEquipment->Terminate();
            m_llEquipment = NULL;
        }
        if( m_szDamage )
        {
            delete [] m_szDamage;
            m_szDamage = NULL;
        }
    };
	bool Update( float fCurTime );
	void PreDraw();
	void Draw();
    void PostDraw();
    void DisplayStats();
    void DisplayInventory(uint8 dwPlacement);
    void DisplayEquipment(uint8 dwPlacement);
    void PickUp( JVector &vPickupPos );
    bool Drop( CItem *pItem );

    bool CanDropHere();

    bool IsWieldable(CLink<CItem> *pLink);
    bool Wield(CLink<CItem> *pItem);

    bool IsRemovable(CLink<CItem> *pLink);
    bool Remove(CLink<CItem> *pLink);
    
    bool IsDrinkable(CLink<CItem> *pLink);
    bool Quaff(CLink<CItem> *pLink);
    
    bool SetName( const char *szName );
    
    float Attack();
    float Damage( float fDamageMult );

    bool Hit( float &fRoll );
    int TakeDamage(float fDamage, char *szMon);

    void OnKillMonster( CMonster *pMon );

	JVector m_vPos;
	CTileset *m_TileSet;
    bool m_bHasSpawned;

    // Inventory
    JLinkList<CItem> *m_llInventory;
    JLinkList<CItem> *m_llEquipment;

    // Combat
    char *m_szDamage;
    float m_fDamageModifier;
    float m_fToHitModifier;
protected:
    JResult SpawnPlayer();
    void GainLevel();


    float m_fArmorClass;
    float m_fHitPoints;
    float m_fCurHitPoints;

    float m_fExperience;
    float m_fLevel;

    CClass *m_pClass;
    char m_szName[MAX_STRING_LENGTH];
};
#endif // __PLAYER_H__
