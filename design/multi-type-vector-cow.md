# multi_type_vector copy-on-write (COW) design notes

This document details the non-obvious design choices behind the
copy-on-write (COW) support in the SOA `multi_type_vector`.

## The two ownership states

An instance is always in exactly one of two states, determined only
by whether `m_cow_store` is null:

- **Sole owner** (`m_cow_store == nullptr`): the instance owns the element
  blocks stored in `m_block_store.element_blocks` and is responsible
  for freeing them.
- **Borrowing** (`m_cow_store != nullptr`): the blocks are owned by a
  shared store (`shared_element_blocks`) reachable through a
  `std::shared_ptr`.  The instance's own `element_blocks` pointers point to
  the element blocks stored in the shared store, and the instance must never
  free them itself.

The shared store frees each block exactly once, in its destructor, when
the last borrower drops its `shared_ptr` reference.  The `shared_ptr`
reference count is the only sharing bookkeeping; there is no per-block
count.

Copying a COW instance via copy construction or `clone()` does not
deep-copy the blocks.  The source calls `share()`, which dynamically
creates the shared store and moves ownership of the block pointers into
it, then both source and copy point at the same shared store.  See
`share()` and the copy / clone constructors on how that works.  At this
point, the source transitions from being a sole owner to being a borrower
itself.

## How block deletion is handled

Deletion has to free every block exactly once and never free a block that
another instance still borrows.  How that is achieved depends on the
state.

### Without COW (or COW instance that is sole owner)

`delete_element_block()` is the primary path for freeing blocks.  It fires
the `element_block_released` event, calls `block_funcs::delete_block()`,
and nulls the slot.  `delete_element_blocks()` loops over it.  Every
mutating operation that removes or replaces blocks eventually finds its
way to this function.

### With COW while borrowing

A borrowing instance must not free shared blocks from its own mutators.
If it goes out of scope while no mutations have been made, it will simply
release its reference to the shared store and it will not free element
blocks on its own.  If the last borrower goes out of scope, then the
reference count of the shared store becomes zero thereby invoking the
destructor of the `shared_element_blocks`, which in turn frees the element
blocks.

If its content gets mutated while borrowing, it first detaches itself from
the shared ownership by copying (or cloning rather) all element blocks in
the shared store and releasing its reference to it first *then* mutates
the freshly copied content.  At this point it becomes a sole owner of the
blocks, and the block deletion happens the same way it does without COW.
See `detach_impl()` on how the detaching process works.

## Event handler semantics under COW

The `element_block_acquired` / `element_block_released` events describe
*logical* acquisition and release as seen by the client, not physical
allocation.  Because of that they fire even for shallow shares and
borrows:

- The copy / clone constructor fires `acquired` for every borrowed block
  even though nothing was allocated.
- The destructor and `clear()` fire `released` for borrowed blocks even
  though nothing is freed there.
- `detach_impl()` fires `released` for the old (borrowed) pointer and
  `acquired` for the new clone.

This keeps every acquire paired with exactly one release across sharing
boundaries, which is what clients that track block ownership rely on.

## Noncopyable blocks

The copy constructor throws an exception on a noncopyable element block up
front under COW the same way a non-COW instance throws.  Without this, a
copy construction would succeed even when the source contains a
noncopyable element block but later detaching would throw.  Throwing on a
noncopyable element block up front is a way to maintain behavior parity
between COW and non-COW instances.
