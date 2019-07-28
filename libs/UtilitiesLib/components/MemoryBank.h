/* File MemoryBank.h
 *	Defines memory management units called "MemoryBank" and "MemoryLoan".
 *
 *	A memory bank is capable if loaning out memory of arbitrary size.  It also
 *	records where/who allocated the memory.  The main purpose of a memory bank is
 *	to keep track of a small number of large memory so that it is possible to
 *	reuse them frequently without having to call malloc/free.
 *
 *
 *
 */

#ifndef MEMORYBANK_H
#define MEMORYBANK_H

typedef struct MemoryLoanImp *MemoryLoan;
typedef struct MemoryBankImp *MemoryBank; 

MemoryBank createMemoryBank();
void initMemoryBank(MemoryBank bank, unsigned int expectedLoans);
void destroyMemoryBank(MemoryBank bank);

#define mbAlloc(bank, allocSize) mbAlloc_dbg(bank, allocSize, __FILE__, __LINE__)
MemoryLoan mbAlloc_dbg(MemoryBank bank, unsigned int allocSize, const char* filename, int lineno);
int mbRealloc(MemoryLoan loan, unsigned int allocSize);
void mbFree(MemoryLoan loan);

void* mbGetMemory(MemoryLoan loan);

void testMemoryBank();
#endif