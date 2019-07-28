//
// Linked lists
// Copyright NCsoft 2005
// 

//#include "Base.h"

#include "SuperAssert.h"

#ifndef _LIST_HPP
#define _LIST_HPP

template<class TYPE> 
class CLinkedList;

//-------------------------------------------------------------------------------------------------
// SORT_COMPARE
//-------------------------------------------------------------------------------------------------

template<typename T> 
struct SSortCompare
{
	typedef int (*Type)(T, T, void*);
};

#define SORT_COMPARE(T) SSortCompare< T >::Type

void MergeSortList(SORT_COMPARE(void*) compare, void** ppList, U32 linkOffset, void* context);

//-------------------------------------------------------------------------------------------------
// LINK macro
//
// Placing 'LINK(TYPE, foo);' inside the class definition of TYPE will declare the 
// following methods:
//
//	static TYPE* fooEntry(const CLink<TYPE>* entry);
//
// The head of the list is a variable of type CLinkedList<TYPE*>
//-------------------------------------------------------------------------------------------------

#define LINK(TYPE, NAME) \
	CLink<TYPE*> NAME; \
	static TYPE* NAME##Entry(CLink<TYPE*> *entry) { return (TYPE*)(((uintptr_t)entry) - offsetof(TYPE, NAME)); } \
	static const TYPE* NAME##Entry(const CLink<TYPE*> *entry) { return (const TYPE*)(((uintptr_t)entry) - offsetof(TYPE, NAME)); } \
	static int NAME##Compare(const CLink<TYPE*> &a, const CLink<TYPE*> &b, int (*cmp)(const TYPE&, const TYPE&)) { return cmp(*NAME##Entry(&a), *NAME##Entry(&b)); }

