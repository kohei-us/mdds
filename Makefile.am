headersdir = $(includedir)/mdds-@API_VERSION@/mdds
pkgconfdir = $(datarootdir)/pkgconfig
docsdir = $(docdir)-@API_VERSION@
htmldir = $(docsdir)/html

AM_CPPFLAGS = -I$(top_srcdir)/include

headers_HEADERS = \
	include/mdds/default_deleter.hpp \
	include/mdds/flat_segment_tree_def.inl \
	include/mdds/flat_segment_tree.hpp \
	include/mdds/flat_segment_tree_itr.hpp \
	include/mdds/global.hpp \
	include/mdds/multi_type_matrix_def.inl \
	include/mdds/multi_type_matrix.hpp \
	include/mdds/multi_type_vector_custom_func1.hpp \
	include/mdds/multi_type_vector_custom_func2.hpp \
	include/mdds/multi_type_vector_custom_func3.hpp \
	include/mdds/multi_type_vector_def.inl \
	include/mdds/multi_type_vector.hpp \
	include/mdds/multi_type_vector_itr.hpp \
	include/mdds/multi_type_vector_macro.hpp \
	include/mdds/multi_type_vector_trait.hpp \
	include/mdds/multi_type_vector_types.hpp \
	include/mdds/node.hpp \
	include/mdds/point_quad_tree.hpp \
	include/mdds/quad_node.hpp \
	include/mdds/rectangle_set_def.inl \
	include/mdds/rectangle_set.hpp \
	include/mdds/segment_tree_def.inl \
	include/mdds/segment_tree.hpp \
	include/mdds/sorted_string_map_def.inl \
	include/mdds/sorted_string_map.hpp \
	include/mdds/trie_map_def.inl \
	include/mdds/trie_map.hpp

dist_docs_DATA = AUTHORS README.md
nodist_docs_DATA = VERSION
nodist_pkgconf_DATA = misc/mdds-@API_VERSION@.pc

DISTCLEANFILES = misc/mdds-@API_VERSION@.pc

EXTRA_DIST = \
	CHANGELOG \
	LICENSE \
	doc/conf.py \
	doc/doxygen.conf \
	doc/flat_segment_tree.rst \
	doc/index.rst \
	doc/multi_type_matrix.rst \
	doc/multi_type_vector.rst \
	doc/packed_trie_map.rst \
	doc/point_quad_tree.rst \
	doc/rectangle_set.rst \
	doc/segment_tree.rst \
	doc/sorted_string_map.rst \
	example/flat_segment_tree.cpp \
	example/Makefile.in \
	example/mixed_type_matrix.cpp \
	example/multi_type_vector.cpp \
	example/point_quad_tree.cpp \
	example/rectangle_set.cpp \
	example/segment_tree.cpp \
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
	multi_type_vector_test_event \
	multi_type_vector_test_custom \
	multi_type_vector_test_default \
	multi_type_vector_test_perf \
	point_quad_tree_test \
	rectangle_set_test \
	segment_tree_test \
	sorted_string_map_test \
	stlperf_test \
	template_test \
	trie_map_test

flat_segment_tree_test_SOURCES = src/flat_segment_tree_test.cpp src/test_global.hpp

multi_type_matrix_test_SOURCES = src/multi_type_matrix_test.cpp src/test_global.hpp

multi_type_vector_test_event_SOURCES = src/multi_type_vector_test_event.cpp src/test_global.hpp

multi_type_vector_test_custom_SOURCES = src/multi_type_vector_test_custom.cpp src/test_global.hpp

multi_type_vector_test_default_SOURCES = src/multi_type_vector_test_default.cpp src/test_global.hpp

multi_type_vector_test_perf_SOURCES = src/multi_type_vector_test_perf.cpp src/test_global.hpp

point_quad_tree_test_SOURCES = src/point_quad_tree_test.cpp src/test_global.hpp

rectangle_set_test_SOURCES = src/rectangle_set_test.cpp src/test_global.hpp

segment_tree_test_SOURCES = src/segment_tree_test.cpp src/test_global.hpp

stlperf_test_SOURCES = src/stlperf_test.cpp

sorted_string_map_test_SOURCES = src/sorted_string_map_test.cpp src/test_global.hpp

template_test_SOURCES = src/template_test.cpp

trie_map_test_SOURCES = src/trie_map_test.cpp src/test_global.hpp

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
	multi_type_vector_test_event \
	multi_type_vector_test_custom \
	multi_type_vector_test_default \
	point_quad_tree_test \
	rectangle_set_test \
	segment_tree_test \
	sorted_string_map_test \
	trie_map_test

# NOTE: AFAIK automake does not support running the same test executable
# in different ways. I workaround this limitation by creating dummy
# test files and redirecting to the real executable inside the wrapper.
if RUN_MEMORY_TESTS
TESTS += \
	flat_segment_tree_test_mem.mem \
	multi_type_matrix_test_mem.mem \
	multi_type_vector_test_event_mem.mem \
	multi_type_vector_test_custom_mem.mem \
	multi_type_vector_test_default_mem.mem \
	point_quad_tree_test_mem.mem \
	rectangle_set_test_mem.mem \
	segment_tree_test_mem.mem \
	sorted_string_map_test_mem.mem \
	trie_map_test_mem.mem
endif

install-data-local:
	$(MKDIR_P) $(DESTDIR)$(docsdir)
	$(INSTALL_DATA) $(top_srcdir)/LICENSE $(DESTDIR)$(docsdir)/COPYING
	$(INSTALL_DATA) $(top_srcdir)/CHANGELOG $(DESTDIR)$(docsdir)/NEWS

uninstall-local:
	rm -f $(DESTDIR)$(docsdir)/COPYING $(DESTDIR)$(docsdir)/NEWS

if BUILD_DOCS
html-local:
	(cd doc && $(DOXYGEN) doxygen.conf)
	$(SPHINX) -b html ./doc/ ./doc/_build
endif
