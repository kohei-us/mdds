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
	tc/basic.hpp \
	tc/common.hpp \
	tc/construction.hpp \
	tc/empty_cells.hpp \
	tc/erase.hpp \
	tc/hints.hpp \
	tc/insert.hpp \
	tc/iterators.hpp \
	tc/iterators_insert.hpp \
	tc/iterators_set.hpp \
	tc/iterators_set_empty.hpp \
	tc/misc.hpp \
	tc/position.hpp \
	tc/run.hpp \
	tc/set.hpp \
	tc/swap_range.hpp \
	tc/transfer.hpp

TESTS = test-aos test-soa

@VALGRIND_CHECK_RULES@
