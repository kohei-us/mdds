SUBDIRS = include example test tools

pkgconfdir = $(datarootdir)/pkgconfig

AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/test/include \
	$(CXXFLAGS_UNITTESTS)

dist_doc_DATA = AUTHORS README.md
nodist_pkgconf_DATA = misc/mdds-@API_VERSION@.pc

DISTCLEANFILES = \
	misc/mdds-@API_VERSION@.pc \
	rtree-test-*.obj \
	rtree-test-*.svg

EXTRA_DIST = \
	autogen.sh \
	CHANGELOG \
	LICENSE \
	doc/conf.py \
	doc/doxygen.conf \
	doc/flat_segment_tree.rst \
	doc/index.rst \
	doc/multi_type_matrix.rst \
	doc/multi_type_vector.rst \
	doc/point_quad_tree.rst \
	doc/segment_tree.rst \
	doc/sorted_string_map.rst \
	doc/trie_map.rst \
	doc/_static/images/mtv_block_structure.png \
	example/flat_segment_tree.cpp \
	example/multi_type_matrix.cpp \
	example/multi_type_vector.cpp \
	example/multi_type_vector_element_block1.cpp \
	example/multi_type_vector_event1.cpp \
	example/multi_type_vector_pos_hint.cpp \
	example/packed_trie_map.cpp \
	example/point_quad_tree.cpp \
	example/segment_tree.cpp \
	example/trie_map.cpp \
	misc/gdb/mdds/__init__.py \
	misc/matrix_perf.cpp \
	misc/mdds.pc.in \
	misc/sorted_string_map_perf.cpp \
	quickcheck/flat_segment_tree.cpp \
	test/mem-wrapper.sh \
	test/test-wrapper.sh \
	test/test.mem.in

check_PROGRAMS = \
	flat_segment_tree_test \
	multi_type_matrix_test \
	multi_type_matrix_test_walk \
	multi_type_vector_test_event_aos \
	multi_type_vector_test_event_soa \
	multi_type_vector_test_custom_aos \
	multi_type_vector_test_custom_soa \
	multi_type_vector_test_default_aos \
	multi_type_vector_test_default_soa \
	multi_type_vector_test_perf \
	multi_type_vector_test_collection_aos \
	multi_type_vector_test_collection_soa \
	point_quad_tree_test \
	segment_tree_test \
	sorted_string_map_test \
	stlperf_test \
	template_test \
	trie_map_test \
	rtree_test \
	rtree_test_bulkload \
	ref_pair_test

flat_segment_tree_test_SOURCES = \
	test/flat_segment_tree_test.cpp \
	test/include/test_global.hpp \
	test/test_global.cpp

multi_type_matrix_test_SOURCES = \
	test/multi_type_matrix_test.cpp \
	test/include/test_global.hpp \
	test/test_global.cpp

multi_type_matrix_test_walk_SOURCES = \
	test/multi_type_matrix_test_walk.cpp \
	test/include/test_global.hpp \
	test/test_global.cpp

multi_type_vector_test_event_aos_SOURCES = \
	test/multi_type_vector/event/aos/test_main.hpp \
	test/multi_type_vector/event/aos/test_main.cpp \
	test/multi_type_vector/event/aos/test_block_counter.cpp \
	test/multi_type_vector/event/aos/test_block_init.cpp \
	test/multi_type_vector/event/aos/test_swap.cpp \
	test/multi_type_vector/event/tc/block_counter.inl \
	test/multi_type_vector/event/tc/block_init.inl \
	test/multi_type_vector/event/tc/swap.inl \
	test/include/test_global.hpp \
	test/test_global.cpp

multi_type_vector_test_event_aos_CPPFLAGS = \
	-I$(top_srcdir)/test/multi_type_vector/event \
	-I$(top_srcdir)/test/multi_type_vector/event/tc \
	$(AM_CPPFLAGS)

multi_type_vector_test_event_soa_SOURCES = \
	test/multi_type_vector/event/soa/test_main.hpp \
	test/multi_type_vector/event/soa/test_main.cpp \
	test/multi_type_vector/event/soa/test_block_counter.cpp \
	test/multi_type_vector/event/soa/test_block_init.cpp \
	test/multi_type_vector/event/aos/test_swap.cpp \
	test/include/test_global.hpp \
	test/test_global.cpp