#define FOREACH_LINK(TYPE, ITER, LIST, NAME) \
	for (TYPE* ITER = ITER->NAME##Entry(LIST.first()); !LIST.isSentinel(ITER->NAME); ITER = ITER->NAME##Entry(ITER->NAME.next()))

#define FOREACH_LINK_SAFE(TYPE, ITER, NEXT, LIST, NAME) \
	for (TYPE* ITER = ITER->NAME##Entry(LIST.first()), *NEXT = ITER->NAME##Entry(ITER->NAME.next()); !LIST.isSentinel(ITER->NAME); ITER = NEXT, NEXT = ITER->NAME##Entry(ITER->NAME.next()))

#define FOREACH_LINK_REVERSE(TYPE, ITER, LIST, NAME) \
	for (TYPE* ITER = ITER->NAME##Entry(LIST.last()); !LIST.isSentinel(ITER->NAME); ITER = ITER->NAME##Entry(ITER->NAME.prev()))

template <class TYPE>
class CLink
{
	friend class CLinkedList<TYPE>;

	CLink<TYPE>* m_next;
	CLink<TYPE>* m_prev;

public:
	CLink() : m_prev(NULL), m_next(NULL)
	{
	}

	~CLink()
	{
		if (linked())
			unlink();
	}

	CLink<TYPE>* prev()
	{
		return m_prev;
	}

	const CLink<TYPE>* prev() const
	{
		return m_prev;
	}

	CLink<TYPE>* next()
	{
		return m_next;
	}

	const CLink<TYPE>* next() const
	{
		return m_next;
	}

	bool linked() const
	{
		return m_prev && m_next;
	}

	void unlink()
	{
		assert(linked());

		m_next->m_prev = m_prev;
		m_prev->m_next = m_next;

		m_prev = NULL;
		m_next = NULL;
	}

	void linkAfter(CLink<TYPE> *element)
	{
		assert(!linked());

		m_next = element->m_next;
		m_next->m_prev = this;
		element->m_next = this;
		m_prev = element;
	}

	void linkBefore(CLink<TYPE> *element)
	{
		assert(!linked());

		m_prev = element->m_prev;
		m_prev->m_next = this;
		element->m_prev = this;
		m_next = element;
	}

	void linkHead(CLinkedList<TYPE> &list);
	void linkTail(CLinkedList<TYPE> &list);
};

//-------------------------------------------------------------------------------------------------
// CLinkedList
//-------------------------------------------------------------------------------------------------

template<class TYPE> 
class CLinkedList
{
	CLink<TYPE> m_listhead;

public:
	CLinkedList()
	{
		m_listhead.m_next = listhead();
		m_listhead.m_prev = listhead();
	}

	~CLinkedList()
	{
		// ideally, we would delink all the links.
		devassertmsg(empty(), "Destroying non-empty list.  Child links will remain attached.");
	}

	bool empty() const
	{
		return m_listhead.m_next == listhead();
	}

	CLink<TYPE> *listhead()
	{
		return &m_listhead;
	}

	const CLink<TYPE> *listhead() const
	{
		return &m_listhead;
	}

	CLink<TYPE> *first()
	{
		return m_listhead.m_next;
	}

	const CLink<TYPE> *first() const
	{
		return m_listhead.m_next;
	}

	CLink<TYPE> *last()
	{
		return m_listhead.m_prev;
	}

	const CLink<TYPE> *last() const
	{
		return m_listhead.m_prev;
	}

	bool isSentinel(CLink<TYPE> &element) const
	{
		return &element == listhead();
	}

	bool isFirst(CLink<TYPE> &element) const
	{
		return element.prev() == listhead();
	}

	bool isLast(CLink<TYPE> &element) const
	{
		return element.next() == listhead();
	}

	void addSortedNearHead(CLink<TYPE> &element, int (*cmp)(const CLink<TYPE>&, const CLink<TYPE>&))
	{
		assert(!element.linked());

		CLink<TYPE> *cur = first();

		while (cur != listhead() && cmp(element, *cur) > 0)
			cur = cur->next();

		element.linkBefore(cur);
	}

	void addSortedNearTail(CLink<TYPE> &element, int (*cmp)(const CLink<TYPE>&, const CLink<TYPE>&))
	{
		assert(!element.linked());

		CLink<TYPE> *cur = last();

		while (cur != listhead() && cmp(element, *cur) < 0)
			cur = cur->prev();

		element.linkAfter(cur);
	}

	void sort(int (*cmp)(const CLink<TYPE>&, const CLink<TYPE>&));
};

template <class TYPE>
void CLink<TYPE>::linkHead(CLinkedList<TYPE>& list)
{
	linkAfter(list.listhead());
}

template <class TYPE>
void CLink<TYPE>::linkTail(CLinkedList<TYPE>& list)
{
	linkBefore(list.listhead());
}

template <class TYPE>
void CLinkedList<TYPE>::sort(int (*cmp)(const CLink<TYPE>&, const CLink<TYPE>&))
{
	TODO(); // needs to be refactored
#if 0
	for (U32 cNode = 1;; cNode *= 2)
	{
		// A starts at head
		void* pNodeA = *ppList;
		BOOL singleMerge = TRUE;

		for(;;)
		{
			// B starts cNode elements ahead of A
			void* pNodeB = pNodeA;
			U32 cNodeA = cNode;

			while(pNodeB && cNodeA)
			{
				pNodeB = ((SLink*) ((char*) pNodeB + linkOffset))->next;
				cNodeA--;
			}


			// Merge A and B
			U32 cNodeB = cNodeA = cNode;

			while(pNodeB && cNodeA && cNodeB)
			{
				if(compare(pNodeA, pNodeB, context) <= 0)
				{
					// Node is in the right place, just need to increment A
					pNodeA = ((SLink*) ((char*) pNodeA + linkOffset))->next;
					cNodeA--;
				}
				else
				{
					// Move node from front of B to before of A
					void* pNode = pNodeB;
					SLink* pLink = (SLink*) ((char*) pNodeB + linkOffset);


					// Unlink from B
					pNodeB = *pLink->prev = pLink->next;
					cNodeB--;

					if(pNodeB)
					{
						SLink* pLinkB = (SLink*) ((char*) pNodeB + linkOffset);
						pLinkB->prev = pLink->prev;
					}


					// Link to before A
					SLink* pLinkA = (SLink*) ((char*) pNodeA + linkOffset);

					pLink->prev = pLinkA->prev;
					pLink->next = pNodeA;

					*pLink->prev = pNode;
					pLinkA->prev = &pLink->next;
				}
			}


			// Remaining elements of A are in the right order
			// Remaining elements of B are in the right order too, just need to skip over them
			while(pNodeB && cNodeB)
			{
				pNodeB = ((SLink*) ((char*) pNodeB + linkOffset))->next;
				cNodeB--;
			}

			if(!pNodeB)
				break;


			// A starts where B left off
			pNodeA = pNodeB;

			// We are doing more than one merge this pass, will need to do at least another pass.
			singleMerge = FALSE;
		}


		// If a single merge covered entire list, we are done
		if(singleMerge)
			break;
	}
#endif
}

#endif
