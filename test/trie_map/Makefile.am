AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/test/include \
	$(CXXFLAGS_UNITTESTS)

check_PROGRAMS = test-main

EXTRA_DIST = \
	test_main.hpp

test_main_SOURCES = \
	test_main.cpp \
	test_state.cpp \
	test_node.cpp \
	test_move_value.cpp \
	test_pack_value_type.cpp \
	$(top_srcdir)/test/test_global.cpp

TESTS = test-main

@VALGRIND_CHECK_RULES@