multi_type_vector_test_event_soa_CPPFLAGS = \
	-I$(top_srcdir)/test/multi_type_vector/event \
	-I$(top_srcdir)/test/multi_type_vector/event/tc \
	$(AM_CPPFLAGS)

multi_type_vector_test_custom_aos_SOURCES = \
	test/multi_type_vector/custom/aos/test_main.hpp \
	test/multi_type_vector/custom/aos/test_main.cpp \
	test/multi_type_vector/custom/aos/test_basic.cpp \
	test/multi_type_vector/custom/aos/test_managed_block.cpp \
	test/multi_type_vector/custom/aos/test_misc.cpp \
	test/multi_type_vector/custom/aos/test_swap.cpp \
	test/multi_type_vector/custom/aos/test_transfer.cpp \
	test/multi_type_vector/custom/tc/basic.inl \
	test/multi_type_vector/custom/tc/managed_block.inl \
	test/multi_type_vector/custom/tc/misc.inl \
	test/multi_type_vector/custom/tc/swap.inl \
	test/multi_type_vector/custom/tc/transfer.inl \
	test/multi_type_vector/custom/common_types.hpp \
	test/include/test_global.hpp \
	test/test_global.cpp

multi_type_vector_test_custom_aos_CPPFLAGS = \
	-I$(top_srcdir)/test/multi_type_vector/custom \
	-I$(top_srcdir)/test/multi_type_vector/custom/tc \
	$(AM_CPPFLAGS)

multi_type_vector_test_custom_soa_SOURCES = \
	test/multi_type_vector/custom/soa/test_main.hpp \
	test/multi_type_vector/custom/soa/test_main.cpp \
	test/multi_type_vector/custom/soa/test_basic.cpp \
	test/multi_type_vector/custom/soa/test_managed_block.cpp \
	test/multi_type_vector/custom/soa/test_misc.cpp \
	test/multi_type_vector/custom/soa/test_swap.cpp \
	test/multi_type_vector/custom/soa/test_transfer.cpp \
	test/multi_type_vector/custom/common_types.hpp \
	test/include/test_global.hpp \
	test/test_global.cpp

multi_type_vector_test_custom_soa_CPPFLAGS = \
	-I$(top_srcdir)/test/multi_type_vector/custom \
	-I$(top_srcdir)/test/multi_type_vector/custom/tc \
	$(AM_CPPFLAGS)

multi_type_vector_test_default_aos_SOURCES = \
	test/multi_type_vector/default/aos/test_construction.cpp \
	test/multi_type_vector/default/aos/test_basic.cpp \
	test/multi_type_vector/default/aos/test_empty_cells.cpp \
	test/multi_type_vector/default/aos/test_erase.cpp \
	test/multi_type_vector/default/aos/test_hints.cpp \
	test/multi_type_vector/default/aos/test_insert.cpp \
	test/multi_type_vector/default/aos/test_iterators.cpp \
	test/multi_type_vector/default/aos/test_iterators_insert.cpp \
	test/multi_type_vector/default/aos/test_iterators_set.cpp \
	test/multi_type_vector/default/aos/test_iterators_set_empty.cpp \
	test/multi_type_vector/default/aos/test_misc.cpp \
	test/multi_type_vector/default/aos/test_position.cpp \
	test/multi_type_vector/default/aos/test_set.cpp \
	test/multi_type_vector/default/aos/test_swap_range.cpp \
	test/multi_type_vector/default/aos/test_transfer.cpp \
	test/multi_type_vector/default/aos/test_main.hpp \
	test/multi_type_vector/default/aos/test_main.cpp \
	test/multi_type_vector/default/tc/construction.inl \
	test/multi_type_vector/default/tc/basic.inl \
	test/multi_type_vector/default/tc/empty_cells.inl \
	test/multi_type_vector/default/tc/erase.inl \
	test/multi_type_vector/default/tc/hints.inl \
	test/multi_type_vector/default/tc/insert.inl \
	test/multi_type_vector/default/tc/iterators.inl \
	test/multi_type_vector/default/tc/iterators_insert.inl \
	test/multi_type_vector/default/tc/iterators_set.inl \
	test/multi_type_vector/default/tc/iterators_set_empty.inl \
	test/multi_type_vector/default/tc/misc.inl \
	test/multi_type_vector/default/tc/position.inl \
	test/multi_type_vector/default/tc/set.inl \
	test/multi_type_vector/default/tc/swap_range.inl \
	test/multi_type_vector/default/tc/transfer.inl \
	test/include/test_global.hpp \
	test/test_global.cpp

