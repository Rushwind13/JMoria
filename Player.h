#ifndef __PLAYER_H__
#define __PLAYER_H__
#include "JMDefs.h"
#include "JLinkList.h"

class CTileset;
class CItem;

class CPlayer
{
public:
    CPlayer():m_bHasSpawned(false)
    {
        m_llInventory = new JLinkList<CItem>;
        m_llEquipment = new JLinkList<CItem>;
    };
    ~CPlayer() { Term(); };

	void Init();
	void Term()
    {
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
    };
	bool Update( float fCurTime );
	void PreDraw();
	void Draw();
    void PostDraw();
    void DisplayInventory();
    void DisplayEquipment();
    void PickUp( JVector &vPickupPos );
    
    bool IsWieldable(CLink<CItem> *pLink);
    bool Wield(CLink<CItem> *pItem);
    
    bool IsRemovable(CLink<CItem> *pLink);
    bool Remove(CLink<CItem> *pLink);
    
	JVector m_vPos;
	CTileset *m_TileSet;
    bool m_bHasSpawned;
    
    JLinkList<CItem> *m_llInventory;
    JLinkList<CItem> *m_llEquipment;
protected:
	JResult SpawnPlayer();
};
#endif // __PLAYER_H__
