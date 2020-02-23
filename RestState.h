//
//  RestState.h
//  JMoria
//
//  Created by Jimbo S. Harris on 1/1/18.
//  Copyright © 2018 Jimbo S. Harris. All rights reserved.
//

#ifndef RestState_h
#define RestState_h
#include "JMDefs.h"
#include "StateBase.h"

#include "Dungeon.h"

class CRestState;
typedef int (CRestState::*RestKeyHandler)(SDL_Keysym *keysym);
enum eRestModifier
{
    REST_INVALID=-1,
    REST_INIT=0,
    REST_TICK=1,
    REST_MAX
};
class CRestState : public CStateBase
{
    // Member Variables
public:
protected:
    char m_cCommand;
    RestKeyHandler    m_pKeyHandlers[REST_MAX];
    RestKeyHandler    m_pCurKeyHandler;

    eRestModifier    m_eCurModifier;
private:
  int m_dwClock;
    float m_fStateTicks;

    // Member Functions
public:
    CRestState();
    ~CRestState();

    virtual void OnUpdate(float fCurTime)
    {
        m_fStateTicks += fCurTime;
        if( m_fStateTicks >= 1.0f )
        {
            DoTick();
            m_fStateTicks -= 1.0f;
        }
    };
    virtual int OnBaseHandleKey( SDL_Keysym *keysym );
    virtual int OnHandleKey( SDL_Keysym *keysym );
protected:
private:
    int OnHandleTick( SDL_Keysym *keysym );
    int OnHandleInit( SDL_Keysym *keysym );

    void ResetToState( int newstate );

    bool DoTick();
};
#endif /* RestState_h */
