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
    (two days into tinkering with implementation seems to indicate that this is a bad idea)
    (this is really only done in the STL to do a quick empty check (empty = front == back))
    (but I have a size value, so having the ends link somewhere, especially to root, who has no data, is just weird)

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



// Modes of operation for dyn_shift
// ...purge is more fun than remove. TODO: Grow up and rename PURGE to REMOVE... maybe
typedef enum {INSERT = 0x01, EXTRACT = 0x02, PURGE = 0x04} DYN_CORE_MODE;

// The heart of any non-trivial operation, check the impl for details
// It'll validate data, just pass it the data with minimal checks
bool dyn_core(dyn_list_t *const dyn_list, const size_t position, const size_t count, const DYN_CORE_MODE mode, void *const data_location);


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


/*

    OH BOY, DYN_SHIFT BUT FOR WEIRD DATA

    dyn_core is the, well, core of any non-trivial dyn function
     (though traversal will probably need to be a function somewhere)
    You want it? dyn_core does it... assuming you're editing N rectilinear nodes. Otherwise, go away

    So, let's get started.

    Configuration is determined by the mode parameter. It's a DYN_CORE_MODE enum.
    Right now there's INSERT, EXTRACT, and PURGE. They do what they sound like.

    These operations are designed to be as safe as possible. If we return true, you're good to go.
        If it's false, well, keeping a clean state may be difficult since we're not contiguous and can't just unwind
        There will be a lot of error code :/ BUT! If it's false, the list will stay in the condition it was given
        It might be a parameter thing. It might be that the list is totaly busted. You may be out of memory.
        That's for you to figure out or just pass along. I miss exceptions. NO ERRNO AHHH.

        Oh, unless we segfault. But we shouldn't. Does that mean we have to validate every pointer? Idk!
        We might just validate root and assume the data didn't get mangled. That's potentially dangerous
        since this is supposed to be for OS code and stuff... so we probably shouldn't do that. We'll see.

    INSERT will attempt to insert N nodes before the given position, using data from the data array
        Uhh... inserting on root may freak it out. Important non-edge edge case (surface case?), haha.
        The doubly-linked nature in general may be a little annoying to manage and the usual edges may
        become a major pain since we're managing EVERYTHING
        If basic sanity checks pass, this operation may fail due to memory allocations.
        We have to do a lot of them (N+1) and they may be large.

    EXTRACT will attempt to extract the data from N nodes, startign with the given position, and
        pipe the data into the given data location, deleting the nodes once it's finished.
        This means it has to do two traversals. Deal.
        If basic sanity checks pass, this operation never fails.

    PURGE will, well, purge the data, applying the destructor if applicable. This may or may not be
        two traversals. I'm still figuring that out.
        Purge isn't exactly a "safe" operation in that we can't really revert if it breaks.
        If basic sanity checks pass, this operation never fails.

    HOO BOY, this funcion, with all its edge cases and whatnot is going to be WAY TOO BIG. Like, crazy complex.
    Each mode had its own pile of concerns. dyn_shift was ok for dyn_array, but I think dyn_core is going to be too unmanageable
    How sad :C

*/
/*
    bool dyn_core(dyn_list_t *const dyn_list, const size_t position, const size_t count, const DYN_CORE_MODE mode, void *const data_location) {

        if (dyn_list && count) {
            if (mode == INSERT && position <= dyn_list->size) {


            } else if (mode & (PURGE | EXTRACT)) { // we'e some sort of removal operation
                if ((position + count) <= dyn_list->size) {
                    // request is sane

                }
            }
        }
        return false;
    }
*/
bool dyn_core_insert(dyn_list_t *const dyn_list, const size_t position, const size_t count, const void *const data_src);

bool dyn_core_extract(dyn_list_t *const dyn_list, const size_t position, const size_t count, void *data_dest);

bool dyn_core_purge(dyn_list_t *const dyn_list, const size_t position, const size_t count);

// rename to at? Locate? ??????
node_t *dyn_core_traverse(const dyn_list_t *const dyn_list, const size_t position) {
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
        for(size_t distance = offset ? position : size - position - 1;
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




