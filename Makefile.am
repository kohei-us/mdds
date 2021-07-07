SUBDIRS = include example test

pkgconfdir = $(datarootdir)/pkgconfig

AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/src/include \
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
	src/mem-wrapper.sh \
	src/test-wrapper.sh \
	src/test.mem.in

check_PROGRAMS = \
	flat_segment_tree_test \
	multi_type_matrix_test \
	multi_type_matrix_test_walk \
	multi_type_vector_test_event \
	multi_type_vector_test_custom \
	multi_type_vector_test_default \
	multi_type_vector_test_default_soa \
	multi_type_vector_test_perf \
	multi_type_vector_test_collection \
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
	src/flat_segment_tree_test.cpp \
	src/include/test_global.hpp \
	src/test_global.cpp

multi_type_matrix_test_SOURCES = \
	src/multi_type_matrix_test.cpp \
	src/include/test_global.hpp \
	src/test_global.cpp

multi_type_matrix_test_walk_SOURCES = \
	src/multi_type_matrix_test_walk.cpp \
	src/include/test_global.hpp \
	src/test_global.cpp

multi_type_vector_test_event_SOURCES = \
	src/multi_type_vector/event/test_main.cpp \
	src/include/test_global.hpp \
	src/test_global.cpp

multi_type_vector_test_custom_SOURCES = \
	src/multi_type_vector/custom/test_main.cpp \
	src/include/test_global.hpp \
	src/test_global.cpp

multi_type_vector_test_default_SOURCES = \
	src/multi_type_vector/default/test_construction.cpp \
	src/multi_type_vector/default/test_basic.cpp \
	src/multi_type_vector/default/test_empty_cells.cpp \
	src/multi_type_vector/default/test_erase.cpp \
	src/multi_type_vector/default/test_insert.cpp \
	src/multi_type_vector/default/test_misc.cpp \
	src/multi_type_vector/default/test_set.cpp \
	src/multi_type_vector/default/test_main.hpp \
	src/multi_type_vector/default/test_main.cpp \
	src/multi_type_vector/default/tc/construction.inl \
	src/multi_type_vector/default/tc/basic.inl \
	src/multi_type_vector/default/tc/empty_cells.inl \
	src/multi_type_vector/default/tc/erase.inl \
	src/multi_type_vector/default/tc/insert.inl \
	src/multi_type_vector/default/tc/misc.inl \
	src/multi_type_vector/default/tc/set.inl \
	src/include/test_global.hpp \
	src/test_global.cpp

multi_type_vector_test_default_CPPFLAGS = \
	-I$(top_srcdir)/src/multi_type_vector/default/tc \
	$(AM_CPPFLAGS)

multi_type_vector_test_default_soa_SOURCES = \
	src/multi_type_vector/default/soa/test_main.hpp \
	src/multi_type_vector/default/soa/test_main.cpp \
	src/multi_type_vector/default/soa/test_construction.cpp \
	src/multi_type_vector/default/soa/test_basic.cpp \
	src/multi_type_vector/default/soa/test_empty_cells.cpp \
	src/multi_type_vector/default/soa/test_erase.cpp \
	src/multi_type_vector/default/soa/test_insert.cpp \
	src/multi_type_vector/default/soa/test_misc.cpp \
	src/multi_type_vector/default/soa/test_set.cpp \
	src/include/test_global.hpp \
	src/test_global.cpp

multi_type_vector_test_default_soa_CPPFLAGS = \
	-I$(top_srcdir)/src/multi_type_vector/default/tc \
	$(AM_CPPFLAGS)

multi_type_vector_test_perf_SOURCES = \
	src/multi_type_vector/perf/test_main.cpp \
	src/include/test_global.hpp \
	src/test_global.cpp

multi_type_vector_test_collection_SOURCES = \
	src/multi_type_vector/collection/test_main.cpp \
	src/include/test_global.hpp \
	src/test_global.cpp

point_quad_tree_test_SOURCES = \
	src/point_quad_tree_test.cpp \
	src/include/test_global.hpp \
	src/test_global.cpp

segment_tree_test_SOURCES = \
	src/segment_tree_test.cpp \
	src/include/test_global.hpp \
	src/test_global.cpp

stlperf_test_SOURCES = src/stlperf_test.cpp

sorted_string_map_test_SOURCES = \
	src/sorted_string_map_test.cpp \
	src/include/test_global.hpp \
	src/test_global.cpp

template_test_SOURCES = src/template_test.cpp

trie_map_test_SOURCES = \
	src/trie_map_test.cpp \
	src/include/test_global.hpp \
	src/test_global.cpp

rtree_test_SOURCES = \
	src/rtree/test_main.cpp \
	src/rtree/test_basic.cpp \
	src/rtree/test_copy.cpp \
	src/rtree/test_erase_directories.cpp \
	src/rtree/test_node_split.cpp \
	src/rtree/test_directory_node_split.cpp \
	src/rtree/test_intersection.cpp \
	src/rtree/test_move.cpp \
	src/rtree/test_square_distance.cpp \
	src/rtree/test_geometry.cpp \
	src/rtree/test_exact_search_by_extent.cpp \
	src/rtree/test_exact_search_by_point.cpp \
	src/rtree/test_forced_reinsertion.cpp \
	src/rtree/test_point_objects.cpp \
	src/rtree/test_global_rtree.hpp \
	src/include/test_global.hpp \
	src/test_global.cpp

rtree_test_bulkload_SOURCES = \
	src/rtree/test_bulkload_main.cpp \
	src/test_global_rtree.hpp \
	src/include/test_global.hpp \
	src/test_global.cpp

ref_pair_test_SOURCES = src/ref_pair_test.cpp src/test_global.cpp

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
LOG_COMPILER = $(top_srcdir)/src/test-wrapper.sh
MEM_LOG_COMPILER = $(top_srcdir)/src/mem-wrapper.sh
AM_TESTS_ENVIRONMENT = \
	VALGRIND="$(VALGRIND)"; export VALGRIND; \
	VALGRINDFLAGS="$(VALGRINDFLAGS)"; export VALGRINDFLAGS; \
	EXEEXT="$(EXEEXT)"; export EXEEXT;

TESTS = \
	flat_segment_tree_test \
	multi_type_matrix_test \
	multi_type_matrix_test_walk \
	multi_type_vector_test_event \
	multi_type_vector_test_custom \
	multi_type_vector_test_default \
	multi_type_vector_test_default_soa \
	multi_type_vector_test_collection \
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
	multi_type_vector_test_event_mem.mem \
	multi_type_vector_test_custom_mem.mem \
	multi_type_vector_test_default_mem.mem \
	multi_type_vector_test_collection_mem.mem \
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
