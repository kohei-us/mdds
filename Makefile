
OBJDIR=./obj
SRCDIR=./src
INCDIR=./inc

CPPFLAGS=-I$(INCDIR) -DDEBUG_NODE_BASE -DUNIT_TEST -Wall -g -std=c++0x
LDFLAGS=

EXECS= \
	flatsegmenttree-test \
	segmenttree-test

HEADERS= \
	$(INCDIR)/node.hpp \
	$(INCDIR)/flatsegmenttree.hpp \
	$(INCDIR)/segmenttree.hpp

OBJFILES= \
	$(OBJDIR)/flatsegmenttree_test.o \
	$(OBJDIR)/segmenttree_test.o

all: $(EXECS)

pre:
	mkdir $(OBJDIR) 2>/dev/null || /bin/true

$(OBJDIR)/flatsegmenttree_test.o: $(SRCDIR)/flatsegmenttree_test.cpp $(HEADERS)
	$(CXX) $(CPPFLAGS) -c -o $@ $(SRCDIR)/flatsegmenttree_test.cpp

$(OBJDIR)/segmenttree_test.o: $(SRCDIR)/segmenttree_test.cpp  $(HEADERS)
	$(CXX) $(CPPFLAGS) -c -o $@ $(SRCDIR)/segmenttree_test.cpp

flatsegmenttree-test: pre $(OBJDIR)/flatsegmenttree_test.o
	$(CXX) $(LDFLAGS) $(OBJDIR)/flatsegmenttree_test.o -o $@

segmenttree-test: pre $(OBJDIR)/segmenttree_test.o
	$(CXX) $(LDFLAGS) $(OBJDIR)/segmenttree_test.o -o $@

test.fst: flatsegmenttree-test
	./flatsegmenttree-test

test.st: segmenttree-test
	./segmenttree-test

test.st.mem: segmenttree-test
	valgrind --tool=memcheck --leak-check=full ./segmenttree-test

clean:
	rm -rf $(OBJDIR)
	rm $(EXECS)

