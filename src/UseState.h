#ifndef __MENUSTATE_H__
#define __MENUSTATE_H__
#include "Item.h"
#include "JLinkList.h"
#include "JMDefs.h"
#include "StateBase.h"

class CUseState;
typedef int ( CUseState::*UseKeyHandler )( JKeysym *keysym );
enum eUseModifier
{
    USE_INVALID = -1,
    USE_WIELD = 0,
    USE_REMOVE = 1,
    USE_INIT,
    USE_DROP,
    USE_QUAFF,
    USE_READ,
    USE_FUEL,
    USE_IDENTIFY,
    USE_MAX
};

class CUseState : public CStateBase
{
    // Member Variables
public:
protected:
    char m_cCommand;
    int m_dwSelected;
    CLink<CItem> *m_pSelected;

private:
    // Member Functions
public:
    CUseState();
    ~CUseState() {}

    virtual void OnUpdate( float fCurTime ) {}
    virtual int OnBaseHandleKey( JKeysym *keysym, eUseModifier whichUse );
    virtual int OnHandleKey( JKeysym *keysym );

    eUseModifier GetModifier() { return m_eCurModifier; }

protected:
    CLink<CItem> *GetResponse( eUseModifier whichUse );

private:
    UseKeyHandler m_pKeyHandlers[USE_MAX];
    UseKeyHandler m_pCurKeyHandler;

    eUseModifier m_eCurModifier;

    int OnHandleWield( JKeysym *keysym );
    int OnHandleRemove( JKeysym *keysym );
    int OnHandleInit( JKeysym *keysym );
    int OnHandleDrop( JKeysym *keysym );
    int OnHandleQuaff( JKeysym *keysym );
    int OnHandleRead( JKeysym *keysym );
    int OnHandleFuel( JKeysym *keysym );
    int OnHandleIdentify( JKeysym *keysym );

    bool TestWield();
    bool DoWield();

    bool TestRemove();
    bool DoRemove();

    bool TestDrop();
    bool DoDrop();

    bool TestQuaff();
    bool DoQuaff();

    bool TestRead();
    bool DoRead();

    bool TestFuel();
    bool DoFuel();

    void ResetToState( int newstate );
};

#endif // __MENUSTATE_H__
