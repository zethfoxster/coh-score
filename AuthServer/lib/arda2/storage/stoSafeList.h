/*****************************************************************************
	created:	2003/03/28
	copyright:	2001,	NCSoft.	All	Rights Reserved
	author(s):	Ryan Prescott
	
	purpose:	
*****************************************************************************/

#ifndef		INCLUDED_stoSafeList
#define		INCLUDED_stoSafeList

#include "arda2/core/corStlQueue.h"
#include "arda2/thread/thrCritical.h"

template <typename T>
class stoSafeList
{
public:
    stoSafeList();
    ~stoSafeList();

    void Add(T item);
    bool Pop(T &item);
    bool Exists(T item);
    bool Remove(T item);
    bool Peek(T &item);

    int Size();
    void Clear();

    void GetItemsCopy(std::vector<T> &items);
    void GetItemsRemove(std::vector<T> &items);
    void AddItems(std::vector<T> &items);


private:
    std::vector<T> m_items;
    thrCriticalSection m_lock;
};


template <typename T> stoSafeList<T>::stoSafeList() 
{
}

template <typename T> stoSafeList<T>::~stoSafeList()
{
}

template <typename T> void stoSafeList<T>::Add(T item)
{
    thrEnterCritical lock(m_lock);
    m_items.push_back(item);
}

template <typename T> bool stoSafeList<T>::Pop(T &item)
{
    thrEnterCritical lock(m_lock);
    if (m_items.size() != 0)
    {
        item = m_items.front();
        m_items.erase( m_items.begin() );
        return true;
    }
    return false;
}

template <typename T> bool stoSafeList<T>::Peek(T &item)
{
    thrEnterCritical lock(m_lock);
    if (m_items.size() != 0)
    {
        item = m_items.front();
        return true;
    }
    return false;
}

template <typename T> bool stoSafeList<T>::Exists(T item)
{
    thrEnterCritical lock(m_lock);
    bool found = false;
    typename std::vector<T>::iterator it = m_items.begin();
    for (; it != m_items.end(); it++)
    {
        if ( *it == item)
        {
            found = true;
            break;
        }
    }
    return found;
}

template <typename T> bool stoSafeList<T>::Remove(T item)
{
    thrEnterCritical lock(m_lock);
    bool found = false;
    typename std::vector<T>::iterator it = m_items.begin();
    for (; it != m_items.end(); it++)
    {
        if ( *it == item)
        {
            m_items.erase(it);
            found = true;
            break;
        }
    }
    return found;
}


template <typename T> int stoSafeList<T>::Size()
{
    thrEnterCritical lock(m_lock);
    return (int)m_items.size();
}

template <typename T> void stoSafeList<T>::Clear()
{
    thrEnterCritical lock(m_lock);
    m_items.clear();
}


/*
* returns a copy of the list of items that is construted
* inside a safe block.
*/
template <typename T> void stoSafeList<T>::GetItemsCopy(std::vector<T> &items)
{
    thrEnterCritical lock(m_lock);
    items = m_items;
}


/*
* returns a copy of the list of items that is construted
* inside a safe block.
*/
template <typename T> void stoSafeList<T>::GetItemsRemove(std::vector<T> &items)
{
    thrEnterCritical lock(m_lock);
    items.resize(0);
    items.swap(m_items);
}


template <typename T> void stoSafeList<T>::AddItems(std::vector<T> &items)
{
    thrEnterCritical lock(m_lock);
    typename std::vector<T>::iterator it = items.begin();
    for (; it != items.end(); it++)
    {
        m_items.push_back( (*it) );
    }
}

#endif //INCLUDED_stoSafeList
