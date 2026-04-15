//
//  ClockStepState.h
//  JMoria
//
//  Created by Jimbo S. Harris on 12/16/17.
//  Copyright © 2017 Jimbo S. Harris. All rights reserved.
//
//  CLOCKSTEP Mode: Visual step-through dungeon generation for debugging
//
//  Enable by adding -DCLOCKSTEP to CFLAGS in Makefile.
//
//  Flow: INTRO → CLOCKSTEP → COMMAND (on ESC)
//  Controls:
//    SPACE - Advance dungeon generation by one tick
//    ESC   - Complete generation, spawn player, start gameplay
//
//  Key implementation details:
//  - Player spawn deferred until ESC pressed (prevents NULL crashes during generation)
//  - m_bLevelPopulated flag ensures scenery/items/monsters placed only once after generation
//  - Viewport centered on entire dungeon (0,0 to 100,100) instead of player position
//  - Visibility checks bypassed (shows all tiles regardless of DUNG_FLAG_SEEN)
//  - Lighting override disabled (tiles use natural colors from definitions)
//

#ifndef ClockStepState_h
#define ClockStepState_h
#include "JMDefs.h"
#include "StateBase.h"

#include "Dungeon.h"

class CClockStepState;
typedef int ( CClockStepState::*ClockStepKeyHandler )( JKeysym *keysym );
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
    bool m_bShowDiagnostics;
    bool m_bLevelPopulated;

    // Member Functions
public:
    CClockStepState();
    ~CClockStepState();

    virtual void OnUpdate( float fCurTime ) {}
    virtual int OnBaseHandleKey( JKeysym *keysym );
    virtual int OnHandleKey( JKeysym *keysym );

protected:
private:
    int OnHandleTick( JKeysym *keysym );
    int OnHandleInit( JKeysym *keysym );

    void ResetToState( int newstate );

    bool DoTick();
    void CompleteGeneration();
};
#endif /* ClockStepState_h */
