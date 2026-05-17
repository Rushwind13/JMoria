#ifndef __MODSTATE_H__
#define __MODSTATE_H__
#include "JMDefs.h"
#include "StateBase.h"

class CModState;
typedef int ( CModState::*ModKeyHandler )( JKeysym *keysym );
enum eModModifier
{
    MOD_INVALID = -1,
    MOD_OPEN = 0,
    MOD_TUNNEL = 1,
    MOD_INIT,
    MOD_CLOSE,
    MOD_SPIKE,
    MOD_MAX
};

class CModState : public CStateBase
{
    // Member Variables
public:
protected:
    char m_cCommand;
    JVector m_vNewPos;

private:
    // Member Functions
public:
    CModState();
    ~CModState() {}

    virtual void OnUpdate( float fCurTime ) {}
    virtual int OnBaseHandleKey( JKeysym *keysym );
    virtual int OnHandleKey( JKeysym *keysym );

protected:
private:
    ModKeyHandler m_pKeyHandlers[MOD_MAX];
    ModKeyHandler m_pCurKeyHandler;

    eModModifier m_eCurModifier;

    int OnHandleOpen( JKeysym *keysym );
    int OnHandleTunnel( JKeysym *keysym );
    int OnHandleClose( JKeysym *keysym );
    int OnHandleSpike( JKeysym *keysym );
    int OnHandleInit( JKeysym *keysym );

    bool TestOpen();
    bool DoOpen();

    bool TestTunnel();
    bool DoTunnel();

    bool TestClose();
    bool DoClose();

    bool TestSpike();
    bool DoSpike();
    bool DoUnspike();

    void ResetToState( int newstate );
};

#endif // __MODSTATE_H__
