#ifndef __GAME_H__
#define __GAME_H__
#include "JMDefs.h"
#include "RenderMode.h"

class CRender;
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

class CGame
{
public:
    CGame();
    ~CGame() { Quit( 0 ); }

    JResult Init( const char *szBasedir, RenderMode mode = RenderMode::OpenGL );
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
    CAIMgr *GetAIMgr() { return m_pAIMgr; }
    void Term();
    void Quit( int returncode );
    void SetState( int eNewState );
    int GetGameStateIndex() { return m_eCurState; }
    CStateBase *GetGameState() { return m_pCurState; }
    float GetFTime() { return m_fGameTime; }
    int GetITime() { return (int)m_fGameTime; }
    int GetTime() { return GetITime(); }

    // ASCII fly-out panel toggles
    void ToggleStats() { m_bShowStats = !m_bShowStats; }
    void ToggleInv() { m_bShowInv = !m_bShowInv; }
    void ToggleEquip() { m_bShowEquip = !m_bShowEquip; }
    bool IsShowingStats() const { return m_bShowStats; }
    bool IsShowingInv() const { return m_bShowInv; }
    bool IsShowingEquip() const { return m_bShowEquip; }

#ifdef TURN_BASED
    void SetReadyForUpdate( const bool isReady ) { m_bReadyForUpdate = isReady; }
#endif
#ifdef CLOCKSTEP
    bool WaitForTick();
#endif

protected:
    CDungeon *m_pDungeon;
    CPlayer *m_pPlayer;
    CAIMgr *m_pAIMgr;

    CDisplayText *m_pMsgsDT;
    CDisplayText *m_pStatsDT;
    CDisplayText *m_pInvDT;
    CDisplayText *m_pEquipDT;
    CDisplayText *m_pUseDT;
    CDisplayText *m_pEndGameDT;

    CStateBase *m_pCurState;
    int m_eCurState;

    CClockStepState *m_pClockStepState;
    CCmdState *m_pCmdState;
    CEndGameState *m_pEndGameState;
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

    // ASCII fly-out panel visibility (toggled by c/i/e keys)
    bool m_bShowStats;
    bool m_bShowInv;
    bool m_bShowEquip;

    void HandleEventsASCII( int &isActive, int &done );
    void UpdateASCIILayout();

    int m_dwNextTime;
    float m_fGameTime;
#ifdef TURN_BASED
    bool m_bReadyForUpdate;
#endif
};
#endif // __GAME_H__
