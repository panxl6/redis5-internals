//
// Created by panxl on 2020/8/8.
//

#include <stdio.h>
#include "../sds.h"
#include "../dict.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "../adlist.h"

//#include "../server.h"

void dictSdsDestructor(void *privdata, void *val)
{
    DICT_NOTUSED(privdata);

    sdsfree(val);
}

uint64_t dictSdsHash(const void *key)
{
    return dictGenHashFunction((unsigned char *)key, sdslen((char *)key));
}

int dictSdsKeyCompare(void *privdata, const void *key1,
                      const void *key2)
{
    int l1, l2;
    DICT_NOTUSED(privdata);

    l1 = sdslen((sds)key1);
    l2 = sdslen((sds)key2);
    if (l1 != l2)
        return 0;
    return memcmp(key1, key2, l1) == 0;
}

/* _serverAssert is needed by dict */
void _serverAssert(const char *estr, const char *file, int line)
{
    fprintf(stderr, "=== ASSERTION FAILED ===");
    fprintf(stderr, "==> %s:%d '%s' is not true", file, line, estr);
    *((char *)-1) = 'x';
}

dictType clusterNodesDictType = {
    dictSdsHash,       /* hash function */
    NULL,              /* key dup */
    NULL,              /* val dup */
    dictSdsKeyCompare, /* key compare */
    dictSdsDestructor, /* key destructor */
    NULL               /* val destructor */
};

//-----------------LRU实现 start--------------------
typedef struct
{
    int size;     // 缓存的大小
    int capacity; // 已经使用的缓存大小
    dict *cache;
    list *queue;
} LRU;

LRU *createLRU(int size)
{
    LRU *lru = (LRU *)malloc(sizeof(LRU));
    lru->size = size;
    lru->capacity = 0;
    lru->cache = dictCreate(&clusterNodesDictType, NULL);
    lru->queue = listCreate();
    
    return lru;
}

// 从lru缓存中查询数据
listNode* get(LRU* lru, sds key)
{
    dictEntry *entry = dictFind(lru->cache, key);
    if (entry == NULL) {
        return NULL;
    }
    return dictGetVal(entry);
}

// 添加数据
void put(LRU* lru, int data)
{
    sds key = sdsfromlonglong(data);
    listNode* ret = get(lru, key);
    // 如果不在缓存中，就添加
    if (ret == NULL) {
        // 如果缓存未满，直接添加
        if (lru->capacity < lru->size) {
            lru->capacity += 1;
            listAddNodeHead(lru->queue, data);
            dictAdd(lru->cache, key, lru->queue->head);
        } else {
            // 如果缓存已满，执行淘汰
            listAddNodeHead(lru->queue, data);
            dictAdd(lru->cache, key, lru->queue->head);
            listDelNode(lru->queue, lru->queue->tail);
        }
    } else {
        // 如果已经存在，刷新缓存
        // 删掉
        listDelNode(lru->queue, ret);
        // 刷新该key成为热点
        listAddNodeHead(lru->queue, data);
    }
}

//-----------------LRU实现 end----------------------

int main(void)
{
    LRU* lru = createLRU(9);
    printf("大小: %d\n", lru->size);
    int caseNum = 10;
    int testCase[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    for (int i = 0; i < caseNum; i++)
    {
        put(lru, testCase[i]);
    }

    listNode* temp = lru->queue->head;
    for (; temp != NULL;) {
        printf("当前节点的值:%d\n", temp->value);
        temp = temp->next;
    }
    
    return 0;
}
