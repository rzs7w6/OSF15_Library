# OS F15 Libraries
Current libraries:
- Bitmap (v1.0)
	- It's a bitmap!
	- v2 wishlist:
		- Overlay: import but use given buffer instead of allocating an internal one
		- FLZ/FLS
		- Better FFZ/FFS
		- Parameter checking
			- Just never give us a bad pointer or bit address and it's fine :p
- Dynamic_array (v1.0)
	- It's a vector!
	- Also supports front operations!
	    - But you should probably never use them for the obvious reasons.
	- Supports destructors! Function pointers are fun.
	- v2 wishlist:
		- Sorting
		- Inserting/removing n objects at a time
		    - Dyn's core already supports this, the API doesn't.
		    	- I didn't want to write more unit tests...


-- Will, the best TA