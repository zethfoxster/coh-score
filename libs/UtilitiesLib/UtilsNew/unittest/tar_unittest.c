/***************************************************************************
 *     Copyright (c) 2008-2008, NCSoft
 *     All Rights Reserved
 *     Confidential Property of NCSoft
 *
 * Module Description:
 *
 *
 ***************************************************************************/
#include "ncstd.h"
#include "ncunittest.h"

int tar_unittest(int argc, char **argv)
{
    char *src,*srcf = "foo.tar"; int srcn;
    char *f1,*f1t,*f1f = "temp.in.txt"; int f1n, f1nt;
    char *f2,*f2t,*f2f = "temp.out.txt"; int f2n, f2nt;
    FILE *fp;
    Tar *t;
    src = file_alloc(srcf,&srcn);
    t = tar_from_data(src,srcn);

    printf("testing tar\n");
#define TST(COND,MSG)                                   \
    if(!(COND)){                                        \
        printf("error " MSG " cond " #COND "\n");       \
        exit(1);                                        \
    }
        
    f1 = file_alloc(f1f,&f1n);
    f1t = tar_alloc_cur_file(t,&f1nt);
    TST(0==strcmp(f1f,t->hdr.fname),"not the same filename")
    TST(f1n == f1nt,"f1 lengths not equal");
    TST(0==memcmp(f1,f1t,f1n),"f1 memory not the same");
    free(f1);
    free(f1t);

    TST(tar_next(t),"couldn't move to second archive'd file");
    f2 = file_alloc(f2f,&f2n);
    f2t = tar_cur_file(t,&f2nt);
    TST(0==strcmp(f2f,t->hdr.fname),"not the same filename")
    TST(f2n == f2nt,"f2 lengths not equal");
    TST(0==memcmp(f2,f2t,f2n),"f2 memory not the same");
    free(f2);
    
    TST(!tar_next(t),"able to move to third archive, should be EOF");    
    Tar_Destroy(t);
    printf("done\n");
}
#endif

