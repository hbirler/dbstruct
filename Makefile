CC := g++
EMCC := emcc
SRCDIR := src
BUILDDIR := build
TARGETDIR := bin
TARGET := $(TARGETDIR)/db
INCDIR := include
 
SRCEXT := cpp
INCEXT := h
SOURCES := $(wildcard $(SRCDIR)/*.$(SRCEXT))
HEADERS := $(wildcard $(INCDIR)/*.$(INCEXT))
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
EMOBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.bc))
CFLAGS := -std=c++11 -g -O2 # -Wall
EMCFLAGS := -std=c++11 -O2
LIB := # 
INC := -I include

all: build test

compile: $(TARGET)

$(BUILDDIR)/%.bc: $(SRCDIR)/%.$(SRCEXT) $(HEADERS)
	@mkdir -p $(BUILDDIR)
	@mkdir -p $(TARGETDIR)
	@echo " $(CC) $(CFLAGS) $(INC) -c -o $@ $<"; $(CC) $(CFLAGS) $(INC) -c -o $@ $<


$(TARGET): $(OBJECTS)
	@echo " Linking..."
	@echo " $(CC) $^ -o $(TARGET) $(LIB)"; $(CC) $^ -fopenmp -o $(TARGET) $(LIB)

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT) $(HEADERS)
	@mkdir -p $(BUILDDIR)
	@mkdir -p $(TARGETDIR)
	@echo " $(CC) $(CFLAGS) $(INC) -c -o $@ $<"; $(CC) $(CFLAGS) $(INC) -c -o $@ $<

$(REQDIRS):
	@mkdir -p $(REQDIRS)

clean:
	@echo " Cleaning..."; 
	@echo " $(RM) -r $(BUILDDIR) $(TARGET)"; $(RM) -r $(BUILDDIR) $(TARGET)

cleanout:
	@echo " Cleaning..."; 
	@echo " $(RM) -r $(REQDIRS)"; $(RM) -r $(REQDIRS)
	@mkdir -p $(REQDIRS)

# Tests

test: $(TARGET)
	$(TARGET) test