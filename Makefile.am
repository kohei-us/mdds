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
	doc/_static/images/fst-example1-initial.svg \
	doc/_static/images/fst-example1-insert1.svg \
	doc/_static/images/fst-example1-insert2.svg \
	doc/_static/images/mtv-block-structure.svg \
	doc/_static/images/mtv_collection_sheet.png \
	doc/_static/images/rtree_bounds2_tree.png \
	doc/_static/images/rtree_bounds2_tree_bulkload.png \
	doc/_static/images/rtree_bounds_src.png \
	doc/_static/images/rtree_bounds_tree.png \
	doc/api.rst \
	doc/conf.py \
	doc/doxygen.conf \
	doc/environment.yml \
	doc/figures.odg \
	doc/flat_segment_tree/api.rst \
	doc/flat_segment_tree/index.rst \
	doc/flat_segment_tree/iterate-segments.rst \
	doc/flat_segment_tree/quickstart.rst \
	doc/global.rst \
	doc/index.rst \
	doc/multi_type_matrix.rst \
	doc/multi_type_vector/api-ref.rst \
	doc/multi_type_vector/examples.rst \
	doc/multi_type_vector/index.rst \
	doc/point_quad_tree.rst \
	doc/requirements.txt \
	doc/rtree.rst \
	doc/segment_tree/api.rst \
	doc/segment_tree/index.rst \
	doc/segment_tree/perf.rst \
	doc/segment_tree/quickstart.rst \
	doc/segment_tree/remove-segments.rst \
	doc/sorted_string_map/api.rst \
	doc/sorted_string_map/index.rst \
	doc/sorted_string_map/quickstart.rst \
	doc/trie_map/api.rst \
	doc/trie_map/direct-packed-trie-map.rst \
	doc/trie_map/index.rst \
	doc/trie_map/pack-trie-map.rst \
	doc/trie_map/populate-trie-map.rst \
	doc/trie_map/save-load-packed-trie-map-custom-value.rst \
	doc/trie_map/save-load-packed-trie-map.rst \
	example/Makefile.am \
	example/flat_segment_tree.cpp \
	example/flat_segment_tree_itrs.cpp \
	example/multi_type_matrix.cpp \
	example/multi_type_vector/Makefile.am \
	example/multi_type_vector/basic.cpp \
	example/multi_type_vector/custom_value_types.cpp \
	example/multi_type_vector/custom_value_types_custom_store.cpp \
	example/multi_type_vector/debug_trace.cpp \
	example/multi_type_vector/element_block1.cpp \
	example/multi_type_vector/event1.cpp \
	example/multi_type_vector/mtv_collection.cpp \
	example/multi_type_vector/pos_hint.cpp \
	example/multi_type_vector/standard_custom_store.cpp \
	example/packed_trie_map.cpp \
	example/packed_trie_state_custom.cpp \
	example/packed_trie_state_int.cpp \
	example/point_quad_tree.cpp \
	example/rtree_erase.cpp \
	example/rtree_medium.cpp \
	example/rtree_medium_bulkload.cpp \
	example/rtree_simple.cpp \
	example/segment_tree.cpp \
	example/sorted_string_map.cpp \
	example/trie_map.cpp \
	misc/gdb/mdds/__init__.py \
	misc/matrix_perf.cpp \
	misc/mdds.pc.in \
	misc/sorted_string_data.dat \
	misc/sorted_string_map_perf.cpp

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

doc-open:
	@xdg-open ./doc/_build/index.html

doc-clean:
	@rm -rf ./doc/_build ./doc/_doxygen

endif

check-gdb:
	make -C test/gdb check
