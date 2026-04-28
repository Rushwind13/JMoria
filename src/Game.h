#ifndef __GAME_H__
#define __GAME_H__
#include "JMDefs.h"
#include "RenderMode.h"

#ifdef RENDER_OPENGL
class CRender;
#endif
class IRenderBackend;
class CDungeon;
class CPlayer;
class CDisplayText;
class CStateBase;
class CClockStepState;
class CCmdState;
class CEndGameState;
class CIntroState;
class CLookState;
class CModState;
class CRangedState;
class CRestState;
class CRunState;
class CStringInputState;
class CTargetState;
class CUseState;
class CAIMgr;
class CMonsterRecall;

class CGame
{
public:
    CGame();
    ~CGame() { Quit( 0 ); }

    JResult Init( const char *szBasedir, RenderMode mode );
#ifdef TURN_BASED
    bool Update(); // someday figure out why this doesn't work...
#else
    bool Update( float fCurTime ); // someday figure out why this doesn't work...
#endif // TURN_BASED
    void HandleEvents( int &isActive, int &done );
    void Draw();

    IRenderBackend *GetRender() { return m_pRender; }
    CDungeon *GetDungeon() { return m_pDungeon; }
    CPlayer *GetPlayer() { return m_pPlayer; }
    CDisplayText *GetMsgs() { return m_pMsgsDT; }
    CDisplayText *GetStats() { return m_pStatsDT; }
    CDisplayText *GetInv() { return m_pInvDT; }
    CDisplayText *GetEquip() { return m_pEquipDT; }
    CDisplayText *GetUse() { return m_pUseDT; }
    CDisplayText *GetEnd() { return m_pEndGameDT; }
    CDisplayText *GetMonsters() { return m_pMonstersDT; }
    CAIMgr *GetAIMgr() { return m_pAIMgr; }
    CMonsterRecall *GetMonsterRecall() { return m_pMonRecall; }
    void Term();
    void Quit( int returncode );
    void SetState( int eNewState );
    int GetGameStateIndex() { return m_eCurState; }
    CStateBase *GetGameState() { return m_pCurState; }
    float GetFTime() { return m_fGameTime; }
    int GetITime() { return (int)m_fGameTime; }
    int GetTime() { return GetITime(); }

    // Panel visibility toggles (i=inventory, e=equipment, C=stats, v=monsters)
    void ToggleStats() { m_bShowStats = !m_bShowStats; }
    void ToggleInv() { m_bShowInv = !m_bShowInv; }
    void ToggleEquip() { m_bShowEquip = !m_bShowEquip; }
    void ToggleMonsters() { m_bShowMonsters = !m_bShowMonsters; }
    bool IsShowingStats() const { return m_bShowStats; }
    bool IsShowingInv() const { return m_bShowInv; }
    bool IsShowingEquip() const { return m_bShowEquip; }
    bool IsShowingMonsters() const { return m_bShowMonsters; }

#ifdef TURN_BASED
    void SetReadyForUpdate( const bool isReady ) { m_bReadyForUpdate = isReady; }
    bool IsReadyForUpdate() const { return m_bReadyForUpdate; }
#endif
#ifdef CLOCKSTEP
    bool WaitForTick();
#endif

protected:
    CDungeon *m_pDungeon;
    CPlayer *m_pPlayer;
    CAIMgr *m_pAIMgr;
    CMonsterRecall *m_pMonRecall;

    CDisplayText *m_pMsgsDT;
    CDisplayText *m_pStatsDT;
    CDisplayText *m_pInvDT;
    CDisplayText *m_pEquipDT;
    CDisplayText *m_pUseDT;
    CDisplayText *m_pEndGameDT;
    CDisplayText *m_pMonstersDT;

    CStateBase *m_pCurState;
    int m_eCurState;

    CClockStepState *m_pClockStepState;
    CCmdState *m_pCmdState;
#ifdef UNIT_TEST
public:
#endif
    CEndGameState *m_pEndGameState;

protected:
    CIntroState *m_pIntroState;
    CLookState *m_pLookState;
    CModState *m_pModState;
    CRangedState *m_pRangedState;
    CRestState *m_pRestState;
    CRunState *m_pRunState;
    CStringInputState *m_pStringInputState;
    CTargetState *m_pTargetState;
    CUseState *m_pUseState;

private:
    IRenderBackend *m_pRender;
    RenderMode m_eRenderMode;

    // Panel visibility (toggled by i/e/C/v keys)
    bool m_bShowStats;
    bool m_bShowInv;
    bool m_bShowEquip;
    bool m_bShowMonsters;

    int m_dwNextTime;
    float m_fGameTime;
#ifdef TURN_BASED
    bool m_bReadyForUpdate;
#endif
};
#endif // __GAME_H__
