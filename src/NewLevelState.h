#ifndef __NEWLEVELSTATE_H__
#define __NEWLEVELSTATE_H__
#include "JMDefs.h"
#include "StateBase.h"

class CNewLevelState : public CStateBase
{
	// Member Variables
public:
protected:
	// "temp" variables for level generation go here.
private:
	
	// Member Functions
public:
	CNewLevelState() {};
	
	void OnUpdate() {};
	
protected:
	virtual int OnHandleKey( SDL_Keysym *keysym );

private:
};

#endif //__NEWLEVELSTATE_H__