#ifndef __MENUSTATE_H__
#define __MENUSTATE_H__
#include "JMDefs.h"
#include "StateBase.h"
#include "JLinkList.h"
#include "Item.h"

class CUseState;
typedef int (CUseState::*UseKeyHandler)(SDL_Keysym *keysym);
enum eUseModifier
{
	USE_INVALID=-1,
	USE_WIELD=0,
	USE_REMOVE=1,
	USE_INIT,
    USE_DROP,
    USE_QUAFF,
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
	~CUseState() {};


	virtual void OnUpdate() {};
	virtual int OnBaseHandleKey( SDL_Keysym *keysym, eUseModifier whichUse );
	virtual int OnHandleKey( SDL_Keysym *keysym );
    
    eUseModifier GetModifier() { return m_eCurModifier; };
protected:
	CLink<CItem>	*GetResponse(eUseModifier whichUse);

private:
	UseKeyHandler	m_pKeyHandlers[USE_MAX];
	UseKeyHandler	m_pCurKeyHandler;

	eUseModifier	m_eCurModifier;

	int OnHandleWield( SDL_Keysym *keysym );
	int OnHandleRemove( SDL_Keysym *keysym );
	int OnHandleInit( SDL_Keysym *keysym );
    int OnHandleDrop( SDL_Keysym *keysym );
    int OnHandleQuaff( SDL_Keysym *keysym );

	bool TestWield();
	bool DoWield();

	bool TestRemove();
	bool DoRemove();
    
    bool TestDrop();
    bool DoDrop();
    
    bool TestQuaff();
    bool DoQuaff();

	void	ResetToState( int newstate );
};

#endif // __MENUSTATE_H__