multi_type_vector_test_default_aos_CPPFLAGS = \
	-I$(top_srcdir)/test/multi_type_vector/default/tc \
	$(AM_CPPFLAGS)

multi_type_vector_test_default_soa_SOURCES = \
	test/multi_type_vector/default/soa/test_main.hpp \
	test/multi_type_vector/default/soa/test_main.cpp \
	test/multi_type_vector/default/soa/test_construction.cpp \
	test/multi_type_vector/default/soa/test_basic.cpp \
	test/multi_type_vector/default/soa/test_empty_cells.cpp \
	test/multi_type_vector/default/soa/test_erase.cpp \
	test/multi_type_vector/default/soa/test_hints.cpp \
	test/multi_type_vector/default/soa/test_insert.cpp \
	test/multi_type_vector/default/soa/test_iterators.cpp \
	test/multi_type_vector/default/soa/test_iterators_insert.cpp \
	test/multi_type_vector/default/soa/test_iterators_set.cpp \
	test/multi_type_vector/default/soa/test_iterators_set_empty.cpp \
	test/multi_type_vector/default/soa/test_misc.cpp \
	test/multi_type_vector/default/soa/test_position.cpp \
	test/multi_type_vector/default/soa/test_set.cpp \
	test/multi_type_vector/default/soa/test_swap_range.cpp \
	test/multi_type_vector/default/soa/test_transfer.cpp \
	test/include/test_global.hpp \
	test/test_global.cpp

multi_type_vector_test_default_soa_CPPFLAGS = \
	-I$(top_srcdir)/test/multi_type_vector/default/tc \
	$(AM_CPPFLAGS)

multi_type_vector_test_perf_SOURCES = \
	test/multi_type_vector/perf/test_main.cpp \
	test/include/test_global.hpp \
	test/test_global.cpp

multi_type_vector_test_collection_aos_SOURCES = \
	test/multi_type_vector/collection/aos/test_main.cpp \
	test/multi_type_vector/collection/tc/all.inl \
	test/include/test_global.hpp \
	test/test_global.cpp

multi_type_vector_test_collection_aos_CPPFLAGS = \
	-I$(top_srcdir)/test/multi_type_vector/collection/tc \
	$(AM_CPPFLAGS)

multi_type_vector_test_collection_soa_SOURCES = \
	test/multi_type_vector/collection/soa/test_main.cpp \
	test/multi_type_vector/collection/tc/all.inl \
	test/include/test_global.hpp \
	test/test_global.cpp

multi_type_vector_test_collection_soa_CPPFLAGS = \
	-I$(top_srcdir)/test/multi_type_vector/collection/tc \
	$(AM_CPPFLAGS)

point_quad_tree_test_SOURCES = \
	test/point_quad_tree_test.cpp \
	test/include/test_global.hpp \
	test/test_global.cpp

segment_tree_test_SOURCES = \
	test/segment_tree_test.cpp \
	test/include/test_global.hpp \
	test/test_global.cpp

stlperf_test_SOURCES = test/stlperf_test.cpp

sorted_string_map_test_SOURCES = \
	test/sorted_string_map_test.cpp \
	test/include/test_global.hpp \
	test/test_global.cpp

template_test_SOURCES = test/template_test.cpp

trie_map_test_SOURCES = \
	test/trie_map_test.cpp \
	test/include/test_global.hpp \
	test/test_global.cpp

rtree_test_SOURCES = \
	test/rtree/test_main.cpp \
	test/rtree/test_basic.cpp \
	test/rtree/test_copy.cpp \
	test/rtree/test_erase_directories.cpp \
	test/rtree/test_node_split.cpp \
	test/rtree/test_directory_node_split.cpp \
	test/rtree/test_intersection.cpp \
	test/rtree/test_move.cpp \
	test/rtree/test_square_distance.cpp \
	test/rtree/test_geometry.cpp \
	test/rtree/test_exact_search_by_extent.cpp \
	test/rtree/test_exact_search_by_point.cpp \
	test/rtree/test_forced_reinsertion.cpp \
	test/rtree/test_point_objects.cpp \
	test/rtree/test_global_rtree.hpp \
	test/include/test_global.hpp \
	test/test_global.cpp

