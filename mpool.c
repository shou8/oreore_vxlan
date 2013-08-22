#include <stdlib.h>

#include "mpool.h"



#define MPOOL_POOL_SIZE 65536
#define MPOOL_ALIGN_SIZE	8



static inline void mp_extend(mpool_pool_t *p, size_t size, mpool_t *pool);
static inline size_t mp_align(size_t size);
static inline size_t mp_decide_size(size_t size);



mpool_t *mp_create(size_t size)
{
	mpool_t *pool;
	size = mp_decide_size(size);
	MPOOL_MALLOC(pool, sizeof(*pool));
	MPOOL_MALLOC(pool->mpool, sizeof(*pool->mpool));
	MPOOL_MALLOC(pool->mpool->pool, size);
	memset(pool->mpool->pool, 0, size);

	if ( !pool->mpool || !pool->mpool->pool )
		return NULL;

	pool->mpool->next = NULL;

	pool->begin = pool->mpool->pool;
	pool->head = pool->mpool;
	pool->usize = 0;
	pool->msize = size;

	return pool;
}



void *mp_alloc(size_t size, mpool_t *pool)
{
	mpool_pool_t **p = &pool->mpool;
	mpool_pool_t *pp = *p;
	size_t usize = mp_align(pool->usize + size);
	size_t msize = pool->msize;
	void *d = pool->begin;

	if (usize > msize)
	{
		mp_extend(pp, usize*2+1, pool);
		pool->usize = 0;
		pool->msize = usize * 2;
		d = pool->begin;
		pool->begin += mp_align(size);
		*p = pp->next;
	}
	else
	{
		pool->usize = usize;
		pool->begin += mp_align(size);
	}

	return d;
}



void mp_destroy(mpool_t *pool)
{
	mpool_pool_t *p;
	for (p=pool->head; p!=NULL; )
	{
		mpool_pool_t *current = p;
		mpool_pool_t *next = p->next;
		MPOOL_FREE(current->pool);
		MPOOL_FREE(current);
		p = next;
	}
	MPOOL_FREE(pool);
}



/* Following is private function */

static inline void mp_extend(mpool_pool_t *p, size_t size, mpool_t *pool)
{
	size = mp_decide_size(size);
	mpool_pool_t *mpp;
	MPOOL_MALLOC(mpp, sizeof(*mpp));
	MPOOL_MALLOC(mpp->pool, size);
	memset(mpp->pool, 0, size);

	mpp->next = NULL;
	p->next = mpp;
	pool->begin = mpp->pool;
}



static inline size_t mp_align(size_t size)
{
	return (size + (MPOOL_ALIGN_SIZE - 1)) & ~(MPOOL_ALIGN_SIZE - 1);
}



static inline size_t mp_decide_size(size_t size)
{
	return size <= 0 ? MPOOL_POOL_SIZE : mp_align(size);
}
