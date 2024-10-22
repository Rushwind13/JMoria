//
//  EndGameState.h
//  JMoria
//
//  Created by Jimbo S. Harris on 12/16/17.
//  Copyright © 2017 Jimbo S. Harris. All rights reserved.
//

#ifndef EndGameState_h
#define EndGameState_h
#include "JLinkList.h"
#include "JMDefs.h"
#include "StateBase.h"

#include "Dungeon.h"
#include "Player.h"

#include <time.h>

class CScore
{
public:
    CScore()
        : m_szName( NULL ),
          m_szClass( NULL ),
          m_szRace( NULL ),
          m_szKilledBy( NULL ),
          m_szToString( NULL ),
          m_dwLevel( 0 ),
          m_dwDepth( 0 ),
          m_dwScore( 0 ),
          m_dwDate( 0 ) {};
    ~CScore()
    {
        if( m_szName )
        {
            delete[] m_szName;
            m_szName = NULL;
        }

        if( m_szClass )
        {
            delete[] m_szClass;
            m_szClass = NULL;
        }

        if( m_szRace )
        {
            delete[] m_szRace;
            m_szRace = NULL;
        }

        if( m_szKilledBy )
        {
            delete[] m_szKilledBy;
            m_szKilledBy = NULL;
        }
    };

    void InitScore()
    {
        m_szName = new char[strlen( g_pGame->GetPlayer()->GetName() ) + 1];
        strcpy( m_szName, g_pGame->GetPlayer()->GetName() );

        m_szClass = new char[strlen( g_pGame->GetPlayer()->GetClass()->m_szName ) + 1];
        strcpy( m_szClass, g_pGame->GetPlayer()->GetClass()->m_szName );

        m_szRace = new char[strlen( g_pGame->GetPlayer()->GetRace()->m_szName ) + 1];
        strcpy( m_szRace, g_pGame->GetPlayer()->GetRace()->m_szName );

        m_szKilledBy = new char[strlen( g_pGame->GetPlayer()->m_szKilledBy ) + 1];
        strcpy( m_szKilledBy, g_pGame->GetPlayer()->m_szKilledBy );

        m_dwLevel = g_pGame->GetPlayer()->GetLevel();
        m_dwDepth = g_pGame->GetDungeon()->depth;
        m_dwScore = (int)g_pGame->GetPlayer()->GetExperience();
        m_dwDate = time( NULL );
    };
    void InitToString()
    {
        m_szToString = new char[100];
        memset( m_szToString, 0, 100 );
        char datebuffer[80];
        strftime( datebuffer, sizeof( datebuffer ), "%Y-%m-%d", localtime( &m_dwDate ) );
        sprintf( m_szToString, "%d - %s, level %d %s %s, killed at %d' by a %s (%s)", m_dwScore,
                 m_szName, m_dwLevel, m_szRace, m_szClass, ( m_dwDepth * 50 ), m_szKilledBy,
                 datebuffer );
    };
    char *GetName() { return m_szToString; }

public:
    char *m_szName;
    char *m_szClass;
    char *m_szRace;
    char *m_szKilledBy;
    int m_dwLevel;
    int m_dwDepth;
    int m_dwScore;
    long m_dwDate;
    char *m_szToString;
};

class CEndGameState;
typedef int ( CEndGameState::*EndGameKeyHandler )( SDL_Keysym *keysym );
enum eEndGameModifier
{
    ENDGAME_INVALID = -1,
    ENDGAME_INIT = 0,
    ENDGAME_TOMB = 1,
    ENDGAME_SCORES,
    ENDGAME_MAX
};
class CEndGameState : public CStateBase
{
    // Member Variables
public:
protected:
    char m_cCommand;
    EndGameKeyHandler m_pKeyHandlers[ENDGAME_MAX];
    EndGameKeyHandler m_pCurKeyHandler;

    eEndGameModifier m_eCurModifier;
    JLinkList<CScore> *m_llScores;

private:
    char *m_szTombstone;
    CScore *m_pScore;

    // Member Functions
public:
    CEndGameState();
    ~CEndGameState();

    virtual void OnUpdate( float fCurTime ) {}
    virtual int OnBaseHandleKey( SDL_Keysym *keysym );
    virtual int OnHandleKey( SDL_Keysym *keysym );

protected:
private:
    int OnHandleTomb( SDL_Keysym *keysym );
    int OnHandleScores( SDL_Keysym *keysym );
    int OnHandleInit( SDL_Keysym *keysym );

    void ResetToState( int newstate );

    bool DoTomb();
    bool InitScores();
    bool DoScores();
};
#endif /* EndGameState_h */
