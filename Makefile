# Make file for building upwords

CC := g++ -std=c++11
SRCDIR := src
TARGETDIR := bin
TARGET_RELEASE := $(TARGETDIR)/ncupwords
TARGET_DEBUG := $(TARGETDIR)/ncupwords_debug

SRCEXT := cpp
SOURCES := $(shell find $(SRCDIR) -type f) # -name *.$(SRCEXT))
LIB := -lncurses
CFLAGS := -g

debug: $(SOURCES)
	mkdir -p $(TARGETDIR)
	$(CC) $(CFLAGS) $(LIB) -o $(TARGET_DEBUG) $(SRCDIR)/main.cpp

release: $(SOURCES)
	mkdir -p $(TARGETDIR)
	$(CC) $(LIB) -o $(TARGET_RELEASE) $(SRCDIR)/main.cpp -O3

clean:
	rm -r $(TARGETDIR)
