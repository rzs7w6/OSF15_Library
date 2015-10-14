#include <stdlib.h>
#include <string.h>
#include <stddef.h>

// Gotta avoid clobbering with list, so names are a little weird
// Maybe I should just append a UUID to every header, haha

typedef struct dyn_list dyn_list_t;


// push/pop/extract front/back
// extract/erase/at(?)
// prune (remove those who's provided function says to remove/extract (WHICH???))
// destroy/create/import
// sort (awwwww, can't just use qsort now... well, without doing something gross)


// try to follow the order of dyn_array?

// create
// import
// export (uhh... give up space for the data and serialize?)
// destroy

dyn_list_t *dyn_list_create(const size_t data_size, void (*destruct_func)(void *const));

// just going to be a loop of push_backs? Surely it can be optimized.
// Something like dyn_shift. dyn_alter? dyn_core? enum on insert/extract/remove?
// Ooooohhh I like that.
dyn_list_t *dyn_list_import(const void *const data, const size_t count, const size_t data_type_size, void (*destruct_func)(void *));

// uhh, return the given pointer if it worked completely? (or at least as far as we know)
// Null if it didn't?
// UHHHHHHHHH no real way to report "shit broke halfway through"
// Or BAD POINTER
// I am NOT adding another errno because ehhhhhhh
// Think it's best to just say "Content of data array not guarenteed on error"
// and return data as well on success.
void *dyn_array_export(const dyn_array_t *const dyn_array, void *data);

void dyn_list_destroy(dyn_list_t *const dyn_list);