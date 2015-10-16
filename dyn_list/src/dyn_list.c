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
    Root connects to the two ends, BUT THE TWO ENDS ALSO CONNECT TO ROOT. It makes relinking faster, and NO LINK IS EVER NULL
    Root, when empty, should point to itself at both ends. PREFER SIZE CHECK TO LINK CHECK

    There is a vital test in the test folder. It will assert that the relative offset of root's back and front are the same
    as the node relative offsets. This way, root can be cast to a node and not cause a freakout, and that the circular link is fine

    Not going to store the size and destructor in the nodes because that's a major waste of space, though it is a little more restrictive
    The list-who-shall-not-be-named (hetero_list) will be interesting and allow varying size and destructors
    (does that make this homo_list? It would be a better name than dyn_list...)
*/

// #define DATA_POINTER(node_ptr) ((void *)((&(node_ptr->next)) + 1))
// WOAH, this is WAY better. This removes the requirement that the node size be the sum of the parts
// So, if node gets padded somehow, we'll avoid the disaster case of us just going crazy, but we will waste the padded space
// ...but isn't padding meant to be wasted?
#define DATA_POINTER(node_ptr) (void *) ((node_ptr + 1)))

// Does what it sounds like, inserts count objects from data_src at position
// False on parameter or malloc failure
bool dyn_core_insert(dyn_list_t *const dyn_list, const size_t position, const size_t count, const void *const data_src);

// Extracts count objects from position, placing it in data_dest and removing the nodes
// False on parameter error
bool dyn_core_extract(dyn_list_t *const dyn_list, const size_t position, const size_t count, void *data_dest);

// Deconstructs count objects at position and removes the nodes.
// False on parameter error
bool dyn_core_deconstruct(dyn_list_t *const dyn_list, const size_t position, const size_t count);

// Hunts down the requested node, NULL on parameter issue
node_t *dyn_core_locate(const dyn_list_t *const dyn_list, const size_t position);

/*
    Considering hiding even dyn_core_locate down below, making EVERYTHING a thin wrapper to a dyn_core
    But the complex functions (sort, prune, map/transform) would end up just being a wrapper of the dyn_core version
        Which seems like a dumb level of indirection

    So, for now, just call DATA_POINTER on the locate... ocne you've check locate didn't NULL on you
        Maybe a locate_data would be better, even if it's just sticking the two together (inline!)

    // Hunts down the data pointer at the requested node, NULL on parameter issue
    void *dyn_core_locate_data(const dyn_list_t *const dyn_list, const size_t position);
*/

// This should be the only creation function
dyn_list_t *dyn_list_create(const size_t data_type_size, void (*destruct_func)(void *const)) {
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

dyn_list_t *dyn_list_import(const void *const data, const size_t count, const size_t data_type_size, void (*destruct_func)(void *)) {
    dyn_list_t dyn_list = dyn_list_create(data_type_size, destruct_func);
    if (dyn_list) {
        // Sorry, lied about the const because core can't handle a const pointer without making it take two, or just de-consting it there
        // dyn_array has to do this, too. I don't like it. A single wasted pointer on the stack can't be that bad.
        // TODO: Figure out what to do with this const casting
        if (! dyn_core(dyn_list, 0, count, INSERT, (void *const)data)) {
            // eww, a jump on the normal path. BOOOOOOOOOOoooooooooooooooo
            dyn_list_destroy(dyn_list);
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


//
///
// HERE BE DRAGONS
///
//


// Because nodes are getting weird and I love them
#define NODE_MALLOC(data_size) malloc(sizeof(node_t) + data_size)

// Returns an array of linked nodes of size count
// Remember to set the pref of the front and next of the back
// Also, free the returned pointer when linked up
// Null on param or malloc failure
// Probably overkill if you're just creating one node
node_t **dyn_core_allocate(const size_t count, const size_t data_size);

// Declared up top since core is gone
//bool dyn_core_insert(dyn_list_t *const dyn_list, const size_t position, const size_t count, const void *const data_src);
//bool dyn_core_extract(dyn_list_t *const dyn_list, const size_t position, const size_t count, void *data_dest);
//bool dyn_core_deconstruct(dyn_list_t *const dyn_list, const size_t position, const size_t count);

// DO NOT TOUCH IF YOU ARE NOT CORE EXTRACT OR DECONSTRUCT
// It will break all of your hopes, dreams, and data integrity if you use it wrong
// It doesn't even check if these objects are all in the same list
// This is meant to be a speedy helper
// DO NOT TOUCH
// YOU HAVE BEEN WARNED

// ANYWAY, unlinks nodes begin and end from the list, correcting any and all links, decrements count,
//  and then starts killing all nodes from begin to end
void dyn_core_purge(dyn_list_t *const dyn_list, node_t *begin, node_t *end, const size_t count) {
    if (dyn_list && begin && end && count) {
        dyn_list->size -= count; // COUNT = CORRECTED, LIST IN BAD STATE
        begin->prev->next = end->next; // BEGIN UNLINKED AND RE-ROUTED, LIST CANNOT BE BACK-TRAVERSED CORRECTLY, LIST IN BAD STATE
        end->next->prev = begin->prev; // END UNLINKED AND RE-ROUTED, LIST IN GOOD STATE
    }

}

// returns node pointer of requested node, NULL on bad request
node_t *dyn_core_locate(const dyn_list_t *const dyn_list, const size_t position) {
    node_t *itr = NULL;
    if (dyn_list && position < dyn_list->size) {
        // Might be a bad idea

        // This could be done with a relative offset since front/back next/prev are the same offsets
        // instead of two loops.
        // EX: offset = front_half ? 1 : 0
        // itr = ((node_t*)dyn_list)[offset]
        // for(distance = front_half? position : size - position;distance;distance--)
        // itr = ((node_t*)itr)[offset]
        // In a perfect world where I have time to do things, I'd test which is better
        // Becuse that sounds neat, but that addition sounds hmmmm
        // Screw it, let's do something cool.

        int offset = (position <= dyn_list->size >> 1) ? 1 : 0;
        itr = ((node_t **)dyn_list)[offset]; // itr is now either front or back node
        for (size_t distance = offset ? position : size - position - 1;
                distance;
                --distance) {
            itr = ((node_t **)itr)[offset]; // So, 1 for forward, 0 for backwards. Neat, huh?
        }

        /*
            if (position <= (dyn_list->size >> 1)) { // Request for the front half
                itr = dyn_list->back;
                for(size_t distance = position)
            } else { // Request for the back half

            }
        */
    }
    return itr;
}




