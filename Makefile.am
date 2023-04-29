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
	doc/point_quad_tree.rst \
	doc/rtree.rst \
	doc/api.rst \
	doc/index.rst \
	doc/flat_segment_tree.rst \
	doc/segment_tree.rst \
	doc/multi_type_matrix.rst \
	doc/trie_map.rst \
	doc/sorted_string_map.rst \
	doc/global.rst \
	doc/multi_type_vector/index.rst \
	doc/multi_type_vector/examples.rst \
	doc/multi_type_vector/api-ref.rst \
	doc/_static/images/mtv-block-structure.svg \
	doc/_static/images/rtree_bounds_src.png \
	doc/_static/images/mtv_collection_sheet.png \
	doc/_static/images/rtree_bounds2_tree_bulkload.png \
	doc/_static/images/rtree_bounds2_tree.png \
	doc/_static/images/fst-example1-initial.svg \
	doc/_static/images/fst-example1-insert2.svg \
	doc/_static/images/rtree_bounds_tree.png \
	doc/_static/images/fst-example1-insert1.svg \
	example/rtree_simple.cpp \
	example/packed_trie_state_int.cpp \
	example/multi_type_matrix.cpp \
	example/rtree_erase.cpp \
	example/packed_trie_map.cpp \
	example/trie_map.cpp \
	example/rtree_medium_bulkload.cpp \
	example/packed_trie_state_custom.cpp \
	example/segment_tree.cpp \
	example/point_quad_tree.cpp \
	example/flat_segment_tree_itrs.cpp \
	example/multi_type_vector/pos_hint.cpp \
	example/multi_type_vector/basic.cpp \
	example/multi_type_vector/debug_trace.cpp \
	example/multi_type_vector/custom_value_types.cpp \
	example/multi_type_vector/custom_value_types_custom_store.cpp \
	example/multi_type_vector/mtv_collection.cpp \
	example/multi_type_vector/standard_custom_store.cpp \
	example/multi_type_vector/event1.cpp \
	example/multi_type_vector/element_block1.cpp \
	example/rtree_medium.cpp \
	example/flat_segment_tree.cpp \
	misc/gdb/mdds/__init__.py \
	misc/matrix_perf.cpp \
	misc/mdds.pc.in \
	misc/sorted_string_map_perf.cpp \
	quickcheck/flat_segment_tree.cpp

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
