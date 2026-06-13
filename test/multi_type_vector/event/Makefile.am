# SPDX-FileCopyrightText: 2025 Kohei Yoshida
#
# SPDX-License-Identifier: MIT

AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/test/include \
	$(CXXFLAGS_UNITTESTS)

check_PROGRAMS = test-aos test-soa

test_aos_SOURCES = \
	test_aos.cpp \
	$(top_srcdir)/test/test_global.cpp

test_aos_CPPFLAGS = \
	-I$(srcdir)/tc \
	$(AM_CPPFLAGS)

test_soa_SOURCES = \
	test_soa.cpp \
	$(top_srcdir)/test/test_global.cpp

test_soa_CPPFLAGS = \
	-I$(srcdir)/tc \
	$(AM_CPPFLAGS)

EXTRA_DIST = \
	tc/block_counter.hpp \
	tc/block_init.hpp \
	tc/run.hpp \
	tc/swap.hpp

TESTS = test-aos test-soa

@VALGRIND_CHECK_RULES@
