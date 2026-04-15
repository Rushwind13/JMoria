#ifndef __TARGETSTATE_H__
#define __TARGETSTATE_H__
#include "JMDefs.h"

#include "StateBase.h"

class CTargetState;
class CMonster;
typedef int ( CTargetState::*TargetKeyHandler )( JKeysym *keysym );
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
    uint32 m_dwCurrentSelection;
    int m_dwPreviousState;

private:
    // Member Functions
public:
    CTargetState();
    ~CTargetState();

    virtual void OnUpdate( float fCurTime ) {}
    virtual int OnBaseHandleKey( JKeysym *keysym );
    virtual int OnHandleKey( JKeysym *keysym );

protected:
private:
    TargetKeyHandler m_pKeyHandlers[TARGET_MAX];
    TargetKeyHandler m_pCurKeyHandler;

    eTargetModifier m_eCurModifier;

    int OnHandleTarget( JKeysym *keysym );
    int OnHandleInit( JKeysym *keysym );

    int DoInit();
    void UpdateLOSLine();

    bool TestTarget();
    bool DoTarget();

    void ResetToState( int newstate );
};

#endif // __TARGETSTATE_H__
