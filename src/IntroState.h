//
//  IntroState.h
//  JMoria
//
//  Created by Jimbo S. Harris on 11/18/24.
//  Copyright © 2024 Jimbo S. Harris. All rights reserved.
//

#ifndef IntroState_h
#define IntroState_h
#include "JLinkList.h"
#include "JMDefs.h"
#include "StateBase.h"

#include "Dungeon.h"
#include "Player.h"

#include <time.h>

class CIntroState;
typedef int ( CIntroState::*IntroKeyHandler )( JKeysym *keysym );
enum eIntroModifier
{
    INTRO_INVALID = -1,
    INTRO_INIT = 0,
    INTRO_SPLASH = 1,
    INTRO_CREATE,
    INTRO_MAX
};
class CIntroState : public CStateBase
{
    // Member Variables
public:
protected:
    char m_cCommand;
    IntroKeyHandler m_pKeyHandlers[INTRO_MAX];
    IntroKeyHandler m_pCurKeyHandler;

    eIntroModifier m_eCurModifier;

private:
    // Member Functions
public:
    CIntroState();
    ~CIntroState();

    virtual void OnUpdate( float fCurTime ) {}
    virtual int OnBaseHandleKey( JKeysym *keysym );
    virtual int OnHandleKey( JKeysym *keysym );

protected:
private:
    int OnHandleSplash( JKeysym *keysym );
    int OnHandleCharacterCreate( JKeysym *keysym );
    int OnHandleInit( JKeysym *keysym );

    void ResetToState( int newstate );

    bool DoSplash();
    bool DoCharacterCreation();

public:
protected:
private:
    char *m_szSplash;
};
#endif /* IntroState_h */
