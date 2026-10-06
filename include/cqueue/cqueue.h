/**
 * @file cqueue.h
 * @brief Generic circular queue implementation.
 *
 * @details
 * This library provides a generic FIFO circular queue capable of storing
 * elements of arbitrary fixed size.
 *
 * The queue does not require the C standard library. Memory and memory-copy
 * operations can be supplied by the application through @ref cqueue_port().
 *
 * The application must define the @c cqsize_t type before including this
 * header. For example:
 *
 * @code
 * #define cqsize_t size_t
 * #include "cqueue/cqueue.h"
 * @endcode
 *
 * The queue can use either:
 *
 * - an externally supplied buffer, owned by the application; or
 * - a dynamically allocated buffer using the configured @c malloc function.
 *
 * @note
 * The queue itself is not thread-safe. Synchronization, if required, must
 * be provided by the application or by a higher-level abstraction.
 */

#ifndef __cqueue_h__
#define __cqueue_h__

/**
 * @brief Size type used internally by cqueue.
 *
 * The application must define @c cqsize_t before including this header.
 *
 * Example:
 *
 * @code
 * #define cqsize_t size_t
 * @endcode
 */
#ifndef cqsize_t
    #error "cqsize_t must be defined by application. Example: #define cqsize_t size_t"
    #define cqsize_t unsigned int
#endif

#ifndef NULL
    /**
     * @brief Null pointer constant fallback.
     *
     * This definition is only used when the application has not already
     * provided a definition for @c NULL.
     */
    #define NULL ((void*)0)
#endif

/**
 * @brief Boolean type used by the CQueue API.
 *
 * This type is represented by an unsigned 8-bit-compatible integer type.
 * The values @ref CQueue_True and @ref CQueue_False are used to represent
 * boolean conditions.
 */
#define CQueue_Bool unsigned char

/**
 * @brief Boolean value representing true.
 */
#define CQueue_True 1

/**
 * @brief Boolean value representing false.
 */
#define CQueue_False 0


/**
 * @brief Error and status codes returned by cqueue operations.
 */
typedef enum {
    /** Operation completed successfully. */
    CQUEUE_OK,

    /** One or more function arguments are invalid. */
    CQUEUE_ERR_INVALID_ARG,

    /** The queue is not in a valid state for the requested operation. */
    CQUEUE_ERR_INVALID_STATE,

    /** Memory allocation failed. */
    CQUEUE_ERR_MALLOC,

    /** The queue does not have space for another element. */
    CQUEUE_ERR_FULL,

    /** The queue does not contain any elements. */
    CQUEUE_ERR_EMPTY
} cqueue_err_t;

/**
 * @brief Platform-dependent operations used by cqueue.
 *
 * @details
 * This structure allows the application to provide its own implementations
 * of memory and allocation functions.
 *
 * The @c memset and @c memcpy callbacks are optional. If they are set to
 * NULL, cqueue uses its own internal implementations.
 *
 * The @c malloc and @c free callbacks must either both be provided or both
 * be NULL. They are only required when cqueue_init() is called with a NULL
 * buffer and the queue therefore needs to allocate its own storage.
 *
 * @note
 * The callbacks are stored globally in @ref cqueue_port_fns. Consequently,
 * changing the configured port functions affects all queues using this
 * library.
 */
typedef struct {
    /**
     * @brief Memory initialization function.
     *
     * Equivalent in purpose to the standard C @c memset function.
     */
    void* (*memset)(void *__s, int __c, cqsize_t __n);

    /**
     * @brief Memory copy function.
     *
     * Equivalent in purpose to the standard C @c memcpy function.
     */
    void* (*memcpy)(void *__restrict__ __dest, const void *__restrict__ __src, cqsize_t __n);

    /**
     * @brief Memory allocation function.
     *
     * Used when cqueue_init() receives a NULL buffer.
     *
     * Must be provided together with @c free.
     */
    void* (*malloc)(cqsize_t __size);

    /**
     * @brief Memory deallocation function.
     *
     * Used by cqueue_end() when the queue owns a dynamically allocated
     * buffer.
     *
     * Must be provided together with @c malloc.
     */
    void (*free)(void *__ptr);
} cqueue_port_api_t;

