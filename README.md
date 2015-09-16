# OS F15 Libraries
Current libraries:
- bitmap (v1.0)
	- It's a bitmap!
	- Wishlist:
		- Overlay: import but use given buffer instead of allocating an internal one
		- FLZ/FLS
		- Better FFZ/FFS
		- Parameter checking
			- Just never give us a bad pointer or bit address and it's fine :p
		- Rename export to data (that's what C++ calls it)???
- dyn_array (v1.2.2)
	- It's a vector, it's a stack, it's a deque, it's all your hopes and dreams!
	- Supports destructors! Function pointers are fun.
	- Wishlist:
		- Better insert_sorted (~~bsearch-based~~ Bsearch doesn't work that way, have to do it manually)
		- dyn_array_for_each, takes a func pointer and applies the func to all objects
		- shrink_to_fit (add a flag to the struct, have it be read by dyn_request_size_increase)
		- Prune (remove those who match a certain criteria (via function pointer))
		- Rename export to data (that's what C++ calls it)???
		- Inserting/removing n objects at a time
		    - Dyn's core already supports this, the API doesn't.
		    	- I didn't want to write more unit tests...


-- Will, the best TA
