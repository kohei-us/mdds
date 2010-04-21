
OBJDIR=./obj
SRCDIR=./src
INCDIR=./inc

CPPFLAGS=-I$(INCDIR) -DDEBUG_NODE_BASE -DUNIT_TEST -Wall -Os -g -std=c++0x
LDFLAGS=

EXECS= \
	flat_segment_tree_test \
	segment_tree_test \
	point_quad_tree_test \
	rectangle_set_test \
	stlperf_test

HEADERS= \
	$(INCDIR)/mdds/node.hpp \
	$(INCDIR)/mdds/global.hpp \
	$(INCDIR)/mdds/flat_segment_tree.hpp \
	$(INCDIR)/mdds/point_quad_tree.hpp \
	$(INCDIR)/mdds/segment_tree.hpp \
	$(INCDIR)/mdds/rectangle_set.hpp

OBJFILES= \
	$(OBJDIR)/flat_segment_tree_test.o \
	$(OBJDIR)/segment_tree_test.o \
	$(OBJDIR)/rectangle_set_test.o

DEPENDS= \
	$(HEADERS)

all: $(EXECS)

pre:
	mkdir $(OBJDIR) 2>/dev/null || /bin/true

$(OBJDIR)/flat_segment_tree_test.o: $(SRCDIR)/flat_segment_tree_test.cpp $(DEPENDS)
	$(CXX) $(CPPFLAGS) -c -o $@ $(SRCDIR)/flat_segment_tree_test.cpp

$(OBJDIR)/segment_tree_test.o: $(SRCDIR)/segment_tree_test.cpp  $(DEPENDS)
	$(CXX) $(CPPFLAGS) -c -o $@ $(SRCDIR)/segment_tree_test.cpp

$(OBJDIR)/rectangle_set_test.o: $(SRCDIR)/rectangle_set_test.cpp  $(DEPENDS)
	$(CXX) $(CPPFLAGS) -c -o $@ $(SRCDIR)/rectangle_set_test.cpp

flat_segment_tree_test: pre $(OBJDIR)/flat_segment_tree_test.o
	$(CXX) $(LDFLAGS) $(OBJDIR)/flat_segment_tree_test.o -o $@

segment_tree_test: pre $(OBJDIR)/segment_tree_test.o
	$(CXX) $(LDFLAGS) $(OBJDIR)/segment_tree_test.o -o $@

rectangle_set_test: pre $(OBJDIR)/rectangle_set_test.o
	$(CXX) $(LDFLAGS) $(OBJDIR)/rectangle_set_test.o -o $@

stlperf_test: pre $(SRCDIR)/stlperf_test.cpp
	$(CXX) $(LDFLAGS) $(CPPFLAGS) $(SRCDIR)/stlperf_test.cpp -o $@

$(OBJDIR)/template_test.o: $(SRCDIR)/template_test.cpp $(DEPENDS)
	$(CXX) $(CPPFLAGS) -c -o $@ $(SRCDIR)/template_test.cpp

test.fst: flat_segment_tree_test
	./flat_segment_tree_test

test.recset: rectangle_set_test
	./rectangle_set_test func

test.recset.perf: rectangle-set_test
	./rectangle_set_test perf

test.recset.mem: rectangle_set_test
	valgrind --tool=memcheck --leak-check=full ./rectangle-set_test func

test.fst.mem: flat_segment_tree_test
	valgrind --tool=memcheck --leak-check=full ./flat_segment_tree_test

test.st: segment_tree_test
	./segment_tree_test func

test.st.perf: segment_tree_test
	./segment_tree_test perf

test.st.mem: segment_tree_test
	valgrind --tool=memcheck --leak-check=full ./segment_tree_test func

test.stl: stlperf_test
	./stlperf_test

clean:
	rm -rf $(OBJDIR) 2>/dev/null || /bin/true
	rm $(EXECS) 2>/dev/null || /bin/true

