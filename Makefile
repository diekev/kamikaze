CXX = g++
CXXFLAGS = -std=c++11 -g -Wall -pthread -fsanitize=address -fno-omit-frame-pointer
LDLIBS = -lGL -lglut -lGLEW -lopenvdb -lHalf -ltbb -lblosc -lz -fsanitize=address

OBJ_DIR = bin
LIB_DIR = -L/usr/lib -L/opt/lib/openvdb/lib -L/opt/lib/openexr/lib -L/opt/lib/blosc/lib
INC_DIR = -I/usr/include -I/opt/lib/openvdb/include -I/opt/lib/openexr/include

SRC = main.cpp volume.cpp GLSLShader.cpp
OBJECTS = $(SRC:%.cpp=$(OBJ_DIR)/%.o)
EXEC = window.out

all: init $(OBJECTS) $(EXEC)

$(EXEC):
	$(CXX) $(CXXFLAGS) $(LIB_DIR) -o $@ $(OBJECTS) $(LDLIBS)

$(OBJ_DIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INC_DIR) -c $< -o $@

init:
	@mkdir -p $(OBJ_DIR)

clean:
	rm $(OBJECTS) $(EXEC)
