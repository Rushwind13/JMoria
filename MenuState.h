#ifndef __MENUSTATE_H__
#define __MENUSTATE_H__
#include "JMDefs.h"
#include "StateBase.h"
#include "JLinkList.h"
#include "Item.h"

class CMenuState;
typedef int (CMenuState::*MenuKeyHandler)(SDL_Keysym *keysym);
enum eMenuModifier
{
	MENU_INVALID=-1,
	MENU_WIELD=0,
	MENU_UNWIELD=1,
    MENU_INIT,
	MENU_MAX
};

class CMenuState : public CStateBase
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
	CMenuState();
	~CMenuState() {};


	virtual void OnUpdate() {};
	virtual int OnBaseHandleKey( SDL_Keysym *keysym, eMenuModifier whichMenu );
	virtual int OnHandleKey( SDL_Keysym *keysym );
protected:
	bool	IsAlpha(SDL_Keysym *keysym);
	CLink<CItem>	*GetResponse(eMenuModifier whichMenu);
	
private:
	MenuKeyHandler	m_pKeyHandlers[MENU_MAX];
	MenuKeyHandler	m_pCurKeyHandler;

	eMenuModifier	m_eCurModifier;

	int OnHandleWield( SDL_Keysym *keysym );
	int OnHandleRemove( SDL_Keysym *keysym );
	int OnHandleInit( SDL_Keysym *keysym );


	bool TestWield();
	bool DoWield();

	bool TestRemove();
	bool DoRemove();
	
	void	ResetToState( int newstate );
};

#endif // __MENUSTATE_H__
