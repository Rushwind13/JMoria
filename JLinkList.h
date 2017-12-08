// jlinklist.h
//
// simple templated linked-list
//
// Jimbo S. Harris 5/10/2002
#ifndef __JLINKLIST_H__
#define __JLINKLIST_H__
#include <stdio.h>

template <class T> class CLink
{
public:
	T	*m_lpData;
	CLink *next, *prev;
	
	inline CLink( T *pData = NULL)
		:m_lpData(pData),
		next(NULL),
		prev(NULL)
	{
	};
	
	inline ~CLink(void)
	{
		m_lpData = NULL;
		next = NULL;
		prev = NULL;
	}
};

template <class T> class JLinkList
{
public:
	inline JLinkList< T >(): m_lpHead(NULL),m_iNumElements(0){};
	virtual inline ~JLinkList(void)
	{
		Terminate();
	};
	// implemented as "addtail",
	// but note that m_lpHead->prev == tail.
	// in other fcns, this spot is checked for,
	// and NULL is returned in a case where you're
	// trying to go between m_lpHead and m_lpHead->prev.
	CLink <T> *Add( T *pData )
	{
		CLink<T> *pLink = new CLink<T>(pData);

		pLink->prev = NULL;
		pLink->next = m_lpHead;
		
		if( m_lpHead != NULL )
		{
			m_lpHead->prev = pLink;
		}

		m_lpHead = pLink;
		
		m_iNumElements++;
		return pLink;
	};
	
	CLink <T> *GetHead() { return m_lpHead; };
	void Remove( CLink <T> *pLink )
	{
		if( pLink->next )
		{
			pLink->next->prev = pLink->prev;
		}

		if( pLink->prev )
		{
			pLink->prev->next = pLink->next;
		}

		
		if( pLink == m_lpHead )
		{
			if( m_iNumElements == 1 )
			{
				m_lpHead = NULL;
			}
			else
			{
				m_lpHead = pLink->next;
			}
		}

		delete pLink->m_lpData;
		delete pLink;
		pLink = NULL;
		m_iNumElements--;
	};
	
	CLink <T> *GetNext( CLink <T> *pLink )
	{
		if( pLink->next == m_lpHead )
		{
			return NULL;
		}
		return pLink->next;
	};
	
	CLink <T> *GetPrev( CLink <T> *pLink )
	{
		if( pLink == m_lpHead )
		{
			return NULL;
		}
		return pLink->prev;
	};
    
    CLink <T> *GetLink( int which_link )
    {
        int count = 0;
        CLink <T> *curr_link = GetHead();
        while( count < which_link )
        {
            if( curr_link->next == NULL ) return curr_link;
            curr_link = GetNext(curr_link);
            count++;
        }
        return curr_link;
    }
    int length() { return m_iNumElements; }
	
	void Terminate()
	{
		if( m_lpHead == NULL )
		{
			return;
		}
		
		CLink<T> *pLink = m_lpHead;
		CLink<T> *pDel = m_lpHead;
		while( pLink )
		{
			pDel = pLink;
			pLink = GetNext(pLink);
			Remove(pDel);
		}
	};
	
protected:
	CLink<T>	*m_lpHead;
	int	m_iNumElements;
	
private:
};
#endif // __LINKLIST_H__
