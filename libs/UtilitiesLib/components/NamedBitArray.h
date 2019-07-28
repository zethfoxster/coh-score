#ifndef NAMEDBITARRAY_H
#define NAMEDBITARRAY_H

typedef struct NamedBitArrayDefImp *NamedBitArrayDef;
NamedBitArrayDef createNamedBitArrayDef(char* defName);
void destroyAllBitArrayDef();

NamedBitArrayDef nbdGetNamedBitArrayDef(char* defName);
NamedBitArrayDef nbdReadDefFromFile(char* filename);

int nbdAddBitName(NamedBitArrayDef def, char* name);
int nbdFindBitName(NamedBitArrayDef def, char* name);




typedef struct NamedBitArrayImp *NamedBitArray;
NamedBitArray createNamedBitArray(NamedBitArrayDef def);
NamedBitArray createNamedBitArrayByDefName(char* filename);
void destroyNamedBitArray(NamedBitArray array);

void nbAttachDef(NamedBitArray array, NamedBitArrayDef def);

int nbSetBitValue(NamedBitArray array, unsigned int bitnum, int value);
int nbSetBitValueByName(NamedBitArray array, char* name, int value);

int	nbSetBit(NamedBitArray array, unsigned int bitnum);
int	nbSetBitByName(NamedBitArray array, char* name);

int nbClearBit(NamedBitArray array, unsigned int bitnum);
int nbClearBitByName(NamedBitArray array, char* name);

int nbClearBits(NamedBitArray target, NamedBitArray mask);

void nbSetAllBits(NamedBitArray array);
void nbClearAllBits(NamedBitArray array);
void nbNotAllBits(NamedBitArray array);

// Bit testing.
int nbIsSet(NamedBitArray array, unsigned int bitnum);
int nbIsSetByName(NamedBitArray array, char* name);

int nbGetMaxBits(NamedBitArray array);
int nbCountSetBits(NamedBitArray array);
int nbOrArray(NamedBitArray array1, NamedBitArray array2, NamedBitArray output);
int nbAndArray(NamedBitArray array1, NamedBitArray array2, NamedBitArray output);
int nbCopyArray(NamedBitArray input, NamedBitArray output);
int nbBitsAreEqual(NamedBitArray array1, NamedBitArray array2);

const char* nbGetBitName(NamedBitArray array, int bit);

#endif