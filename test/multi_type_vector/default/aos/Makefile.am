AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/test/include \
	$(CXXFLAGS_UNITTESTS)

check_PROGRAMS = test-main

test_main_SOURCES = \
	test_construction.cpp \
	test_basic.cpp \
	test_empty_cells.cpp \
	test_erase.cpp \
	test_hints.cpp \
	test_insert.cpp \
	test_iterators.cpp \
	test_iterators_insert.cpp \
	test_iterators_set.cpp \
	test_iterators_set_empty.cpp \
	test_misc.cpp \
	test_position.cpp \
	test_set.cpp \
	test_swap_range.cpp \
	test_transfer.cpp \
	test_main.cpp \
	$(top_srcdir)/test/test_global.cpp

test_main_CPPFLAGS = \
	-I$(srcdir)/.. \
	-I$(srcdir)/../tc \
	$(AM_CPPFLAGS)

EXTRA_DIST = test_main.hpp

TESTS = test-main

@VALGRIND_CHECK_RULES@
