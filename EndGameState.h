//
//  EndGameState.h
//  JMoria
//
//  Created by Jimbo S. Harris on 12/16/17.
//  Copyright © 2017 Jimbo S. Harris. All rights reserved.
//

#ifndef EndGameState_h
#define EndGameState_h
#include "JMDefs.h"
#include "StateBase.h"

class CEndGameState;
typedef int (CEndGameState::*EndGameKeyHandler)(SDL_Keysym *keysym);
enum eEndGameModifier
{
    ENDGAME_INVALID=-1,
    ENDGAME_INIT=0,
    ENDGAME_TOMB=1,
    ENDGAME_SCORES,
    ENDGAME_MAX
};
class CEndGameState : public CStateBase
{
    // Member Variables
public:
protected:
    char m_cCommand;
    EndGameKeyHandler    m_pKeyHandlers[ENDGAME_MAX];
    EndGameKeyHandler    m_pCurKeyHandler;
    
    eEndGameModifier    m_eCurModifier;
private:
    char *m_szTombstone;
    
    // Member Functions
public:
    CEndGameState();
    ~CEndGameState();
    
    virtual void OnUpdate() {};
    virtual int OnBaseHandleKey( SDL_Keysym *keysym );
    virtual int OnHandleKey( SDL_Keysym *keysym );
protected:
private:
    int OnHandleTomb( SDL_Keysym *keysym );
    int OnHandleScores( SDL_Keysym *keysym );
    int OnHandleInit( SDL_Keysym *keysym );
    
    void ResetToState( int newstate );
    
    bool DoTomb();
    bool DoScores();
};
#endif /* EndGameState_h */
