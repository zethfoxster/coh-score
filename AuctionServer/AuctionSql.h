#ifndef AUCTIONSQL_H
#define AUCTIONSQL_H

#include "sql/sqlconn.h"
#include "sql/sqlinclude.h"

C_DECLARATIONS_BEGIN

typedef struct ParseTable ParseTable;
typedef struct AuctionShard AuctionShard;
typedef struct LoadedAuctionHistory LoadedAuctionHistory;

void aucsql_open();
void aucsql_close();

bool aucsql_read_shards_ver(U32* ver);
bool aucsql_read_ents_ver(U32* ver);
bool aucsql_read_history_ver(U32* ver);

bool aucsql_write_shards_ver(U32 ver);
bool aucsql_write_ents_ver(U32 ver);
bool aucsql_write_history_ver(U32 ver);

bool aucsql_read_shards(ParseTable pti[], AuctionShard*** hShards);
bool aucsql_read_ents(ParseTable pti[], AuctionEnt*** hEnts);
bool aucsql_read_history(ParseTable pti[], AuctionHistoryItem*** hHistItems);

bool aucsql_insert_shard(const char** estr_shard);
bool aucsql_insert_or_update_ent(int ent_id, const char* shard, const char** estr_ent);
bool aucsql_insert_or_update_history(const char* identifier, const char** estr_history);

bool aucsql_delete_ent(int ent_id, const char* shard);

C_DECLARATIONS_END

#endif
