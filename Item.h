#ifndef __ITEM_H__
#define __ITEM_H__

#include "JMDefs.h"

class CItemDef;

class CItem
{
	// Member Variables
	public:
	JVector m_vPos;
	CItemDef *m_mtd;
	protected:
	private:

	// Member Functions
	public:
		bool IsOpenable() { return false; } // for chests, etc.
		bool IsCloseable() { return false; } // closeable pickup?
		bool IsTunnelable() { return false; } // Tunnelable pickup? unlikely.
	protected:
	private:
};

class CItemDef
{
	// Member Variables
	public:
	int m_dwFlags;
	int m_dwType; // DTD_WALL, DTD_FLOOR, etc.
	int m_dwBaseHP; // for busting down walls, disarming traps, etc.
	protected:
	private:

	// Member Functions
	public:
	virtual int HandleModify(/*cmd?*/); // how do you deal with a modify cmd?
	// ... need one of these per state (virt in base, defined in subclasses)?	
	protected:
	private:
};

#endif // __ITEMTILE_H__