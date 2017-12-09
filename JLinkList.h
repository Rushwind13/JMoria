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
    int m_dwIndex;
	CLink *next, *prev;
	
	inline CLink( T *pData = NULL, int dwIndex=-1 )
		:m_lpData(pData),
        m_dwIndex(dwIndex),
		next(NULL),
		prev(NULL)
	{
	};
	
	inline ~CLink(void)
	{
		m_lpData = NULL;
        m_dwIndex = -1;
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
	CLink <T> *Add( T *pData, int dwIndex=-1 )
	{
		CLink<T> *pLink = new CLink<T>(pData, dwIndex);

        if( dwIndex != -1 )
        {
            CLink <T> *curr_link = GetHead();
            if( curr_link == NULL )
            {
                pLink->prev = NULL;
                pLink->next = m_lpHead;
                
                m_lpHead = pLink;
            }
            else
            {
                while( curr_link )
                {
                    if( curr_link->m_dwIndex >= dwIndex )
                    {
                        // found correct spot; insert before curr_link
                        pLink->prev = curr_link->prev;
                        pLink->next = curr_link;
                        if( curr_link->prev == NULL )
                        {
                            m_lpHead = pLink;
                        }
                        else
                        {
                            curr_link->prev->next = pLink;
                        }
                        curr_link->prev = pLink;
                        break;
                        
                    }
                    if( curr_link->next == NULL )
                    {
                        // incoming index > than all in list, insert at tail
                        pLink->next = curr_link->next;
                        curr_link->next = pLink;
                        pLink->prev = curr_link;
                        break;
                    }
                    curr_link = GetNext(curr_link);
                }
            }
        }
        else
        {
            pLink->prev = NULL;
            pLink->next = m_lpHead;
            
            if( m_lpHead != NULL )
            {
                m_lpHead->prev = pLink;
            }
            
            m_lpHead = pLink;
        }
		
		m_iNumElements++;
		return pLink;
	};
	
	CLink <T> *GetHead() { return m_lpHead; };
	void Remove( CLink <T> *pLink, bool bDelete=true )
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

        if( bDelete )
        {
            delete pLink->m_lpData;
            delete pLink;
            pLink = NULL;
        }
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
