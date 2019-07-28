#ifndef MINING_ACCUMULATOR_H
#define MINING_ACCUMULATOR_H

#ifndef CLIENT

#define MINEACC_NAME_LEN		255
#define MINEACC_FIELD_LEN		255

typedef struct NetLink NetLink;
typedef struct Packet Packet;

int MiningAccumulatorCount(); 

void MiningAccumulatorAdd(const char *name, const char *field, int val);
// does what an accumulator does best, probably the only function you need
// this adds the value to the global mining accumulator list
//
// try to use a format like ("salvage.[name]","in.drop",1)
// see mininghelper.h for some useful macros


// statserver only
int MiningAccumulatorUnpackAndMerge(char *container_data, U32 dbid, char **name);
bool MiningAccumulatorWeekendUpdate(); // returns true on a new week
char* MiningAccumulatorPackageNext(const char **name, int *dbid);
void MiningAccumulatorSetDBID(const char *name, U32 dbid); // used when an accumulator has been newly persisted
void MiningAccumulatorReceive(Packet *pak);


// statserver clients only (mapserver and auctionserver)
void MiningAccumulatorSendAndClear(NetLink *link);
void MiningAccumulatorSendImmediate(const char *name, const char *field, int val, NetLink *link);


// mapserver only
char* MiningAccumulatorTemplate();
char* MiningAccumulatorSchema();


#else // ifdef CLIENT
#define MiningAccumulatorAdd(name,field,val)
#endif

#endif // MINING_ACCUMULATOR_H
