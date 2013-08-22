#ifndef MPOOL_H_INCLUDED
#define MPOOL_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>



#define MPOOL_MALLOC(p, size)				\
	do {									\
		if (((p) = malloc(size)) == NULL) {	\
			perror("malloc");				\
			exit(EXIT_FAILURE);				\
		}									\
	} while(0)								\

#define MPOOL_FREE(p)						\
	do {									\
		if (p != NULL) {					\
			free(p);						\
			(p) = NULL;						\
		}									\
	} while(0)								\



typedef struct _mpool_pool_t_
{
	void					*pool;
	struct _mpool_pool_t_	*next;
} mpool_pool_t;



typedef struct mpool_t
{
	mpool_pool_t	*head;	// Memory pool head
	void			*begin; // Data for internal conduct
	size_t			usize;	// Used pool size
	size_t			msize;	// Max pool size
	mpool_pool_t	*mpool;	// Memory pool
} mpool_t;



mpool_t *mp_create(size_t size);
void *mp_alloc(size_t size, mpool_t *pool);
void mp_destroy(mpool_t *pool);



#endif /* MPOOL_H_INCLUDED */

