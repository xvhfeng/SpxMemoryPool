/*************************************************************
 *                     _ooOoo_
 *                    o8888888o
 *                    88" . "88
 *                    (| -_- |)
 *                    O\  =  /O
 *                 ____/`---'\____
 *               .'  \\|     |//  `.
 *              /  \\|||  :  |||//  \
 *             /  _||||| -:- |||||-  \
 *             |   | \\\  -  /// |   |
 *             | \_|  ''\---/''  |   |
 *             \  .-\__  `-`  ___/-. /
 *           ___`. .'  /--.--\  `. . __
 *        ."" '<  `.___\_<|>_/___.'  >'"".
 *       | | :  `- \`.;`\ _ /`;.`/ - ` : | |
 *       \  \ `-.   \_ __\ /__ _/   .-` /  /
 *  ======`-.____`-.___\_____/___.-`____.-'======
 *                     `=---='
 *  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
 *           佛祖保佑       永无BUG
 *
 * ==========================================================================
 *
 * this software or lib may be copied only under the terms of the gnu general
 * public license v3, which may be found in the source kit.
 *
 *       Filename:  main.c
 *        Created:  2014/10/11 15时31分45秒
 *         Author:  Seapeak.Xu (seapeak.cnblog.com), xvhfeng@gmail.com
 *        Company:  Tencent Literature
 *         Remark:
 *
 ***********************************************************************/

#include <stdlib.h>
#include <stdio.h>

#include "spx_types.h"
#include "spx_defs.h"
#include "spx_alloc.h"
#include "spx_mpool.h"
#include "spx_object.h"

struct test{
    int idx;
    char *p;
};

int main(int argc,char **argv){
    err_t err= 0;

    size_t largesize = sizeof(struct spx_large);
    size_t objsize = sizeof(struct spx_object);
    size_t obj_algin_size = SpxObjectAlignSize;

    printf("%ld %ld %ld .\n",largesize,objsize,obj_algin_size);
    struct spx_mpool *pool = NULL;
    pool = spx_mpool_new(NULL,0,20 * SpxKB,4 * SpxMB,&err);
    bool_t is_first = true;
    while(true){
        struct test *t1 = (struct test *) spx_mpool_alloc_alone(pool,sizeof(struct test),&err);
        if(NULL == t1){
            printf("calloc t1 from pool is fail.\n");
            return 0;
        }

        struct test *t2 = (struct test *) spx_mpool_alloc_alone(NULL,sizeof(struct test),&err);
        if(NULL == t2){
            printf("alloc t2 from pool is fail.\n");
            return 0;
        }

        struct test *t3 = (struct test *) spx_object_new_alone(sizeof(struct test),&err);
        if(NULL == t3){
            printf("alloc t3 from pool is fail.\n");
            return 0;
        }

        size_t size = 10 * SpxKB;
        char *s1 = (char *) spx_mpool_alloc_alone(pool,size,&err);
        if(NULL == s1){
            printf("alloc s1 from pool is fail.\n");
            return 0;

        }

        char *ss = (char *) spx_mpool_alloc_alone(pool,size,&err);
        if(NULL == ss){
            printf("alloc s1 from pool is fail.\n");
            return 0;

        }

        size = 9 * SpxKB;
        char *s2 = (char *) spx_mpool_alloc_alone(pool,size,&err);
        if(NULL == s2){
            printf("alloc s2 from pool is fail.\n");
            return 0;
        }


        char *ss1 = (char *) spx_mpool_alloc_alone(pool,size,&err);
        if(NULL == ss1){
            printf("alloc s2 from pool is fail.\n");
            return 0;
        }
        SpxMemPoolFree(pool,s2);

        size = 7 * SpxKB;
        char *s3 = (char *) spx_mpool_alloc_alone(pool,size,&err);
        if(NULL == s3){
            printf("alloc s3 from pool is fail.\n");
            return 0;

        }

        char *s4 = (char *) spx_mpool_alloc_alone(pool,size,&err);
        if(NULL == s4){
            printf("alloc s4 from pool is fail.\n");
            return 0;

        }

        char *s5 = (char *) spx_mpool_alloc_alone(pool,size,&err);//add new buffer
        if(NULL == s5){
            printf("alloc s5 from pool is fail.\n");
            return 0;
        }

        SpxMemPoolFree(pool,s5);
        if(is_first){
            spx_mpool_clear(pool);
            continue;
        } else {
            break;
        }
    }

    SpxMemPoolDestory(pool);
    return 0;
}
