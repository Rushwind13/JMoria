#ifndef __CMDSTATE_H__
#define __CMDSTATE_H__
#include "JMDefs.h"
#include "StateBase.h"

extern JVector vPlayer;

class CCmdState : public CStateBase
{
    // Member Variables
public:
protected:
private:
    // Member Functions
public:
    CCmdState() : m_vNewPos( 0, 0 ) {}
    ~CCmdState() {}

    void OnUpdate( float fCurTime ) {}

protected:
    virtual int OnHandleKey( SDL_Keysym *keysym );

private:
    bool IsModifierNeeded( SDL_Keysym *keysym );
    bool IsUseCommand( SDL_Keysym *keysym );
    bool IsStairsCommand( SDL_Keysym *keysym );
    bool IsMagicCommand( SDL_Keysym *keysym ) { return false; }
    bool IsMenuCommand( SDL_Keysym *keysym ) { return false; }
    bool IsHelpCommand( SDL_Keysym *keysym ) { return false; }
    bool IsRestCommand( SDL_Keysym *keysym );
    bool IsStringInputCommand( SDL_Keysym *keysym );
    bool IsMonster( int collide_type ) { return ( collide_type == DUNG_COLL_MONSTER ); }
    void ResetToState( int newstate ) {}

    int TestCollision( JVector &vTestDir );

    void UpdatePlayerPos( JVector &vDir );

    void PickUpItem( JVector &vDir );
    void HandleCollision( int dwCollideType );

    int OnHandleStairs( SDL_Keysym *keysym );
    int TestStairs();

    void DisplayInventory();

    // This variable gets deleted a lot, but is better than
    // creating a vew vector every frame.
    JVector m_vNewPos;
};

#endif // __CMDSTATE_H__
