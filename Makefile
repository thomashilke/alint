CXX = g++
CXXFLAGS = -g -std=c++11
LDFLAGS = -g

BIN_DIR = ~/.local/bin/

BIN = 
OBJECTS = 

.PHONY = all clean install

all: $(BIN)

bin/...: build/....o
\t$(CXX) $(LDFLAGS) -o  $<

build/....o: src/...
\t$(CXX) $(CXXFLAGS) -c -o  $<

clean:
\trm -f $(OBJECTS)
\trm -f $(BIN)

install: bin/...
\tcp bin/... $(BIN_DIR)
