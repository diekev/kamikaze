CXX = g++
CXXFLAGS = -std=c++11 -g -Wall -pthread
LDLIBS = -lGL -lglut -lGLEW

OBJ_DIR = bin
LIB_DIR = -L/usr/lib
INC_DIR = -I/usr/include

SRC = main.cpp GLSLShader.cpp
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