/**
 * @brief Circular FIFO queue object.
 *
 * @details
 * The queue stores fixed-size elements in a circular buffer.
 *
 * The logical queue state is represented by three indexes/counters:
 *
 * - @c h: position of the next element to be removed;
 * - @c t: position where the next element will be inserted;
 * - @c c: current number of elements stored in the queue.
 *
 * The queue is empty when:
 *
 * @code
 * c == 0
 * @endcode
 *
 * and full when:
 *
 * @code
 * c == en
 * @endcode
 *
 * The buffer can contain exactly @c en elements; no slot needs to be
 * sacrificed to distinguish between the empty and full states.
 */
typedef struct {
    /**
     * @brief Pointer to the queue storage buffer.
     *
     * The buffer may be supplied by the application or allocated internally
     * by cqueue_init().
     */
    void *buf;

    /**
     * @brief Total buffer size in bytes.
     *
     * This value is equivalent to:
     *
     * @code
     * esz * en
     * @endcode
     */
    cqsize_t msz;

    /**
     * @brief Size of a single queue element in bytes.
     */
    cqsize_t esz;

    /**
     * @brief Maximum number of elements the queue can store.
     */
    cqsize_t en;

    /**
     * @brief Head position.
     *
     * Indicates the position of the next element to be removed.
     */
    cqsize_t h;

    /**
     * @brief Tail position.
     *
     * Indicates the position where the next element will be inserted.
     */
    cqsize_t t;

    /**
     * @brief Number of elements currently stored in the queue.
     */
    cqsize_t c;

    /**
     * @brief Buffer ownership flag.
     *
     * A non-zero value indicates that the buffer was dynamically allocated
     * by cqueue and must be released by cqueue_end().
     *
     * A zero value indicates that the buffer is externally owned by the
     * application and must not be released by cqueue.
     */
    unsigned char mf;
} cqueue_t;


#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief Configure platform-dependent operations.
 *
 * @param[in] port_fns Pointer to the callback configuration.
 *
 * @details
 * Configures the memory and allocation functions used internally by cqueue.
 *
 * If @c memset or @c memcpy is NULL, the library's internal implementation
 * is used.
 *
 * The @c malloc and @c free callbacks must be supplied as a pair. If only
 * one of them is provided, both are disabled.
 *
 * Passing NULL leaves the current configuration unchanged.
 *
 * @warning
 * The configuration is global and affects all queues.
 */
void cqueue_port(cqueue_port_api_t *port_fns);

/**
 * @brief Initialize a circular queue.
 *
 * @param[out] queue Pointer to the queue object to initialize.
 * @param[in]  esz    Size of each element in bytes.
 * @param[in]  en     Maximum number of elements.
 * @param[in]  buf    External storage buffer, or NULL to allocate internally.
 *
 * @return @ref CQUEUE_OK on success.
 * @return @ref CQUEUE_ERR_INVALID_ARG if @p queue is NULL, @p esz is zero,
 *         or @p en is zero.
 * @return @ref CQUEUE_ERR_INVALID_STATE if @p buf is NULL and no allocation
 *         function is configured.
 * @return @ref CQUEUE_ERR_MALLOC if internal buffer allocation fails.
 *
 * @details
 * Initializes a queue capable of storing @p en elements, each containing
 * @p esz bytes.
 *
 * If @p buf is not NULL, the supplied buffer is used directly and remains
 * owned by the application.
 *
 * If @p buf is NULL, cqueue allocates a buffer of:
 *
 * @code
 * esz * en
 * @endcode
 *
 * bytes using the configured @c malloc callback.
 *
 * The allocated or supplied buffer is initialized to zero.
 *
 * @note
 * When using an external buffer, the caller must ensure that the buffer
 * contains enough storage for @p esz * @p en bytes.
 */
cqueue_err_t cqueue_init(cqueue_t *queue, cqsize_t esz, cqsize_t en, void *buf);

/**
 * @brief Insert an element into the queue.
 *
 * @param[in,out] queue Pointer to an initialized queue.
 * @param[in]     ein   Pointer to the element to insert.
 *
 * @return @ref CQUEUE_OK if the element was inserted.
 * @return @ref CQUEUE_ERR_INVALID_ARG if @p queue or @p ein is NULL.
 * @return @ref CQUEUE_ERR_INVALID_STATE if the queue has no valid buffer.
 * @return @ref CQUEUE_ERR_FULL if the queue is already full.
 *
 * @details
 * Copies exactly @c queue->esz bytes from @p ein into the next available
 * position in the circular buffer.
 *
 * The element is inserted at the tail position and the tail advances
 * automatically with circular wrap-around.
 */
