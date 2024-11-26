#ifndef __LOOKSTATE_H__
#define __LOOKSTATE_H__
#include "JMDefs.h"
#include "StateBase.h"

class CLookState;
typedef int ( CLookState::*LookKeyHandler )( SDL_Keysym *keysym );
enum eLookModifier
{
    LOOK_INVALID = -1,
    LOOK_LOOK = 0,
    LOOK_INIT = 1,
    LOOK_MAX
};

class CLookState : public CStateBase
{
    // Member Variables
public:
protected:
    char m_cCommand;
    JVector m_vLookPos;

private:
    // Member Functions
public:
    CLookState();
    ~CLookState() {}

    virtual void OnUpdate( float fCurTime ) {}
    virtual int OnBaseHandleKey( SDL_Keysym *keysym );
    virtual int OnHandleKey( SDL_Keysym *keysym );

protected:
private:
    LookKeyHandler m_pKeyHandlers[LOOK_MAX];
    LookKeyHandler m_pCurKeyHandler;

    eLookModifier m_eCurModifier;

    int OnHandleLook( SDL_Keysym *keysym );
    int OnHandleInit( SDL_Keysym *keysym );

    bool TestLook();
    bool DoLook();

    void ResetToState( int newstate );
};

#endif // __LOOKSTATE_H__
