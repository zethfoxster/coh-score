#ifndef _CONTAINER_TPLT_H
#define _CONTAINER_TPLT_H

#include "gametypes.h"
#include "container_sql.h"

void tpltRegisterForeignKeyConstraint(char *table, char *key, char *foreign_table);
void tpltSetAllForeignKeyConstraintsAsync(void);

TableInfo *tpltFindTable(ContainerTemplate *tplt,char *name);
void hashColumnNames(TableInfo *table);
void hashAllNames(ContainerTemplate *tplt, const char *fname);
ContainerTemplate *tpltLoad(int dblist_id,char *table_name,char *fname);
void tpltAddMembership(int memberlist_id,ContainerTemplate *group);
void tpltDropUnreferencedTables(void);
void tpltUpdateSqlcolumns(ContainerTemplate *tplt);
AttributeList *tpltLoadAttributes(char *fname, char *alt_fname, char *tablename);
void tpltDeleteAll(ContainerTemplate *tplt, char *null_field, bool renumber);

#endif
