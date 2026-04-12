//
//  RunState.h
//  JMoria
//
//  Created by Jimbo S. Harris on 11/14/24.
//  Copyright © 2024-2026 Jimbo S. Harris. All rights reserved.
//

#ifndef RunState_h
#define RunState_h
#include "JMDefs.h"
#include "StateBase.h"

#include "Dungeon.h"

class CRunState;
typedef int ( CRunState::*RunKeyHandler )( JKeysym *keysym );
enum eRunModifier
{
    RUN_INVALID = -1,
    RUN_INIT = 0,
    RUN_TICK = 1,
    RUN_MAX
};
class CRunState : public CStateBase
{
    // Member Variables
public:
protected:
    char m_cCommand;
    RunKeyHandler m_pKeyHandlers[RUN_MAX];
    RunKeyHandler m_pCurKeyHandler;

    eRunModifier m_eCurModifier;

private:
    int m_dwClock;
    float m_fStateTicks;

    // Member Functions
public:
    CRunState();
    ~CRunState();

    virtual void OnUpdate( float fCurTime )
    {
        m_fStateTicks += fCurTime;
        if( m_fStateTicks >= 1.0f )
        {
            DoTick();
            m_fStateTicks -= 1.0f;
        }
    };
    virtual int OnBaseHandleKey( JKeysym *keysym );
    virtual int OnHandleKey( JKeysym *keysym );

protected:
private:
    int OnHandleTick( JKeysym *keysym );
    int OnHandleInit( JKeysym *keysym );

    void ResetToState( int newstate );

    bool DoTick();
};
#endif /* RunState_h */
