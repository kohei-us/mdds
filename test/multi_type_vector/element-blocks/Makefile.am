
AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/test/include

check_PROGRAMS = test test-soa test-aos

test_SOURCES = \
	test_main.cpp \
	test_basic.cpp \
	test_std_vector_bool.cpp \
	test_store_types.cpp \
	test_functions.cpp \
	$(top_srcdir)/test/test_global.cpp

test_soa_SOURCES = \
	test_soa.cpp \
	test_no_standard_blocks_soa.cpp \
	$(top_srcdir)/test/test_global.cpp

test_aos_SOURCES = \
	test_aos.cpp \
	test_no_standard_blocks_aos.cpp \
	$(top_srcdir)/test/test_global.cpp

EXTRA_DIST = \
	no_standard_blocks_defs.inl \
	no_standard_blocks_funcs.inl \
	test_soa.hpp \
	test_aos.hpp \
	test_main.hpp

TESTS = test test-soa test-aos

@VALGRIND_CHECK_RULES@
