#include "../include/dyn_list.h"

typedef struct node {
    node_t *prev, next; // ORDER AND LOCATION IS VITAL, DO NOT REORDER THESE (make it an array of 2?)
} node_t;

struct dyn_list {
    node_t *back, front; // ORDER AND LOCATION IS VITAL, DO NOT REORDER THESE (make it an array of 2?)
    size_t size;
    size_t data_size;
    void (*destructor)(void *const);
};

// Since I can't scan my notes and attach it here...
/*
	going to have the front and back root pointers point to nodes, duh, but nodes MAY OR MAY NOT point back to root
	because I'm not sure about implementation yet (should just require an extra loop condition)

	There is a vital test in the test folder. It will assert that the relative offset of root's back and front are the same
	as the node relative offsets. This way, root can be cast to a node and not cause a freakout, and then we can probably do the full
	circular link.
	Another vital assert will be that the size of the node is equal to the sum of it's parts. Padding will mess up addressing

	Not going to store the size and destructor in the nodes because that's a major waste of space, though it is a little more restrictive
	The list-who-shall-not-be-named will be interesting and do this to pull off... weird things.
*/

// Because nodes are getting weird to allow for contiguity
#define NODE_MALLOC(data_size) malloc(sizeof(node_t) + data_size)

// #define DATA_POINTER(node_ptr) ((void *)((&(node_ptr->next)) + 1))
// WOAH, this is WAY better. This removes the requirement that the node size be the sum of the parts
// So, if node gets padded somehow, we'll avoid the disaster case of us just going crazy, but we will waste the padded space
// ...but isn't padding meant to be wasted?
#define DATA_POINTER(node_ptr) (void *) ((node_ptr + 1)))


dyn_list_t *dyn_list_create(size_t data_size, void (*destruct_func)(void *const)) {
    dyn_list_t dyn_list = NULL;
    if (data_size) {
        dyn_list = malloc(sizeof(dyn_list_t));
        if (dyn_list) {
            dyn_list->back = dyn_list;
            dyn_list->front = dyn_list;
            dyn_list->size = 0;
            dyn_list->data_size = data_size;
            dyn_list->destructor = destruct_func;
        }
    }
    return dyn_list;
}

void dyn_list_destroy(dyn_list_t *const dyn_list) {
    // it was ok to do a double pass with dyn_array, but we're not contiguous,
    //  so two traversals is kinda gross
    // But an if in the inner loop is also gross.
    // But an if selecting two nearly-idential loops is also gross
    // ...Two loops, but the first makes an array of pointers, hahaha.
    //    A destructor that mallocs is gross and weird

    // TODO: Get smarter and find a better way to do this
    // TODO: Also, probably aggregate this somewhere because clear will want a copy
    if (dyn_list) {
        // technically the for will do this as well, but with less work
        // ...but more work in the average (we have contents) case... HMMMMM
        // TODO: Be better
        if (dyn_list->size) {
            for (node_t *itr = dyn_list->front, *ptr_to_free = itr;
                    itr != (node_t *const)dyn_list;
                    itr = itr->next, ptr_to_free = itr) {
                if (dyn_list->destructor) {
                    dyn_list->destructor(DATA_POINTER(itr));
                }
                // Uhh, gotta shift the pointer but also free it. Hmmmm
                free(ptr_to_free);
            }
        }
        free(dyn_list);
    }
}
