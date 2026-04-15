//
//  StringInputState.h
//  JMoria
//
//  Created by Jimbo S. Harris on 12/16/17.
//  Copyright © 2017 Jimbo S. Harris. All rights reserved.
//

#ifndef StringInputState_h
#define StringInputState_h
#include "JMDefs.h"
#include "StateBase.h"

class CStringInputState;
typedef int ( CStringInputState::*StringInputKeyHandler )( JKeysym *keysym );
enum eStringInputModifier
{
    SI_INVALID = -1,
    SI_INIT = 0,
    SI_NAME = 1,
    SI_HAGGLE,
    SI_FLAG,
    SI_ITEM,
    SI_MONSTER,
    SI_PASSWORD,
    SI_MAX
};
class CStringInputState : public CStateBase
{
    // Member Variables
public:
protected:
    char m_cCommand;
    StringInputKeyHandler m_pKeyHandlers[SI_MAX];
    StringInputKeyHandler m_pCurKeyHandler;

    eStringInputModifier m_eCurModifier;
    char m_szInput[MAX_STRING_LENGTH];

private:
    // Member Functions
public:
    CStringInputState();
    ~CStringInputState() {}

    virtual void OnUpdate( float fCurTime ) {}
    virtual int OnBaseHandleKey( JKeysym *keysym );
    virtual int OnHandleKey( JKeysym *keysym );

protected:
private:
    int OnHandleName( JKeysym *keysym );
    int OnHandleFlag( JKeysym *keysym );
    int OnHandleItem( JKeysym *keysym );
    int OnHandleMonster( JKeysym *keysym );
    int OnHandleHaggle( JKeysym *keysym );
    int OnHandleInit( JKeysym *keysym );
    int OnHandlePassword( JKeysym *keysym );

    bool TestName();
    bool DoName();

    bool TestHaggle();
    bool DoHaggle();

    bool TestFlag();
    bool DoFlag();

    bool TestItem();
    bool DoItem();

    bool TestMonster();
    bool DoMonster();

    void ResetToState( int newstate );
};

#endif /* StringInputState_h */
