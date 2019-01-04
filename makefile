CC=gcc

CXXFLAGS= -std=c99
ODIR = build
BINDIR = bin
SRCDIR = src
INCDIR = headers
CFLAGS=-I $(INCDIR) -lhpdf -lxml2 -lz -lm -g -lpng
CFLAGSNOLINK = -I $(INCDIR) -lxml2 -lhpdf -g -lpng

_DEPS = view.h error.h verify.h util.h layout.h pdf.h draw.h color.h fonts.h templates.h print.h
DEPS = $(patsubst %,$(INCDIR)/%,$(_DEPS))

_OBJ = main.o view.o error.o layout.o util.o draw.o color.o fonts.o templates.o print.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

all: setup main

$(ODIR)/%.o: $(SRCDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGSNOLINK) $(CXXFLAGS)

setup:
	mkdir -p $(BINDIR)
	mkdir -p $(ODIR)

main: $(OBJ)
	$(CC) -o $(BINDIR)/nick $^ $(CFLAGS) $(CXXFLAGS) -fPIC

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(SRCDIR)/*~
