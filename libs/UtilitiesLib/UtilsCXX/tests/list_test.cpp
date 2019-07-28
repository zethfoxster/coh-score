#include "list.hpp"
#include "UnitTest++.h"

class TestList {
public:
	TestList(int val) : val(val) {}

	int val;

	LINK(TestList, test);

	static int cmp(const TestList &a, const TestList &b) {
		return a.val - b.val;
	}

	static int cmp(const CLink<TestList*> &a, const CLink<TestList*> &b) {
		return testCompare(a, b, cmp);
	}
};

#define N_LINKS 50

void setupList(CLinkedList<TestList*> &list, TestList *links[N_LINKS]) {
	for(int i = 0; i < N_LINKS; i++){
		links[i] = new TestList(i);
		links[i]->test.linkTail(list);
	}
}

void teardownList(TestList *links[N_LINKS]) {
	for(int i = 0; i < N_LINKS; i++){
		links[i]->test.unlink();
		delete(links[i]);
	}
}

TEST(ListAllocAndDealloc) {
	TestList t(1);
}

TEST(ListLink) {
	CLinkedList<TestList*> list;
	TestList link(0);

	CHECK(list.empty());
	CHECK(!link.test.linked());
	link.test.linkHead(list);

	CHECK(!list.empty());
	CHECK(link.test.linked());
	link.test.unlink();

	CHECK(list.empty());
	CHECK(!link.test.linked());
	link.test.linkTail(list);

	CHECK(!list.empty());
	CHECK(link.test.linked());
	link.test.unlink();
}

TEST(ListOrder) {
	TestList *links[N_LINKS];
	CLinkedList<TestList*> list; 
	setupList(list, links);

	CHECK(list.isFirst(*list.first()));
	CHECK(list.isLast(*list.last()));

	int i = 0;

	FOREACH_LINK(TestList, curr, list, test) {
		CHECK(curr);
		CHECK(curr->test.linked());
		CHECK(curr->val == i);
		i++;
	}

	CHECK(i == N_LINKS);

	FOREACH_LINK_REVERSE(TestList, curr, list, test) {
		i--;
		CHECK(curr);
		CHECK(curr->test.linked());
		CHECK(curr->val == i);
	}

	CHECK(i == 0);

	teardownList(links);
}

TEST(TestLinkHead) {
	TestList *links[N_LINKS];
	CLinkedList<TestList*> list;
	setupList(list, links);

	TestList link(-1);
	link.test.linkHead(list);
	CHECK(list.isFirst(link.test));

	int i = -1;

	FOREACH_LINK(TestList, curr, list, test) {
		CHECK(curr);
		CHECK(curr->test.linked());
		CHECK(curr->val == i);
		i++;
	}

	CHECK(i == N_LINKS);

	teardownList(links);

	link.test.unlink();
}

TEST(FooLinkSorted) {
	TestList A(1), B(2), C(3);
	CLinkedList<TestList*> list;

	B.test.linkHead(list);
	
	list.addSortedNearHead(A.test, A.cmp);
	CHECK(list.isFirst(A.test));
	CHECK(list.isLast(B.test));

	list.addSortedNearTail(C.test, C.cmp);
	CHECK(list.isFirst(A.test));
	CHECK(list.isLast(C.test));

	A.test.unlink();
	CHECK(list.isFirst(B.test));
	CHECK(list.isLast(C.test));

	list.addSortedNearTail(A.test, A.cmp);
	CHECK(list.isFirst(A.test));
	CHECK(list.isLast(C.test));

	C.test.unlink();
	CHECK(list.isFirst(A.test));
	CHECK(list.isLast(B.test));

	list.addSortedNearHead(C.test, C.cmp);
	CHECK(list.isFirst(A.test));
	CHECK(list.isLast(C.test));

	B.test.unlink();
	CHECK(list.isFirst(A.test));
	CHECK(list.isLast(C.test));

	A.test.unlink();
	CHECK(list.isFirst(C.test));
	CHECK(list.isLast(C.test));

	C.test.unlink();
	CHECK(list.empty());
}

TEST(FooLinkSortedDuplicate) {
	TestList *links[N_LINKS];
	CLinkedList<TestList*> list;
	setupList(list, links);

	TestList link(N_LINKS/2);

	list.addSortedNearTail(link.test, link.cmp);
	CHECK_CLOSE(link.val, TestList::testEntry(link.test.prev())->val, 1);
	CHECK_CLOSE(link.val, TestList::testEntry(link.test.next())->val, 1);
	link.test.unlink();

	list.addSortedNearHead(link.test, link.cmp);
	CHECK_CLOSE(link.val, TestList::testEntry(link.test.prev())->val, 1);
	CHECK_CLOSE(link.val, TestList::testEntry(link.test.next())->val, 1);

	teardownList(links);
}

TestList *findLink(CLinkedList<TestList*> &list, int value) {
	FOREACH_LINK(TestList, cur, list, test)
		if (cur->val == value)
			return cur;
	return NULL;
}

TEST(ListChurn) {
	TestList *links[N_LINKS];
	CLinkedList<TestList*> list;
	CLinkedList<TestList*> newList;
	setupList(list, links);

	TestList *middle = findLink(list, N_LINKS/2);
	middle->test.unlink();
	middle->test.linkHead(newList);

	FOREACH_LINK_SAFE(TestList, cur, next, list, test) {
		cur->test.unlink();
		if(cur->val % 2)
			cur->test.linkAfter(&middle->test);
		else
			cur->test.linkBefore(&middle->test);
	}

	bool seenMiddle = false;

	FOREACH_LINK(TestList, cur, newList, test) {
		if (cur == middle) {
			seenMiddle = true;
			continue;
		}

		if (seenMiddle)
			CHECK((cur->val%2));
		else
			CHECK(!(cur->val%2));
	}

	CHECK(seenMiddle);
	
	teardownList(links);
}

TEST(FooSort) {
	TestList *links[N_LINKS];
	CLinkedList<TestList*> list;
	setupList(list, links);
	
	srand(0);
	FOREACH_LINK(TestList, cur, list, test) {
		TODO(); // sort needs to be refactored
#if 0
		cur->val = rand();
#endif
	}

	list.sort(TestList::cmp);

	FOREACH_LINK_SAFE(TestList, cur, next, list, test) {
		if (list.isSentinel(next->test))
			break;
		CHECK(cur->val < next->val);
	}

	teardownList(links);
}