rtree_test_bulkload_SOURCES = \
	test/rtree/test_bulkload_main.cpp \
	test/rtree/test_global_rtree.hpp \
	test/include/test_global.hpp \
	test/test_global.cpp

ref_pair_test_SOURCES = test/ref_pair_test.cpp test/test_global.cpp

test.fst.perf: flat_segment_tree_test
	./flat_segment_tree_test perf

test.st.perf: segment_tree_test
	./segment_tree_test perf

test.mtv.perf: multi_type_vector_test_perf
	./multi_type_vector_test_perf

test.mtmatrix.perf: multi_type_matrix_test
	./multi_type_matrix_test perf

test.stl: stlperf_test
	./stlperf_test

TEST_EXTENSIONS = .mem
LOG_COMPILER = $(top_srcdir)/test/test-wrapper.sh
MEM_LOG_COMPILER = $(top_srcdir)/test/mem-wrapper.sh
AM_TESTS_ENVIRONMENT = \
	VALGRIND="$(VALGRIND)"; export VALGRIND; \
	VALGRINDFLAGS="$(VALGRINDFLAGS)"; export VALGRINDFLAGS; \
	EXEEXT="$(EXEEXT)"; export EXEEXT;

TESTS = \
	flat_segment_tree_test \
	multi_type_matrix_test \
	multi_type_matrix_test_walk \
	multi_type_vector_test_event_aos \
	multi_type_vector_test_event_soa \
	multi_type_vector_test_custom_aos \
	multi_type_vector_test_custom_soa \
	multi_type_vector_test_default_aos \
	multi_type_vector_test_default_soa \
	multi_type_vector_test_collection_aos \
	multi_type_vector_test_collection_soa \
	point_quad_tree_test \
	segment_tree_test \
	sorted_string_map_test \
	trie_map_test \
	rtree_test \
	rtree_test_bulkload \
	ref_pair_test

# NOTE: AFAIK automake does not support running the same test executable
# in different ways. I workaround this limitation by creating dummy
# test files and redirecting to the real executable inside the wrapper.
if RUN_MEMORY_TESTS
TESTS += \
	flat_segment_tree_test_mem.mem \
	multi_type_matrix_test_mem.mem \
	multi_type_matrix_test_walk_mem.mem \
	multi_type_vector_test_event_aos_mem.mem \
	multi_type_vector_test_event_soa_mem.mem \
	multi_type_vector_test_custom_aos_mem.mem \
	multi_type_vector_test_custom_soa_mem.mem \
	multi_type_vector_test_default_aos_mem.mem \
	multi_type_vector_test_default_soa_mem.mem \
	multi_type_vector_test_collection_aos_mem.mem \
	multi_type_vector_test_collection_soa_mem.mem \
	point_quad_tree_test_mem.mem \
	segment_tree_test_mem.mem \
	sorted_string_map_test_mem.mem \
	trie_map_test_mem.mem \
	rtree_test_mem.mem \
	rtree_test_bulkload_mem.mem
endif

install-data-local:
	$(MKDIR_P) $(DESTDIR)$(docdir)
	$(INSTALL_DATA) $(top_srcdir)/LICENSE $(DESTDIR)$(docdir)/COPYING
	$(INSTALL_DATA) $(top_srcdir)/CHANGELOG $(DESTDIR)$(docdir)/NEWS

uninstall-local:
	rm -f $(DESTDIR)$(docdir)/COPYING $(DESTDIR)$(docdir)/NEWS

if BUILD_DOCS

doc-doxygen:
	@echo "Building documentation by doxygen..."
	@cd doc && $(DOXYGEN) doxygen.conf

doc-sphinx:
	@echo "Building documentation by sphinx..."
	@$(SPHINX) -b html ./doc/ ./doc/_build

doc: doc-doxygen doc-sphinx

endif

check-gdb:
	make -C test/gdb check
