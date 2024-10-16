#ifndef __MODSTATE_H__
#define __MODSTATE_H__
#include "JMDefs.h"
#include "StateBase.h"

class CModState;
typedef int (CModState::*ModKeyHandler)(SDL_Keysym *keysym);
enum eModModifier
{
	MOD_INVALID=-1,
	MOD_OPEN=0,
	MOD_TUNNEL=1,
	MOD_INIT,
	MOD_CLOSE,
	MOD_MAX
};

class CModState : public CStateBase
{
	// Member Variables
public:
protected:
	char m_cCommand;
	JVector m_vNewPos;
private:
	
	// Member Functions
public:
	CModState();
	~CModState() {};

	virtual void OnUpdate(float fCurTime) {};
	virtual int OnBaseHandleKey( SDL_Keysym *keysym );
	virtual int OnHandleKey( SDL_Keysym *keysym );
protected:
private:
	ModKeyHandler	m_pKeyHandlers[MOD_MAX];
	ModKeyHandler	m_pCurKeyHandler;

	eModModifier	m_eCurModifier;

	int OnHandleOpen( SDL_Keysym *keysym );
	int OnHandleTunnel( SDL_Keysym *keysym );
	int OnHandleClose( SDL_Keysym *keysym );
	int OnHandleInit( SDL_Keysym *keysym );


	bool TestOpen();
	bool DoOpen();

	bool TestTunnel();
	bool DoTunnel();	

	bool TestClose();
	bool DoClose();
	
	void	ResetToState( int newstate );
};

#endif // __MODSTATE_H__
