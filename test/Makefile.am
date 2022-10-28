SUBDIRS = \
	gdb \
	flat_segment_tree \
	include \
	multi_type_vector \
	multi_type_matrix \
	point_quad_tree \
	rtree \
	segment_tree \
	sorted_string_map \
	trie_map

AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/test/include \
	$(CXXFLAGS_UNITTESTS)

check_PROGRAMS = \
	ref-pair-test

ref_pair_test_SOURCES = \
	ref_pair_test.cpp \
	test_global.cpp

TESTS = \
	ref-pair-test

@VALGRIND_CHECK_RULES@
