### ---------------------------------------------------------------------------
### EXECUTABLES
### ---------------------------------------------------------------------------

CC = g++

### ---------------------------------------------------------------------------
### FILES
### ---------------------------------------------------------------------------

SOURCES = $(wildcard *.cpp)
HEADERS = $(SOURCES:.cpp=.h)
OBJECTS = $(SOURCES:.cpp=.o)

OUTPUT = bin/dudegui

### ---------------------------------------------------------------------------
### PARAMETERS
### ---------------------------------------------------------------------------

STANDARD = -std=c++11
WARNINGS = -Wall
# NOOLDCAST = -Wold-style-cast
# EFFICIENCY = -Weffc++
TUNNING = -O3 -fshort-enums
DEBUGING = -g -fno-omit-frame-pointer

FLAGS = $(STANDARD) $(WARNINGS) $(EFFICIENCY) $(NOOLDCAST) $(TUNNING)
#FLAGS = $(STANDARD) $(WARNINGS) $(EFFICIENCY) $(NOOLDCAST) $(DEBUGGING)

LIBS=`pkg-config --cflags --libs gtkmm-3.0 libxml++-2.6`

### ---------------------------------------------------------------------------
### TARGETS
### ---------------------------------------------------------------------------

bin: $(OBJECTS)
	$(CC) $(FLAGS) $(OBJECTS) -o $(OUTPUT) $(LIBS)

clean:
	-rm -f *.o *.lst *~ ./bin/*~ $(OUTPUT)

### ---------------------------------------------------------------------------
### RECIPES
### ---------------------------------------------------------------------------

%.o: %.cpp
	 $(CC) $(FLAGS) -c -o $@ $^ $(LIBS)
