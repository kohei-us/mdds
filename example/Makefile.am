
AM_CPPFLAGS = -I$(top_srcdir)/include -DNDEBUG

EXTRA_PROGRAMS = \
	flat-segment-tree \
	flat-segment-tree-itrs \
	mtv-collection \
	multi-type-matrix \
	multi-type-vector \
	multi-type-vector-element-block1 \
	multi-type-vector-event1 \
	multi-type-vector-pos-hint \
	packed-trie-map \
	point-quad-tree \
	rectangle-set \
	segment-tree \
	trie-map

flat_segment_tree_SOURCES = flat_segment_tree.cpp
flat_segment_tree_itrs_SOURCES = flat_segment_tree_itrs.cpp
mtv_collection_SOURCES = mtv_collection.cpp
multi_type_matrix_SOURCES = multi_type_matrix.cpp
multi_type_vector_SOURCES = multi_type_vector.cpp
multi_type_vector_element_block1_SOURCES = multi_type_vector_element_block1.cpp
multi_type_vector_event1_SOURCES = multi_type_vector_event1.cpp
multi_type_vector_pos_hint_SOURCES = multi_type_vector_pos_hint.cpp
packed_trie_map_SOURCES = packed_trie_map.cpp
point_quad_tree_SOURCES = point_quad_tree.cpp
rectangle_set_SOURCES = rectangle_set.cpp
segment_tree_SOURCES = segment_tree.cpp
trie_map_SOURCES = trie_map.cpp

TESTS = \
	flat-segment-tree \
	flat-segment-tree-itrs \
	mtv-collection \
	multi-type-matrix \
	multi-type-vector \
	multi-type-vector-element-block1 \
	multi-type-vector-event1 \
	multi-type-vector-pos-hint \
	packed-trie-map \
	point-quad-tree \
	rectangle-set \
	segment-tree \
	trie-map

distclean-local:
	rm -rf $(TESTS)
