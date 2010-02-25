
EXEC=test.bin
OBJDIR=./obj
SRCDIR=./src
INCDIR=./inc

CPPFLAGS=-I$(INCDIR) -DDEBUG_NODE_BASE -DUNIT_TEST
LDFLAGS=

HEADERS= \
	$(INCDIR)/node.hpp \
	$(INCDIR)/flatsegmenttree.hpp

OBJFILES= \
	$(OBJDIR)/main.o

all: $(EXEC)

pre:
	mkdir $(OBJDIR) 2>/dev/null || /bin/true

$(OBJDIR)/main.o: $(SRCDIR)/main.cpp
	$(CXX) $(CPPFLAGS) -c -o $@ $(SRCDIR)/main.cpp

$(EXEC): pre $(OBJFILES)
	$(CXX) $(LDFLAGS) $(OBJFILES) -o $(EXEC)

test: $(EXEC)
	./$(EXEC)

clean:
	rm -rf $(OBJDIR)
	rm $(EXEC)

