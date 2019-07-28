#ifndef SEQTYPE_H
#define SEQTYPE_H

typedef struct SeqType SeqType;

void seqTypeLoadFiles(void);
SeqType *seqTypeFind(const char *type_name);


#endif