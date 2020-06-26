SUBDIRS = include example test

pkgconfdir = $(datarootdir)/pkgconfig

AM_CPPFLAGS = -I$(top_srcdir)/include $(CXXFLAGS_UNITTESTS)

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
	doc/rectangle_set.rst \
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
	example/rectangle_set.cpp \
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
	multi_type_vector_test_perf \
	multi_type_vector_test_collection \
	point_quad_tree_test \
	rectangle_set_test \
	segment_tree_test \
	sorted_string_map_test \
	stlperf_test \
	template_test \
	trie_map_test \
	rtree_test \
	rtree_test_bulkload \
	ref_pair_test

flat_segment_tree_test_SOURCES = src/flat_segment_tree_test.cpp src/test_global.hpp

multi_type_matrix_test_SOURCES = src/multi_type_matrix_test.cpp src/test_global.hpp

multi_type_matrix_test_walk_SOURCES = src/multi_type_matrix_test_walk.cpp src/test_global.hpp

multi_type_vector_test_event_SOURCES = src/multi_type_vector_test_event.cpp src/test_global.hpp

multi_type_vector_test_custom_SOURCES = src/multi_type_vector_test_custom.cpp src/test_global.hpp

multi_type_vector_test_default_SOURCES = src/multi_type_vector_test_default.cpp src/test_global.hpp

multi_type_vector_test_perf_SOURCES = src/multi_type_vector_test_perf.cpp src/test_global.hpp

multi_type_vector_test_collection_SOURCES = src/multi_type_vector_test_collection.cpp src/test_global.hpp

point_quad_tree_test_SOURCES = src/point_quad_tree_test.cpp src/test_global.hpp

rectangle_set_test_SOURCES = src/rectangle_set_test.cpp src/test_global.hpp

segment_tree_test_SOURCES = src/segment_tree_test.cpp src/test_global.hpp

stlperf_test_SOURCES = src/stlperf_test.cpp

sorted_string_map_test_SOURCES = src/sorted_string_map_test.cpp src/test_global.hpp

template_test_SOURCES = src/template_test.cpp

trie_map_test_SOURCES = src/trie_map_test.cpp src/test_global.hpp

rtree_test_SOURCES = src/rtree_test.cpp src/test_global.hpp src/test_global_rtree.hpp

rtree_test_bulkload_SOURCES = src/rtree_test_bulkload.cpp src/test_global.hpp src/test_global_rtree.hpp

ref_pair_test_SOURCES = src/ref_pair_test.cpp

test.fst.perf: flat_segment_tree_test
	./flat_segment_tree_test perf

test.recset.perf: rectangle_set_test
	./rectangle_set_test perf

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
	multi_type_vector_test_collection \
	point_quad_tree_test \
	rectangle_set_test \
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
	rectangle_set_test_mem.mem \
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
