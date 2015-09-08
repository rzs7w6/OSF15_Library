#ifndef DYNAMIC_ARRAY_H__
#define DYNAMIC_ARRAY_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef struct dynamic_array dynamic_array_t;
// Next version, push/pop_N_back/front for bulk loading
// Erase_n
// etc


// Creates a dyn_array with the desired capacity (not size!), object size, and optional destructor (NULL to disable)
dynamic_array_t *dynamic_array_create(const size_t capacity, const size_t data_type_size, void (*destruct_func)(void *));

// Creates a dynamic array from a standard array
dynamic_array_t *dynamic_array_import(void *const data, const size_t count, const size_t data_type_size, void (*destruct_func)(void *));

// Returns a pointer to an array for bulk exporting (for example, to disk)
// Pointer is internal, so if you break it, it's your own fault. DO NOT FREE.
// Pointer may be invalidated on size increase
// Don't forget to query the array's size.
// (change to a struct of {size, pointer} ???)
const void *dynamic_array_export(const dynamic_array_t *const dyn_array);

// Deconstructs dynamic array, applying optional destructor to remaining elements
void dynamic_array_destroy(dynamic_array_t *const dyn_array);




// Prefer the X_back functions if you use a lot of push/pop operations

// All insertions/extractions are via memcpy, so giving us pointers overlapping ourselves is UNDEFINED
// Which is FINE. If you need overlapping, that's really weird and you probably shouldn't, but
// you should also be smart enough to to edit the 2(!) memcpys to allow it if you REALLY need it

// Returns a pointer to the object at the front of the array
void *dynamic_array_front(const dynamic_array_t *const dyn_array);

// Copies the given object and places it at the front of the array, increasing container size by one
bool dynamic_array_push_front(dynamic_array_t *const dyn_array, void *const object);

// Removes and optionally destructs the object at the front of the array, decreasing the container size by one
// Returns false only when array is empty or NULL was given
bool dynamic_array_pop_front(dynamic_array_t *const dyn_array);

// Removes the object in the front of the array and places it in the desired location, decreasing container size
// Does not destruct since it was returned to the user
bool dynamic_array_extract_front(dynamic_array_t *const dyn_array, void *const object);




// Returns a pointer to the object at the end of the array
void *dynamic_array_back(const dynamic_array_t *const dyn_array);

// Copies the given object and places it at the back of the array, increasing container size by one
bool dynamic_array_push_back(dynamic_array_t *const dyn_array, void *const object);

// Removes and optionally destructs the object at the back of the array
bool dynamic_array_pop_back(dynamic_array_t *const dyn_array);

// Removes the object in the back of the array and places it in the desired location
// Does not destruct since it was returned to the user
bool dynamic_array_extract_back(dynamic_array_t *const dyn_array, void *const object);



// Returns a pointer to the desired object in the array
// Pointer may be invalidated if the container increases in size
void *dynamic_array_at(const dynamic_array_t *const dyn_array, const size_t index);

// Inserts the given object at the given index in the array, increasing the container size by one
// and moving any contents at index and beyond down one
bool dynamic_array_insert(dynamic_array_t *const dyn_array, const size_t index,
                          void *const object);

// Removes and optionally destructs the object at the given index
bool dynamic_array_erase(dynamic_array_t *const dyn_array, const size_t index);

// Removes the object at the given index and places it at the desired location
// Does not destruct the object since it is returned to the user
bool dynamic_array_extract(dynamic_array_t *const dyn_array, const size_t index,
                           void *const object);



// Removes and optionally destructs all array elements
void dynamic_array_clear(dynamic_array_t *const dyn_array);

// Tests if array is empty
bool dynamic_array_empty(const dynamic_array_t *const dyn_array);

// Returns size of array
size_t dynamic_array_size(const dynamic_array_t *const dyn_array);

#endif