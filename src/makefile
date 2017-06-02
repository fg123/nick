CC=gcc

CXXFLAGS= -std=c99
ODIR = ../build
BINDIR = ../bin
SRCDIR = ./
INCDIR = ../headers
CFLAGS=-I $(INCDIR) -lxml2 -lhpdf -g 
CFLAGSNOLINK = -I $(INCDIR) -g

_DEPS = view.h
DEPS = $(patsubst %,$(INCDIR)/%,$(_DEPS))

_OBJ = main.o view.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: $(SRCDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGSNOLINK) $(CXXFLAGS)

main: $(OBJ)
	$(CC) -o $(BINDIR)/nick $^ $(CFLAGS) $(CXXFLAGS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(SRCDIR)/*~ 