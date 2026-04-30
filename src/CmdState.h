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
    virtual int OnHandleKey( JKeysym *keysym );

private:
    bool IsModifierNeeded( JKeysym *keysym );
    bool IsUseCommand( JKeysym *keysym );
    bool IsStairsCommand( JKeysym *keysym );
    bool IsLookCommand( JKeysym *keysym );
    bool IsMagicCommand( JKeysym *keysym ) { return false; }
    bool IsMenuCommand( JKeysym *keysym ) { return false; }
    bool IsHelpCommand( JKeysym *keysym ) { return false; }
    bool IsRestCommand( JKeysym *keysym );
    bool IsTargetCommand( JKeysym *keysym );
    bool IsTeleportCommand( JKeysym *keysym );
    bool IsSetIntrinsicCommand( JKeysym *keysym );
    bool IsFireCommand( JKeysym *keysym );
    bool IsZapCommand( JKeysym *keysym );
    bool IsXchangeCommand( JKeysym *keysym );
    bool IsCreateItemCommand( JKeysym *keysym );
    bool IsSummonMonsterCommand( JKeysym *keysym );
    bool IsPickupCommand( JKeysym *keysym );
    bool IsExitWizardCommand( JKeysym *keysym );
    bool IsSearchCommand( JKeysym *keysym );
    bool IsStringInputCommand( JKeysym *keysym );
    bool IsToggleCommand( JKeysym *keysym );
    void ResetToState( int newstate ) {}

    int OnHandleStairs( JKeysym *keysym );
    int TestStairs();

    void DisplayInventory();

    // This variable gets deleted a lot, but is better than
    // creating a vew vector every frame.
    JVector m_vNewPos;
};

#endif // __CMDSTATE_H__
