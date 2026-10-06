#ifndef cqsize_t
    #define cqsize_t unsigned int
#endif

#include "cqueue/cqueue.h"

#define _port_fns cqueue_port_fns

#define BUF_ALLOCATED_BY_MALLOC 1
#define BUF_EXTERN              0

#define HEAD 1
#define TAIL 0

typedef unsigned char cqu8_t;

static inline cqsize_t next_pos(cqueue_t *queue, cqu8_t h);

cqueue_port_api_t cqueue_port_fns =
    {
        .memset = cqueue_memset,
        .memcpy = cqueue_memcpy,
        .malloc = NULL,
        .free   = NULL
    };

void cqueue_port(cqueue_port_api_t *port_fns){
    if(port_fns == NULL){ return; }

    cqueue_memcpy(&_port_fns, port_fns, sizeof(cqueue_port_api_t));

    if(port_fns->memset == NULL){
        _port_fns.memset = cqueue_memset;
    }

    if(port_fns->memcpy == NULL){
        _port_fns.memcpy = cqueue_memcpy;
    }

    if((port_fns->malloc == NULL && port_fns->free) || (port_fns->malloc && port_fns->free == NULL)){
        _port_fns.malloc = NULL;
        _port_fns.free   = NULL;
    }
}

cqueue_err_t cqueue_init(cqueue_t *queue, cqsize_t esz, cqsize_t en, void *buf){
    if(queue == NULL || !esz || !en){
        return CQUEUE_ERR_INVALID_ARG;
    }

    _port_fns.memset(queue, 0, sizeof(cqueue_t));

    queue->esz = esz;
    queue->en  = en;
    queue->msz = esz * en;

    queue->h = 0;
    queue->t = 0;
    queue->c = 0;

    if(buf == NULL){
        if(_port_fns.malloc == NULL){
            return CQUEUE_ERR_INVALID_STATE;
        }

        queue->buf = _port_fns.malloc(queue->msz);
        queue->mf  = BUF_ALLOCATED_BY_MALLOC;

        if(queue->buf == NULL){
            return CQUEUE_ERR_MALLOC;
        }
    }

    else {
        queue->buf = buf;
        queue->mf  = BUF_EXTERN;
    }

    _port_fns.memset(queue->buf, 0, queue->msz);
    return CQUEUE_OK;
}

cqueue_err_t cqueue_insert(cqueue_t *queue, void *ein){
    if(queue == NULL || ein == NULL){
        return CQUEUE_ERR_INVALID_ARG;
    }

    if(queue->buf == NULL){
        return CQUEUE_ERR_INVALID_STATE;
    }

    if(queue->c == queue->en){
        return CQUEUE_ERR_FULL;
    }

    _port_fns.memcpy(queue->buf + queue->esz * queue->t, ein, queue->esz);

    queue->t = next_pos(queue, TAIL);
    queue->c++;

    return CQUEUE_OK;
}

cqueue_err_t cqueue_remove(cqueue_t *queue, void *eout){
    if(queue == NULL || eout == NULL){
        return CQUEUE_ERR_INVALID_ARG;
    }

    if(queue->buf == NULL){
        return CQUEUE_ERR_INVALID_STATE;
    }

    if(queue->c == 0){
        return CQUEUE_ERR_EMPTY;
    }

    _port_fns.memcpy(eout, queue->buf + queue->esz * queue->h, queue->esz);

    queue->h = next_pos(queue, HEAD);
    queue->c--;

    return CQUEUE_OK;
}

cqueue_err_t cqueue_end(cqueue_t *queue){
    if(queue == NULL){
        return CQUEUE_ERR_INVALID_ARG;
    }

    if(queue->buf == NULL){
        return CQUEUE_ERR_INVALID_STATE;
    }

    if(queue->mf){
        _port_fns.free(queue->buf);
    }

    _port_fns.memset(queue, 0, sizeof(cqueue_t));
    return CQUEUE_OK;
}

CQueue_Bool cqueue_is_empty(const cqueue_t *queue){
    return queue->c == 0 ? CQueue_True : CQueue_False;
}

CQueue_Bool cqueue_is_full(const cqueue_t *queue){
    return queue->c == queue->en ? CQueue_True : CQueue_False;
}

void *cqueue_memset(void *__s, int __c, cqsize_t __n){
    cqu8_t *_s = (cqu8_t*)__s;

    for(cqsize_t i = 0; i < __n; i++){
        _s[i] = __c;
    }

    return __s;
}

void *cqueue_memcpy(void *__restrict__ __dest, const void *__restrict__ __src, cqsize_t __n){
    cqu8_t *_dest = (cqu8_t*)__dest;
    cqu8_t *_src  = (cqu8_t*)__src;

    for(cqsize_t i = 0; i < __n; i++){
        _dest[i] = _src[i];
    }

    return __dest;
}

static inline cqsize_t next_pos(cqueue_t *queue, cqu8_t h){
    cqsize_t x = h ? queue->h : queue->t;
    return x == (queue->en - 1) ? 0 : (x + 1);
}

#undef _port_fns

#undef BUF_ALLOCATED_BY_MALLOC
#undef BUF_EXTERN

#undef HEAD
#undef TAIL
