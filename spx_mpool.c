#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "spx_alloc.h"
#include "spx_types.h"
//#include "spx_errno.h"
#include "spx_mpool.h"
#include "spx_object.h"

#define mem_align_ptr(p, a)                                                   \
    (ubyte_t *) (((uintptr_t) (p) + ((uintptr_t) a - 1)) & ~((uintptr_t) a - 1))

#define SpxMemPoolingSizeDefault (8 * SpxKB)
#define SpxMemPoolBufferSizeDefault (1 * SpxMB)

struct spx_mpool *spx_mpool_new(SpxLogDelegate *log,
        size_t pooling_size,
        size_t buffsize,
        err_t *err){/*{{{*/
    size_t real_pooing_size =
        0 == pooling_size ? SpxMemPoolingSizeDefault : pooling_size;

    size_t real_buffer_size =
        0 == buffsize
        ? SpxMemPoolBufferSizeDefault
        : buffsize;

    struct spx_mpool *pool = (struct spx_mpool *)
        spx_object_new(sizeof(*pool),err);
    if(NULL == pool){
        return NULL;
    }
    pool->pooling_size = real_pooing_size;
    pool->log = log;
    pool->lg_header  = NULL;
    pool->lg_tail = NULL;
    pool->buffsize = SpxAlign(real_buffer_size,SpxAlignSize);

    struct spx_mbuff *mbuff = (struct spx_mbuff *)
        spx_object_new(sizeof(struct spx_mbuff) + pool->buffsize,err);
    if(NULL == mbuff){
        spx_object_free(pool);
        return NULL;
    }
    pool->mb_header = mbuff;
    pool->mb_curr = mbuff;

    mbuff->ptr =SpxMemIncr(mbuff , sizeof(struct spx_mbuff));
    mbuff->freesize = pool->buffsize;
    return pool;
}/*}}}*/

void *spx_mpool_malloc(struct spx_mpool *pool,
        size_t size,err_t *err){/*{{{*/
    if(0 == size){
        *err = EINVAL;
        return NULL;
    }
    if(NULL == pool){
        return spx_object_new(size,err);
    }
    size_t largesize = sizeof(struct spx_large) + size;
    size_t realsize = SpxObjectAlignSize + SpxAlign(size,SpxAlignSize);
    if(realsize >= pool->pooling_size){
        struct spx_large *large = (struct spx_large *)
            spx_alloc_alone(largesize,err);//eq new large
        if(NULL ==large){
            return NULL;
        }
        large->spx_object_is_pooling = false;
        large->spx_object_refs = 1;
        large->spx_object_size = size;
        if(NULL == pool->lg_header){
            pool->lg_header = large;
            pool->lg_tail = large;
        }else {
            large->prev = pool->lg_tail;
            pool->lg_tail->next = large;
            pool->lg_tail = large;
        }
        return SpxMemIncr(large, sizeof(struct spx_large));
    } else {
        if(pool->mb_curr->freesize < realsize){
            if(NULL != pool->mb_curr->next){
                pool->mb_curr = pool->mb_curr->next;
            } else {
                struct spx_mbuff *mbuff = (struct spx_mbuff *)
                    spx_object_new(pool->buffsize,err);
                if(NULL == mbuff){
                    SpxFree(pool);
                    return NULL;
                }
                mbuff->ptr =SpxMemIncr(mbuff , sizeof(struct spx_mbuff));
                mbuff->freesize = pool->buffsize;
                pool->mb_curr->next = mbuff;
                pool->mb_curr = mbuff;
            }
        }
        struct spx_object *o =(struct spx_object *) pool->mb_curr->ptr;
        SpxMemIncr(pool->mb_curr->ptr,realsize);
        o->spx_object_size = SpxAlign(size,SpxAlignSize);
        o->spx_object_refs = 1;
        o->spx_object_is_pooling = true;
        pool->mb_curr->freesize -= realsize;
        return SpxMemIncr(o ,SpxObjectAlignSize);
    }
    return NULL;
}/*}}}*/

void *spx_mpool_alloc(struct spx_mpool *pool,
        size_t numbs,size_t size,err_t *err){/*{{{*/
    return spx_mpool_malloc(pool,numbs * size,err);
}/*}}}*/

void *spx_mpool_alloc_alone(struct spx_mpool *pool,
        size_t size,err_t *err){/*{{{*/
    return spx_mpool_malloc(pool,size,err);
}/*}}}*/

bool_t spx_mpool_free(struct spx_mpool *pool,
        void *p){/*{{{*/
    if(NULL == pool){
        SpxObjectFree(p);
        return false;//no any operator
    }

    struct spx_object *o =(struct spx_object *) SpxMemDecr(p, SpxObjectAlignSize);
    if(o->spx_object_is_pooling){
        if(0 == (SpxAtomicDecr(&(o->spx_object_refs)))){
            size_t realsize = o->spx_object_size + SpxObjectAlignSize;//just reuse memory in the end
            memset(o,0,realsize);
            if( realsize == SpxPtrDecr(pool->mb_curr->ptr,o)){
                pool->mb_curr->ptr = (char *) o;
                pool->mb_curr->freesize += realsize;
            }
            return true;
        }
    } else {
        if(0 == (SpxAtomicDecr(&(o->spx_object_refs)))){
            struct spx_large *large = (struct spx_large *)
               SpxMemDecr(p, sizeof(struct spx_large));
            if(NULL == large->prev){
                pool->lg_header = large->next;
            }
            if(NULL == large->next){
                pool->lg_tail = large->prev;
            }
            if(NULL != large->prev){
                large->prev->next = large->next;
            }
            if(NULL != large->next){
                large->next->prev = large->prev;
            }
            free(large);
            return true;
        }
    }
    return false;
}/*}}}*/

err_t spx_mpool_clear(struct spx_mpool *pool){/*{{{*/
    if(NULL == pool){
        return EINVAL;
    }
    struct spx_large *large = NULL;
    while(NULL != (large  = pool->lg_header)){
        pool->lg_header = large->next;
        free(large);
    }
    pool->lg_header = NULL;
    pool->lg_tail = NULL;
    struct spx_mbuff *mbuff = NULL;
    struct spx_mbuff *header = pool->mb_header;
    while(NULL != (mbuff = pool->mb_header)){
        pool->mb_header = mbuff->next;
        memset(mbuff->buff,0,pool->buffsize);
        mbuff->freesize = pool->buffsize;
    }
    pool->mb_header = header;
    pool->mb_curr = header;
    pool->mb_curr->ptr = header->buff;
    return 0;
}/*}}}*/

err_t spx_mpool_destory(struct spx_mpool *pool){/*{{{*/
    if(NULL == pool){
        return EINVAL;
    }
    struct spx_large *large = NULL;
    while(NULL != (large  = pool->lg_header )){
        pool->lg_header = large->next;
        free(large);
    }
    pool->lg_header = NULL;
    pool->lg_tail = NULL;
    struct spx_mbuff *mbuff = NULL;
    while(NULL != (mbuff = pool->mb_header)){
        pool->mb_header = mbuff->next;
        spx_object_free(mbuff);
    }
    spx_object_free(pool);
    return 0;
}/*}}}*/



