//
//  MoreState.h
//  JMoria
//
//  Created May 25, 2026.
//
//  CMoreState — waits for any key, then calls CDisplayText::AdvancePage()
//  to show the next chunk of a long message.  All state is owned by the Msgs
//  CDisplayText instance; this state is just the "press any key" gate.

#ifndef __MOREPROMPTSTATE_H__
#define __MOREPROMPTSTATE_H__

#include "JMDefs.h"
#include "StateBase.h"

class CMoreState : public CStateBase
{
public:
    CMoreState() {}
    ~CMoreState() {}

    virtual void Begin( int /*previousState*/ ) {}
    virtual void OnUpdate( float /*fCurTime*/ ) {}
    virtual int OnHandleKey( JKeysym *keysym );

protected:
    virtual void ResetToState( int newstate );
};

#endif // __MOREPROMPTSTATE_H__
