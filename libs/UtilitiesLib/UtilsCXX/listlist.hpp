#include "list.hpp"

#ifndef _LISTLIST_HPP
#define _LISTLIST_HPP

template<class LIST_TYPE, class KEY_TYPE = unsigned short>
class ListLink {
	const KEY_TYPE m_key;
	CLinkedList<LIST_TYPE> m_value;

public:
	ListLink(const KEY_TYPE &key) : m_key(key) {}

	const KEY_TYPE key() { return m_key; }
	const CLinkedList<LIST_TYPE> &value() const { return &m_value; }
	CLinkedList<LIST_TYPE> &value() { return m_value; }

	LINK(ListLink, link);
};

template<class LIST_TYPE, class KEY_TYPE = unsigned short>
class ListList {
public:
	typedef ListLink<LIST_TYPE, KEY_TYPE> TListLink;

	ListList() : m_count(0) {}

	~ListList() {
		FOREACH_LINK_SAFE(TListLink, cur, next, m_lists, link)
			delete cur;

		m_count = 0;
	}

	TListLink *at(KEY_TYPE id) {
		FOREACH_LINK(TListLink, cur, m_lists, link)
			if (cur->key() == id)
				return cur;
		return NULL;
	}

	CLinkedList<LIST_TYPE> &operator[](KEY_TYPE id) {
		TListLink *found = at(id);
		assert(found);
		return found->value();
	}

	CLinkedList<LIST_TYPE> *get(KEY_TYPE id) {
		TListLink *found = at(id);
		return found ? &found->value() : NULL;
	}

	CLinkedList<LIST_TYPE> &add(KEY_TYPE id) {
		TListLink *found = at(id);

		if (devassertmsg(!found, "Attempt to add a list that already exists.")) {
			found = new TListLink(id);
			found->link.linkHead(m_lists);
			m_count++;
		}

		return found->value();
	}

	bool remove(KEY_TYPE id) {
		TListLink *found = at(id);
		if (!found)
			return false;

		delete found;
		m_count--;

		devassertmsg((m_count==0) == (m_lists.empty()), "ListList corrupted.");
		return true;
	}

	unsigned int count() const { return m_count; }

	CLinkedList<TListLink*> &array() {
		return m_lists;
	}

	const CLinkedList<TListLink*> &array() const {
		return m_lists;
	}

private:
	CLinkedList<TListLink*> m_lists;
	unsigned int m_count;
};

#endif