
AM_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/test/include \
    -I$(srcdir)/../tc

check_PROGRAMS = test

test_SOURCES = \
    test_main.hpp \
    test_main.cpp \
    test_loop_unrolling.cpp \
    test_loop_unrolling_simd.cpp \
    $(top_srcdir)/test/test_global.cpp

TESTS = test
