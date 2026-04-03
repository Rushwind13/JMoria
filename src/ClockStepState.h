//
//  ClockStepState.h
//  JMoria
//
//  Created by Jimbo S. Harris on 12/16/17.
//  Copyright © 2017 Jimbo S. Harris. All rights reserved.
//

#ifndef ClockStepState_h
#define ClockStepState_h
#include "JMDefs.h"
#include "StateBase.h"

#include "Dungeon.h"

class CClockStepState;
typedef int ( CClockStepState::*ClockStepKeyHandler )( SDL_Keysym *keysym );
enum eClockStepModifier
{
    CLOCKSTEP_INVALID = -1,
    CLOCKSTEP_INIT = 0,
    CLOCKSTEP_TICK = 1,
    CLOCKSTEP_MAX
};
class CClockStepState : public CStateBase
{
    // Member Variables
public:
protected:
    char m_cCommand;
    ClockStepKeyHandler m_pKeyHandlers[CLOCKSTEP_MAX];
    ClockStepKeyHandler m_pCurKeyHandler;

    eClockStepModifier m_eCurModifier;

private:
    int m_dwClock;
    int m_dwStep;

    // Member Functions
public:
    CClockStepState();
    ~CClockStepState();

    virtual void OnUpdate( float fCurTime ) {}
    virtual int OnBaseHandleKey( SDL_Keysym *keysym );
    virtual int OnHandleKey( SDL_Keysym *keysym );

protected:
private:
    int OnHandleTick( SDL_Keysym *keysym );
    int OnHandleInit( SDL_Keysym *keysym );

    void ResetToState( int newstate );

    bool DoTick();
};
#endif /* ClockStepState_h */
