// Dynamic arrays via macros over structs in C
// Author: mentoltea (Agoev T.)
// https://github.com/mentoltea/cutils

#ifndef CUTILS_DARRAY
#define CUTILS_DARRAY

/*
Default darray struct:
{
    T* items;
    size_t count;
    size_t capacity;    
}
*/

#include "assert.h"

#ifndef CUTILS_DARRAY_ALLOCATOR
#include <stdlib.h>
// void* malloc( size_t size );
#define CUTILS_DARRAY_ALLOCATOR malloc
#endif

#ifndef CUTILS_DARRAY_DEALLOCATOR
#include <stdlib.h>
// void free( void *ptr );
#define CUTILS_DARRAY_DEALLOCATOR free
#endif

#ifndef CUTILS_DARRAY_MEMCPY
#include <string.h>
// void* memcpy( void* dest, const void* src, std::size_t count );
#define CUTILS_DARRAY_MEMCPY memcpy
#endif

#ifndef CUTILS_DARRAY_DEFAULT_SIZE
#define CUTILS_DARRAY_DEFAULT_SIZE 10
#endif

#ifndef CUTILS_DARRAY_DEFAULT_EXPANSION
#define CUTILS_DARRAY_DEFAULT_EXPANSION 2.0
#endif

#define da_free(da) \
{ \
    if ( (da).items ) { \
        CUTILS_DARRAY_DEALLOCATOR((da).items); \
    } \
    (da).items = NULL; \
    (da).count = 0; \
    (da).capacity = 0; \
}

#define da_append(da, elem) \
{ \
    if (!(da).items) { \
        (da).items = CUTILS_DARRAY_ALLOCATOR( CUTILS_DARRAY_DEFAULT_SIZE * sizeof( *((da).items) ) ); \
        (da).capacity = CUTILS_DARRAY_DEFAULT_SIZE; \
        (da).count = 0; \
    } \
    if ((da).count == (da).capacity) { \
        size_t newcap = CUTILS_DARRAY_DEFAULT_EXPANSION * (da).capacity; \
        void* newitems = CUTILS_DARRAY_ALLOCATOR( newcap * sizeof( *((da).items) ) ); \
        CUTILS_DARRAY_MEMCPY(newitems, (da).items, (da).count * sizeof( *((da).items)) ); \
        CUTILS_DARRAY_DEALLOCATOR( (da).items ); \
        (da).items = newitems; \
        (da).capacity = newcap; \
    } \
    (da).items[(da).count] = elem; \
    (da).count++; \
}   

#define da_appends(da, T, ...) \
{ \
    assert( sizeof( *((da).items) ) == sizeof( T ) && "Assertion of da_appends" ); \
    T _append_buf_[] = { __VA_ARGS__ }; \
    size_t _arr_count = sizeof( (_append_buf_) )/(sizeof( *(_append_buf_) )); \
    if ((da).capacity < (da).count + _arr_count) { \
        size_t newcap = (da).count + _arr_count; \
        void* newitems = CUTILS_DARRAY_ALLOCATOR( newcap * sizeof( *((da).items) ) ); \
        CUTILS_DARRAY_MEMCPY(newitems, (da).items, (da).count * sizeof( *((da).items)) ); \
        CUTILS_DARRAY_DEALLOCATOR( (da).items ); \
        (da).items = newitems; \
        (da).capacity = newcap; \
    } \
    CUTILS_DARRAY_MEMCPY((da).items + (da).count, _append_buf_, _arr_count * sizeof(T)); \
    (da).count += _arr_count; \
}

// da1: 1, 2, 3
// da2: 4, 5, 6
// da_concat(da1, da2) ->
// da1: 1, 2, 3, 4, 5, 6
// da2: 4, 5, 6
#define da_concat(da1, da2) \
{ \
    assert( sizeof( *((da1).items) ) == sizeof( *((da2).items) ) && "Assertion of da_concat" ); \
    if ((da1).capacity < (da1).count + (da2).count) { \
        if ( ! (da1).items ) { \
            (da1).items = CUTILS_DARRAY_ALLOCATOR( (da2).count * sizeof( *((da2).items) ) ); \
            (da1).capacity = (da2).count; \
            (da1).count = 0; \
        } else { \
            size_t newcap = (da1).count + (da2).count; \
            void* newitems = CUTILS_DARRAY_ALLOCATOR( newcap * sizeof( *((da1).items) ) ); \
            CUTILS_DARRAY_MEMCPY(newitems, (da1).items, (da1).count * sizeof( *((da1).items)) ); \
            CUTILS_DARRAY_DEALLOCATOR( (da1).items ); \
            (da1).items = newitems; \
            (da1).capacity = newcap; \
        } \
    } \
    CUTILS_DARRAY_MEMCPY((da1).items + (da1).count, (da2).items, (da2).count * sizeof( *((da2).items)) ); \
    (da1).count += (da2).count; \
}

#define da_pop(da, index) \
{\
    CUTILS_DARRAY_MEMCPY((da).items + (index), (da).items + (index) + 1, ( (da).count - (index) - 1 ) * sizeof( *((da).items))); \
    (da).count--; \
}

#define da_foreach(T, it, da) for (T* (it) = (da).items; (it) < (da).items + (da).count; (it)++)

#define da_copy(da_to, da_from) { \
    da_free( (da_to) ); \
    (da_to).items = CUTILS_DARRAY_ALLOCATOR( (da_from).count * sizeof( *((da_from).items) ) ); \
    (da_to).capacity = (da_from).count; \
    CUTILS_DARRAY_MEMCPY((da_to).items, (da_from).items, (da_from).count * sizeof( *((da_from).items) ) ); \
    (da_to).count = (da_from).count; \
}


#endif // CUTILS_DARRAY