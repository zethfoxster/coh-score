#include "listlist.hpp"
#include "UnitTest++.h"

class TestList {
public:
	TestList(int val) : val(val) {}

	int val;

	LINK(TestList, test);
};

typedef ListList<TestList*, char> CharListTestList;

#define N_LINKS 50

void listlist_setupList(CLinkedList<TestList*> &list, TestList *links[N_LINKS]){
	for(int i = 0; i < N_LINKS; i++){
		links[i] = new TestList(i);
		links[i]->test.linkHead(list);
	}
}

void listlist_teardownList(TestList *links[N_LINKS]){
	for(int i = 0; i < N_LINKS; i++){
		links[i]->test.unlink();
		delete(links[i]);
	}
}

TEST(ListListAllocAndDealloc){
	ListList<TestList*, int> listList;

	for(int i = 0; i < 1000; i++)
		listList.add(i);

	for(int i = 0; i < 999; i++)
		listList.remove(i);
}

TEST(ListListGet) { //and add list
	TestList *linksA[N_LINKS], *linksB[N_LINKS];
	CharListTestList listList;

	CLinkedList<TestList*> &listA = listList.add('a');
	listlist_setupList(listA, linksA);

	CLinkedList<TestList*> &listB = listList.add('b');
	listlist_setupList(listB, linksB);

	CHECK(&listA == listList.get('a'));
	CHECK(&listB == listList.get('b'));

	listlist_teardownList(linksA);
	listlist_teardownList(linksB);
}

TEST(ListListRemove) { //and add list
	TestList *linksA[N_LINKS];
	CharListTestList listList;
	CLinkedList<TestList*> &listA = listList.add('a');
	listlist_setupList(listA, linksA);
	listlist_teardownList(linksA);
	listList.remove('a');
}

TEST(ListListIterate) { 
	TestList *linksA[N_LINKS], *linksB[N_LINKS];
	ListList<TestList*, char> listList;

	CLinkedList<TestList*> &listA = listList.add('a');
	listlist_setupList(listA, linksA);

	CLinkedList<TestList*> &listB = listList.add('b');
	listlist_setupList(listB, linksB);

	unsigned int loops = 0;
	FOREACH_LINK(CharListTestList::TListLink, it, listList.array(), link)
		loops++;
	CHECK(loops == listList.count());

	listlist_teardownList(linksA);
	listlist_teardownList(linksB);
}
