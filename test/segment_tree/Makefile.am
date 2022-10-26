AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/test/include \
	$(CXXFLAGS_UNITTESTS)

check_PROGRAMS = test-main

test_main_SOURCES = \
	test_main.cpp \
	$(top_srcdir)/test/test_global.cpp

TESTS = test-main

@VALGRIND_CHECK_RULES@
