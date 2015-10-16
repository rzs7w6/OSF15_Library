#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdbool.h>

// Gotta avoid clobbering with list, so names are a little weird
// Maybe I should just append a UUID to every header, haha

typedef struct dyn_list dyn_list_t;

typedef struct dyn_list_itr dyn_list_itr_t;


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

dyn_list_t *dyn_list_create(const size_t data_type_size, void (*destruct_func)(void *const));

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
// and return data as well on success. Uhh... bool? I forgot about bools.
// Cannot fail on correct parameters
bool dyn_array_export(const dyn_array_t *const dyn_array, void *data);

void dyn_list_destroy(dyn_list_t *const dyn_list);

push_front
pop_front
extract_front

push_back
pop_back
extract_back

insert
extract
remove
at

itr_insert // insert at position, iterator does not change, prev of node does
itr_extract // removes itr, position gets incremented
itr_remove // removes itr, position gets incremented
itr_at // just returns pointer of itr'd data (rooted = NULL)

itr_create // take list and desired position? itr_begin itr_end itr_at itr_create(rooted)?
itr_destroy
itr_reset
itr_rooted // need better name, detect if itr at root (is reset/taversal complete)
itr_inc
itr_dec // merge the two?
itr_distance // check backwards or return SIZE_MAX if begin after end?
itr_equal // same as distance == 0, which short circuits distance, but if not eq, would trigger full distance hunt

sort
splice
merge
remove_if/prune
for_each
