#ifndef __STATES_H__
#define __STATES_H__
#include "StateBase.h"

// I expect that this file will go away as I fully define each game state
// and move it to its own file...
// Jimbo 2/22/2002
#include "CmdState.h"
#include "ModState.h"

class CMenuState : public CStateBase
{
	// Member Variables
	public:
	protected:
	private:

	// Member Functions
	public:
	protected:
	virtual int OnHandleKey( SDL_keysym *keysym );
	private:
};

class CHelpState : public CStateBase
{
	// Member Variables
	public:
	protected:
	private:

	// Member Functions
	public:
	protected:
	virtual int OnHandleKey( SDL_keysym *keysym );
	private:
};

class CSpellState : public CStateBase
{
	// Member Variables
	public:
	protected:
	private:

	// Member Functions
	public:
	protected:
	virtual int OnHandleKey( SDL_keysym *keysym );
	private:
};

class CUseState : public CStateBase
{
	// Member Variables
	public:
	protected:
	private:

	// Member Functions
	public:
	protected:
	virtual int OnHandleKey( SDL_keysym *keysym );
	private:
};

class CRestState : public CStateBase
{
	// Member Variables
	public:
	protected:
	private:

	// Member Functions
	public:
	protected:
	virtual int OnHandleKey( SDL_keysym *keysym );
	private:
};

#endif // __STATES_H__