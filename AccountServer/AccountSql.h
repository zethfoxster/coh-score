#ifndef ACCOUNTSQL_H
#define ACCOUNTSQL_H

#include "sql/sqlconn.h"
#include "sql/sqlinclude.h"
#include "AccountTypes.h"

#define LOYALTY_SQL_BYTES 16 // this fits 128 bits
#define ACCOUNT_NAME_MAX 14

C_DECLARATIONS_BEGIN

typedef struct Account Account;
typedef struct MicroTransaction MicroTransaction;
typedef struct GameTransaction GameTransaction;
typedef struct MultiGameTransaction MultiGameTransaction;

typedef struct asql_account {
	U32 auth_id;
	char name[ACCOUNT_NAME_MAX+1];
	U8 loyalty_bits[LOYALTY_SQL_BYTES];
	U16 last_loyalty_point_count;
	U16 loyalty_points_spent;
	SQL_TIMESTAMP_STRUCT last_email_date;
	U16 last_num_emails_sent;
	SQL_TIMESTAMP_STRUCT free_xfer_date;
} asql_account;

/**
* @note Modifying this structure requires updating @ref accountInventory_LoadFinished
*       to log failures.
*/
typedef struct asql_inventory {
	SkuId sku_id;
	int granted_total;
	int claimed_total;
	int saved_total;
	SQL_TIMESTAMP_STRUCT expires;
} asql_inventory;

/**
* @note Modifying this structure requires updating @ref Transaction_MicroFinished
*       to log failures.
*/
typedef struct asql_micro_transaction {
	OrderId order_id;
	U32 auth_id;
	SkuId sku_id;
	SQL_TIMESTAMP_STRUCT transaction_date;
	long quantity;
	long points;
} asql_micro_transaction;

/**
* @note Modifying this structure requires updating @ref Transaction_GameFinished
*       to log failures.
*/
typedef struct asql_game_transaction {
	OrderId order_id;
	U32 auth_id;
	SkuId sku_id;
	SQL_TIMESTAMP_STRUCT transaction_date;
	U8 shard_id;
	U32 ent_id;
	long granted;
	long claimed;
	U8 csr_did_it;
} asql_game_transaction;

typedef struct asql_flexible_inventory {
	ssize_t count;
	int list_size;
	asql_inventory single; // used if the task returns only a single result
	asql_inventory * list; // used if the task returns multiple results
} asql_flexible_inventory;

void asql_open();
void asql_close();

void asql_tick();
void asql_wait_for_tick();
int asql_in_flight();

void asql_sync_products();
void asql_search_for_account_async(const char *auth_name_or_id);
void asql_find_or_create_account_async(Account * acc);
void asql_update_account_async(Account * acc);
void asql_add_micro_transaction_async(MicroTransaction * transaction);
void asql_add_game_transaction_async(GameTransaction * transaction);
void asql_revert_game_transaction_async(Account * acc, OrderId order_id);
void asql_add_multi_game_transaction_async(MultiGameTransaction * transaction);
void asql_save_game_transaction_async(Account * acc, OrderId order_id);
void asql_read_unsaved_game_transactions_async(Account * acc, U8 shard_id, U32 ent_id);

C_DECLARATIONS_END

#endif
