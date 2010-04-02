
OBJDIR=./obj
SRCDIR=./src
INCDIR=./inc

CPPFLAGS=-I$(INCDIR) -DDEBUG_NODE_BASE -DUNIT_TEST -Wall -Os -g -std=c++0x
LDFLAGS=

EXECS= \
	flatsegmenttree-test \
	segmenttree-test \
	stlperf-test

HEADERS= \
	$(INCDIR)/node.hpp \
	$(INCDIR)/flatsegmenttree.hpp \
	$(INCDIR)/segmenttree.hpp

OBJFILES= \
	$(OBJDIR)/flatsegmenttree_test.o \
	$(OBJDIR)/segmenttree_test.o

DEPENDS= \
	$(HEADERS)

all: $(EXECS)

pre:
	mkdir $(OBJDIR) 2>/dev/null || /bin/true

$(OBJDIR)/flatsegmenttree_test.o: $(SRCDIR)/flatsegmenttree_test.cpp $(DEPENDS)
	$(CXX) $(CPPFLAGS) -c -o $@ $(SRCDIR)/flatsegmenttree_test.cpp

$(OBJDIR)/segmenttree_test.o: $(SRCDIR)/segmenttree_test.cpp  $(DEPENDS)
	$(CXX) $(CPPFLAGS) -c -o $@ $(SRCDIR)/segmenttree_test.cpp

flatsegmenttree-test: pre $(OBJDIR)/flatsegmenttree_test.o
	$(CXX) $(LDFLAGS) $(OBJDIR)/flatsegmenttree_test.o -o $@

segmenttree-test: pre $(OBJDIR)/segmenttree_test.o
	$(CXX) $(LDFLAGS) $(OBJDIR)/segmenttree_test.o -o $@

stlperf-test: pre $(SRCDIR)/stlperf_test.cpp
	$(CXX) $(LDFLAGS) $(CPPFLAGS) $(SRCDIR)/stlperf_test.cpp -o $@

$(OBJDIR)/template_test.o: $(SRCDIR)/template_test.cpp $(DEPENDS)
	$(CXX) $(CPPFLAGS) -c -o $@ $(SRCDIR)/template_test.cpp

test.fst: flatsegmenttree-test
	./flatsegmenttree-test

test.fst.mem: flatsegmenttree-test
	valgrind --tool=memcheck --leak-check=full ./flatsegmenttree-test

test.st: segmenttree-test
	./segmenttree-test func

test.st.mem: segmenttree-test
	valgrind --tool=memcheck --leak-check=full ./segmenttree-test func

test.stl: stlperf-test
	./stlperf-test

clean:
	rm -rf $(OBJDIR) 2>/dev/null || /bin/true
	rm $(EXECS) 2>/dev/null || /bin/true

