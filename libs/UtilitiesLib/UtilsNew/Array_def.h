/***************************************************************************
 *     Copyright (c) 2008-2008, NCSoft
 *     All Rights Reserved
 *     Confidential Property of NCSoft
 *
 * Module Description: an X-macro file, not really a header
 *
 *
 ***************************************************************************/
#ifndef TYPE_T
#error "TYPE_T not defined"
#endif

#ifndef TYPE_FUNC_PREFIX
#error "TYPE_FUNC_PREFIX not defined"
#endif

// do a cut-n-paste and search replace:
// #define foo_create(capacity) foo_create_dbg(capacity  DBG_PARMS_INIT)
// #define foo_destroy(ha, fp) foo_destroy_dbg(ha, fp DBG_PARMS_INIT)
// #define foo_size(ha) foo_size_dbg(ha  DBG_PARMS_INIT)
// #define foo_setsize(ha,size) foo_setsize_dbg(ha, size  DBG_PARMS_INIT)
// #define foo_push(ha) foo_push_dbg(ha  DBG_PARMS_INIT)
// #define foo_pushn(ha,n) foo_pushn_dbg(ha,(n)  DBG_PARMS_INIT)
// #define foo_pop(ha) foo_pop_dbg(ha  DBG_PARMS_INIT)
// #define foo_top(ha) foo_top_dbg(ha  DBG_PARMS_INIT)
// #define foo_cp(hdest, hsrc, n) foo_cp_dbg(hdest, hsrc, n  DBG_PARMS_INIT)
// #define foo_insert(hdest, src, i, n) foo_insert_dbg(hdest, src, i, n  DBG_PARMS_INIT)
// #define foo_append(hdest, src, n) foo_append_dbg(hdest, src, n  DBG_PARMS_INIT)
// #define foo_happend(hdest, hsrc) foo_append_dbg(hdest, *hsrc, foo_size(hsrc)  DBG_PARMS_INIT)
// #define foo_rm(ha, offset, n) foo_rm_dbg(ha, offset, n  DBG_PARMS_INIT)
// #define foo_find(ha, b, cmp_fp, ctxt) foo_find_dbg(ha, b, cmp_fp, ctxt DBG_PARMS_INIT)
// #define foo_foreach_ctxt(ha, cmp_fp, ctxt) foo_foreach_ctxt_dbg(ha, cmp_fp, ctxt DBG_PARMS_INIT)
// #define foo_foreach(ha, cmp_fp) foo_foreach_dbg(ha, cmp_fp DBG_PARMS_INIT)
// #define foo_pushfront(ha) foo_pushfront_dbg(ha  DBG_PARMS_INIT) // expensive!!!



typedef void ADECL(destroyelt_fp)(TYPE_T *helt);
typedef int ADECL(cmp_fp)(TYPE_T *hlhs,TYPE_T *hrhs, void *ctxt);
typedef void ADECL(foreach_ctxt_fp)(TYPE_T *ht, void *ctxt);
typedef void ADECL(foreach_fp)(TYPE_T *ht);

TYPE_T* ADECL(init_dbg)(void *mem, size_t mem_size, U32 flags DBG_PARMS);
TYPE_T* ADECL(create_dbg)(int capacity DBG_PARMS);
void ADECL(destroy_dbg)(TYPE_T** ha, ADECL(destroyelt_fp) *fp DBG_PARMS);
int ADECL(size_dbg)(TYPE_T const * const * ha DBG_PARMS);
void ADECL(setsize_dbg)(TYPE_T** ha, int size DBG_PARMS);
TYPE_T* ADECL(push_dbg)(TYPE_T **ha DBG_PARMS);
TYPE_T* ADECL(pushn_dbg)(TYPE_T **ha, int n DBG_PARMS);
TYPE_T* ADECL(pushfront_dbg)(TYPE_T **ha DBG_PARMS);
int ADECL(push_by_cp_dbg)(TYPE_T **ha, TYPE_T b DBG_PARMS);
TYPE_T* ADECL(pop_dbg)(TYPE_T **ha DBG_PARMS);
TYPE_T* ADECL(top_dbg)(TYPE_T **ha DBG_PARMS);
void ADECL(cp_dbg)(TYPE_T **hdest,TYPE_T const * const *hsrc,int n DBG_PARMS);
void ADECL(cp_raw_dbg)(TYPE_T **hdest,TYPE_T const *s,int n DBG_PARMS); // second arg may not be an Array
void ADECL(insert_dbg)(TYPE_T **hdest, TYPE_T *src, int i, int n DBG_PARMS);
void ADECL(append_dbg)(TYPE_T **hdest, TYPE_T *src, int n DBG_PARMS);
void ADECL(rm_dbg)(TYPE_T **ha, int offset, int n DBG_PARMS);
int ADECL(find_dbg)(TYPE_T **ha, TYPE_T *b, ADECL(cmp_fp) *cmp, void *ctxt DBG_PARMS);
void ADECL(foreach_ctxt_dbg)(TYPE_T **ha, ADECL(foreach_ctxt_fp) *fp, void *ctxt DBG_PARMS);
void ADECL(foreach_dbg)(TYPE_T **ha, ADECL(foreach_fp) *fp DBG_PARMS);
