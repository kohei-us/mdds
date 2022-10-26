AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/test/include \
	$(CXXFLAGS_UNITTESTS)

DISTCLEANFILES = \
	rtree-test-*.obj \
	rtree-test-*.svg

check_PROGRAMS = test-main test-bulkload

test_main_SOURCES = \
	test_main.cpp \
	$(top_srcdir)/test/test_global.cpp

test_main_SOURCES = \
	test_basic.cpp \
	test_copy.cpp \
	test_directory_node_split.cpp \
	test_erase_directories.cpp \
	test_exact_search_by_extent.cpp \
	test_exact_search_by_point.cpp \
	test_forced_reinsertion.cpp \
	test_geometry.cpp \
	test_intersection.cpp \
	test_main.cpp \
	test_move.cpp \
	test_node_split.cpp \
	test_point_objects.cpp \
	test_square_distance.cpp \
	$(top_srcdir)/test/test_global.cpp

test_bulkload_SOURCES = \
	test_bulkload_main.cpp \
	$(top_srcdir)/test/test_global.cpp

EXTRA_DIST = test_global_rtree.hpp

TESTS = test-main test-bulkload

@VALGRIND_CHECK_RULES@
