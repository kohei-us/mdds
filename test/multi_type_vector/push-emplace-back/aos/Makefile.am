AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/test/include \
	$(CXXFLAGS_UNITTESTS)

check_PROGRAMS = test-main

test_main_SOURCES = \
	test_main.cpp \
	test_push_back.cpp \
	$(top_srcdir)/test/test_global.cpp

test_main_CPPFLAGS = \
	-I$(srcdir)/.. \
	-I$(srcdir)/../tc \
	$(AM_CPPFLAGS)

EXTRA_DIST = test_main.hpp

TESTS = test-main

@VALGRIND_CHECK_RULES@
