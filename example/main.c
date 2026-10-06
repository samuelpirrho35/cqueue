#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define cqsize_t size_t
#include "cqueue/cqueue.h"

typedef struct {
    int n;
    char s[32];
} example_t;

int main(){
    cqueue_port_api_t port_fns = {
        .memset = memset,
        .memcpy = memcpy,
        .malloc = malloc,
        .free   = free
    };

    cqueue_port(&port_fns);

    cqueue_t queue;
    cqueue_err_t err = cqueue_init(&queue, sizeof(example_t), 4, NULL);

    printf("err: %d\n\n", (int)err);

    for(int i = 0; i < 5; i++){
        example_t e = {
            .n = i
        };

        sprintf(e.s, "str: %d", i);

        err = cqueue_insert(&queue, &e);
        printf("err: %d\n", (int)err);
    }

    printf("\n");

    for(int i = 0; i < 5; i++){
        example_t e = { 0 };

        err = cqueue_remove(&queue, &e);
        printf("err: %d | data: ( %d | %s )\n", (int)err, e.n, e.s);
    }

    printf("\n");

    for(int i = 0; i < 3; i++){
        example_t e = {
            .n = i
        };

        sprintf(e.s, "str: %d", i + 5);

        err = cqueue_insert(&queue, &e);
        printf("err: %d\n", (int)err);
    }

    for(int i = 0; i < 3; i++){
        example_t e = { 0 };

        err = cqueue_remove(&queue, &e);
        printf("err: %d | data: ( %d | %s )\n", (int)err, e.n, e.s);
    }

    err = cqueue_end(&queue);
    printf("\nerr: %d\n", (int)err);
}
