
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
	packed-trie-state-custom \
	packed-trie-state-int \
	point-quad-tree \
	rectangle-set \
	segment-tree \
	trie-map \
	rtree-simple \
	rtree-erase \
	rtree-medium \
	rtree-medium-bulkload

flat_segment_tree_SOURCES = flat_segment_tree.cpp
flat_segment_tree_itrs_SOURCES = flat_segment_tree_itrs.cpp
mtv_collection_SOURCES = mtv_collection.cpp
multi_type_matrix_SOURCES = multi_type_matrix.cpp
multi_type_vector_SOURCES = multi_type_vector.cpp
multi_type_vector_element_block1_SOURCES = multi_type_vector_element_block1.cpp
multi_type_vector_event1_SOURCES = multi_type_vector_event1.cpp
multi_type_vector_pos_hint_SOURCES = multi_type_vector_pos_hint.cpp
packed_trie_map_SOURCES = packed_trie_map.cpp
packed_trie_state_custom_SOURCES = packed_trie_state_custom.cpp
packed_trie_state_int_SOURCES = packed_trie_state_int.cpp
point_quad_tree_SOURCES = point_quad_tree.cpp
rectangle_set_SOURCES = rectangle_set.cpp
segment_tree_SOURCES = segment_tree.cpp
trie_map_SOURCES = trie_map.cpp
rtree_simple_SOURCES = rtree_simple.cpp
rtree_erase_SOURCES = rtree_erase.cpp
rtree_medium_SOURCES = rtree_medium.cpp
rtree_medium_bulkload_SOURCES = rtree_medium_bulkload.cpp

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
	packed-trie-state-custom \
	packed-trie-state-int \
	point-quad-tree \
	rectangle-set \
	segment-tree \
	trie-map \
	rtree-simple \
	rtree-erase \
	rtree-medium \
	rtree-medium-bulkload

TEST_ARTIFACTS = \
	*.svg \
	us-presidents.bin \
	cities.bin

distclean-local:
	rm -f $(TESTS) $(TEST_ARTIFACTS)

