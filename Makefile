
OBJDIR=./obj
SRCDIR=./src
INCDIR=./inc

CPPFLAGS=-I$(INCDIR) -DDEBUG_NODE_BASE -DUNIT_TEST -Wall
LDFLAGS=

EXECS= \
	flatsegmenttree-test \
	segmenttree-test

HEADERS= \
	$(INCDIR)/node.hpp \
	$(INCDIR)/flatsegmenttree.hpp \
	$(INCDIR)/segmenttree.hpp

OBJFILES= \
	$(OBJDIR)/main.o \
	$(OBJDIR)/segmenttree_test.o

all: $(EXECS)

pre:
	mkdir $(OBJDIR) 2>/dev/null || /bin/true

$(OBJDIR)/main.o: $(SRCDIR)/main.cpp
	$(CXX) $(CPPFLAGS) -c -o $@ $(SRCDIR)/main.cpp

$(OBJDIR)/segmenttree_test.o: $(SRCDIR)/segmenttree_test.cpp
	$(CXX) $(CPPFLAGS) -c -o $@ $(SRCDIR)/segmenttree_test.cpp

flatsegmenttree-test: pre $(OBJDIR)/main.o
	$(CXX) $(LDFLAGS) $(OBJDIR)/main.o -o $@

segmenttree-test: pre $(OBJDIR)/segmenttree_test.o
	$(CXX) $(LDFLAGS) $(OBJDIR)/segmenttree_test.o -o $@

test.fst: flatsegmenttree-test
	./flatsegmenttree-test

test.st: segmenttree-test
	./segmenttree-test

clean:
	rm -rf $(OBJDIR)
	rm $(EXECS)

