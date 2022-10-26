SUBDIRS = include example test tools

pkgconfdir = $(datarootdir)/pkgconfig

AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/test/include \
	$(CXXFLAGS_UNITTESTS)

dist_doc_DATA = AUTHORS README.md
nodist_pkgconf_DATA = misc/mdds-@API_VERSION@.pc

DISTCLEANFILES = \
	misc/mdds-@API_VERSION@.pc

EXTRA_DIST = \
	autogen.sh \
	CHANGELOG \
	LICENSE \
	doc/conf.py \
	doc/doxygen.conf \
	doc/flat_segment_tree.rst \
	doc/index.rst \
	doc/multi_type_matrix.rst \
	doc/multi_type_vector.rst \
	doc/point_quad_tree.rst \
	doc/segment_tree.rst \
	doc/sorted_string_map.rst \
	doc/trie_map.rst \
	doc/_static/images/mtv_block_structure.png \
	example/flat_segment_tree.cpp \
	example/multi_type_matrix.cpp \
	example/multi_type_vector.cpp \
	example/multi_type_vector_element_block1.cpp \
	example/multi_type_vector_event1.cpp \
	example/multi_type_vector_pos_hint.cpp \
	example/packed_trie_map.cpp \
	example/point_quad_tree.cpp \
	example/segment_tree.cpp \
	example/trie_map.cpp \
	misc/gdb/mdds/__init__.py \
	misc/matrix_perf.cpp \
	misc/mdds.pc.in \
	misc/sorted_string_map_perf.cpp \
	quickcheck/flat_segment_tree.cpp

check_PROGRAMS = \
	ref_pair_test

ref_pair_test_SOURCES = test/ref_pair_test.cpp test/test_global.cpp

TESTS = \
	ref_pair_test

@VALGRIND_CHECK_RULES@

install-data-local:
	$(MKDIR_P) $(DESTDIR)$(docdir)
	$(INSTALL_DATA) $(top_srcdir)/LICENSE $(DESTDIR)$(docdir)/COPYING
	$(INSTALL_DATA) $(top_srcdir)/CHANGELOG $(DESTDIR)$(docdir)/NEWS

uninstall-local:
	rm -f $(DESTDIR)$(docdir)/COPYING $(DESTDIR)$(docdir)/NEWS

if BUILD_DOCS

doc-doxygen:
	@echo "Building documentation by doxygen..."
	@cd doc && $(DOXYGEN) doxygen.conf

doc-sphinx:
	@echo "Building documentation by sphinx..."
	@$(SPHINX) -b html ./doc/ ./doc/_build

doc: doc-doxygen doc-sphinx

endif

check-gdb:
	make -C test/gdb check
