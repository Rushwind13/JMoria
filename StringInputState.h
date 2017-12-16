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
typedef int (CStringInputState::*StringInputKeyHandler)(SDL_Keysym *keysym);
enum eStringInputModifier
{
    SI_INVALID=-1,
    SI_INIT=0,
    SI_NAME=1,
    SI_HAGGLE,
    SI_MAX
};
class CStringInputState : public CStateBase
{
    // Member Variables
public:
protected:
    char m_cCommand;
    StringInputKeyHandler    m_pKeyHandlers[SI_MAX];
    StringInputKeyHandler    m_pCurKeyHandler;
    
    eStringInputModifier    m_eCurModifier;
    char m_szInput[32];
private:
    
    // Member Functions
public:
    CStringInputState();
    ~CStringInputState() {};
    
    virtual void OnUpdate() {};
    virtual int OnBaseHandleKey( SDL_Keysym *keysym );
    virtual int OnHandleKey( SDL_Keysym *keysym );
protected:
private:
    int OnHandleName( SDL_Keysym *keysym );
    int OnHandleHaggle( SDL_Keysym *keysym );
    int OnHandleInit( SDL_Keysym *keysym );

    bool TestName();
    bool DoName();
    
    bool TestHaggle();
    bool DoHaggle();

    bool IsAlpha(SDL_Keysym *keysym) {return false;};
    char GetInput(SDL_Keysym *keysym) { return nul;};

    void ResetToState( int newstate );
};

#endif /* StringInputState_h */
