#ifndef __TARGETSTATE_H__
#define __TARGETSTATE_H__
#include "JMDefs.h"

#include "JLinkList.h"
#include "StateBase.h"

class CTargetState;
class CMonster;
typedef int ( CTargetState::*TargetKeyHandler )( SDL_Keysym *keysym );
enum eTargetModifier
{
    TARGET_INVALID = -1,
    TARGET_TARGET = 0,
    TARGET_INIT = 1,
    TARGET_MAX
};

class CTargetState : public CStateBase
{
    // Member Variables
public:
protected:
    char m_cCommand;
    JLinkList<uint32> *m_llTargets;
    uint32 m_dwCurrentSelection;
    int m_dwPreviousState;

private:
    // Member Functions
public:
    CTargetState();
    ~CTargetState() {}

    virtual void OnUpdate( float fCurTime ) {}
    virtual int OnBaseHandleKey( SDL_Keysym *keysym );
    virtual int OnHandleKey( SDL_Keysym *keysym );

protected:
private:
    TargetKeyHandler m_pKeyHandlers[TARGET_MAX];
    TargetKeyHandler m_pCurKeyHandler;

    eTargetModifier m_eCurModifier;

    int OnHandleTarget( SDL_Keysym *keysym );
    int OnHandleInit( SDL_Keysym *keysym );

    int DoInit();

    bool TestTarget();
    bool DoTarget();

    void ResetToState( int newstate );
};

#endif // __TARGETSTATE_H__