cqueue_err_t cqueue_insert(cqueue_t *queue, void *ein);

/**
 * @brief Remove the oldest element from the queue.
 *
 * @param[in,out] queue Pointer to an initialized queue.
 * @param[out]    eout  Destination where the removed element is copied.
 *
 * @return @ref CQUEUE_OK if an element was removed.
 * @return @ref CQUEUE_ERR_INVALID_ARG if @p queue or @p eout is NULL.
 * @return @ref CQUEUE_ERR_INVALID_STATE if the queue has no valid buffer.
 * @return @ref CQUEUE_ERR_EMPTY if the queue contains no elements.
 *
 * @details
 * Copies exactly @c queue->esz bytes from the oldest element into @p eout.
 *
 * Elements are removed in FIFO order: the first element inserted into the
 * queue is the first element returned by this function.
 */
cqueue_err_t cqueue_remove(cqueue_t *queue, void *eout);

/**
 * @brief Release resources associated with a queue.
 *
 * @param[in,out] queue Pointer to an initialized queue.
 *
 * @return @ref CQUEUE_OK if the queue was successfully ended.
 * @return @ref CQUEUE_ERR_INVALID_ARG if @p queue is NULL.
 * @return @ref CQUEUE_ERR_INVALID_STATE if the queue has no valid buffer.
 *
 * @details
 * If the queue owns its buffer, the configured @c free callback is used to
 * release it.
 *
 * Externally supplied buffers are never released by this function.
 *
 * After successful completion, the queue object is reset to zero.
 */
cqueue_err_t cqueue_end(cqueue_t *queue);

/**
 * @brief Checks whether the queue is empty.
 *
 * A queue is considered empty when it contains no elements.
 * Internally, this is determined by comparing the current element count
 * with zero.
 *
 * @param[in] queue Pointer to the queue to check.
 *
 * @return @ref CQueue_True if the queue contains no elements.
 * @return @ref CQueue_False otherwise.
 *
 * @note The queue pointer must not be NULL.
 */
CQueue_Bool cqueue_is_empty(const cqueue_t *queue);

/**
 * @brief Checks whether the queue is full.
 *
 * A queue is considered full when the number of stored elements reaches
 * its configured capacity.
 *
 * @param[in] queue Pointer to the queue to check.
 *
 * @return @ref CQueue_True if the queue contains the maximum number of
 *         elements.
 * @return @ref CQueue_False otherwise.
 *
 * @note The queue pointer must not be NULL.
 */
CQueue_Bool cqueue_is_full(const cqueue_t *queue);

/**
 * @brief Internal fallback memory initialization function.
 *
 * @param[out] __s Destination memory.
 * @param[in]  __c Value used to initialize each byte.
 * @param[in]  __n Number of bytes to initialize.
 *
 * @return The original destination pointer.
 *
 * @details
 * This function is used when the application does not provide a @c memset
 * implementation through @ref cqueue_port().
 *
 * It exists so that cqueue can operate without depending on the C standard
 * library.
 */
void *cqueue_memset(void *__s, int __c, cqsize_t __n);

/**
 * @brief Internal fallback memory copy function.
 *
 * @param[out] __dest Destination memory.
 * @param[in]  __src  Source memory.
 * @param[in]  __n    Number of bytes to copy.
 *
 * @return The original destination pointer.
 *
 * @details
 * This function is used when the application does not provide a @c memcpy
 * implementation through @ref cqueue_port().
 *
 * It exists so that cqueue can operate without depending on the C standard
 * library.
 *
 * @warning
 * The source and destination memory regions must not overlap.
 */
void *cqueue_memcpy(void *__restrict__ __dest, const void *__restrict__ __src, cqsize_t __n);


#ifdef __cplusplus
}
#endif


/**
 * @brief Global platform operation configuration.
 *
 * @details
 * This object contains the currently configured memory and allocation
 * callbacks used by cqueue.
 *
 * It can be configured using @ref cqueue_port().
 */
extern cqueue_port_api_t cqueue_port_fns;

#endif /* __cqueue_h__ */