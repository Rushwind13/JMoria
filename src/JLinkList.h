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
    T *m_lpData;
    int m_dwIndex;
    int m_dwInstanceId;
    CLink *next, *prev;

    inline CLink( T *pData = NULL, int dwIndex = -1, int dwInstanceId = -1 )
        : m_lpData( pData ),
          m_dwIndex( dwIndex ),
          m_dwInstanceId( dwInstanceId ),
          next( NULL ),
          prev( NULL ) {};

    inline ~CLink( void )
    {
        if( m_lpData )
        {
            delete m_lpData;
            m_lpData = NULL;
        }
        m_dwIndex = -1;
        m_dwInstanceId = -1;
        next = NULL;
        prev = NULL;
    }
};

template <class T> class JStack
{
public:
    inline JStack<T>() : m_lpHead( NULL ), m_iNumElements( 0 ) {}
    virtual inline ~JStack( void ) { Terminate(); }
    void Push( T *pData )
    {
        CLink<T> *pLink = new CLink<T>( pData );
        pLink->next = m_lpHead;
        m_lpHead = pLink;
        m_iNumElements++;
    };
    CLink<T> *Pop()
    {
        if( m_lpHead == NULL )
        {
            return NULL;
        }
        CLink<T> *pLink = m_lpHead;
        m_lpHead = pLink->next;
        m_iNumElements--;

        return pLink;
    };

    void Remove( CLink<T> *pLink )
    {
        delete pLink;
        pLink = NULL;
    };

    void Terminate()
    {
        if( m_lpHead == NULL )
        {
            return;
        }

        CLink<T> *pLink = m_lpHead;
        CLink<T> *pDel = pLink;
        while( pLink )
        {
            pDel = pLink;
            pLink = Pop();
            Remove( pDel );
        }
    };

    int length() { return m_iNumElements; }

protected:
    CLink<T> *m_lpHead;
    int m_iNumElements;

private:
};

template <class T> class JLinkList
{
public:
    inline JLinkList<T>() : m_lpHead( NULL ), m_lpTail( NULL ), m_iNumElements( 0 ) {}
    virtual inline ~JLinkList( void ) { Terminate(); };
    CLink<T> *Add( T *pData, int dwIndex = -1, int dwInstanceId = -1, bool bAscending = true )
    {
        CLink<T> *pLink = new CLink<T>( pData, dwIndex, dwInstanceId );

        if( dwIndex != -1 )
        {
            CLink<T> *curr_link = GetHead();
            if( curr_link == NULL )
            {
                pLink->prev = NULL;
                pLink->next = NULL;

                m_lpHead = pLink;
                m_lpTail = pLink;
            }
            else
            {
                while( curr_link )
                {
                    if( ( bAscending && curr_link->m_dwIndex >= dwIndex ) ||
                        ( !bAscending && curr_link->m_dwIndex <= dwIndex ) )
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
                        pLink->next = NULL;
                        curr_link->next = pLink;
                        pLink->prev = curr_link;
                        m_lpTail = pLink;
                        break;
                    }
                    curr_link = GetNext( curr_link );
                }
            }
        }
        else
        {
            pLink->m_dwIndex = m_iNumElements;

            if( m_lpHead == NULL )
            {
                pLink->prev = NULL;
                pLink->next = NULL;
                m_lpHead = pLink;
                m_lpTail = pLink;
            }
            else
            {
                // O(1) append to tail
                m_lpTail->next = pLink;
                pLink->prev = m_lpTail;
                pLink->next = NULL;
                m_lpTail = pLink;
            }
        }

        m_iNumElements++;
        return pLink;
    };

    CLink<T> *GetHead() { return m_lpHead; }
    void Remove( CLink<T> *pLink, bool bDelete = true )
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
                m_lpTail = NULL;
            }
            else
            {
                m_lpHead = pLink->next;
            }
        }

        if( pLink == m_lpTail )
        {
            m_lpTail = pLink->prev;
        }

        if( bDelete )
        {
            delete pLink;
            pLink = NULL;
        }
        m_iNumElements--;
    };

    CLink<T> *GetNext( CLink<T> *pLink )
    {
        if( pLink->next == m_lpHead )
        {
            return NULL;
        }
        return pLink->next;
    };

    CLink<T> *GetPrev( CLink<T> *pLink )
    {
        if( pLink == m_lpHead )
        {
            return NULL;
        }
        return pLink->prev;
    };

    // Retrieve the (0-based) Nth entry in the list
    CLink<T> *GetNthLink( int which_link )
    {
        int count = 0;
        CLink<T> *curr_link = GetHead();

        // empty list; no entry for you
        if( curr_link == NULL )
            return NULL;
        while( count < which_link )
        {
            if( curr_link->next == NULL )
            {
                // printf( "Asked for link %d, but there were only %d in the list.\n", which_link,
                //         count );
                return NULL;
            }
            curr_link = GetNext( curr_link );
            count++;
        }
        return curr_link;
    }

    // Retrieve the entry with the given index (m_dwIndex) from the list
    CLink<T> *GetLink( int which_link )
    {
        int count = 0;
        CLink<T> *curr_link = GetHead();

        // empty list; no entry for you
        if( curr_link == NULL )
            return NULL;
        while( count < which_link )
        {
            if( curr_link->m_dwIndex == which_link )
            {
                return curr_link;
            }

            if( curr_link->next == NULL )
            {
                return NULL;
            }
            curr_link = GetNext( curr_link );
            count++;
        }
        return curr_link;
    }

    // Retrieve the entry with the given instance ID (m_dwInstanceId) from the list
    CLink<T> *GetInstanceId( int dwInstanceId )
    {
        if( dwInstanceId == -1 )
            return NULL;

        CLink<T> *curr_link = GetHead();
        while( curr_link != NULL )
        {
            if( curr_link->m_dwInstanceId == dwInstanceId )
            {
                return curr_link;
            }
            curr_link = GetNext( curr_link );
        }
        return NULL;
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
            pLink = GetNext( pLink );
            Remove( pDel );
        }
    };

protected:
    CLink<T> *m_lpHead;
    CLink<T> *m_lpTail;
    int m_iNumElements;

private:
};
#endif // __LINKLIST_H__
