CXX = g++
CXXFLAGS = -std=c++11 -g -Wall -pthread -fsanitize=address -fno-omit-frame-pointer
LDLIBS = -lGL -lglut -lGLEW -lopenvdb -lHalf -ltbb -lblosc -lz -fsanitize=address

OBJ_DIR = bin
LIB_DIR = -L/usr/lib -L/opt/lib/openvdb/lib -L/opt/lib/openexr/lib -L/opt/lib/blosc/lib
INC_DIR = -I/usr/include -I/opt/lib/openvdb/include -I/opt/lib/openexr/include

SRC = main.cc volume.cc GLSLShader.cc
OBJECTS = $(SRC:%.cc=$(OBJ_DIR)/%.o)
EXEC = window.out

all: init $(OBJECTS) $(EXEC)

$(EXEC): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(LIB_DIR) -o $@ $^ $(LDLIBS)

$(OBJ_DIR)/%.o: %.cc
	$(CXX) $(CXXFLAGS) $(INC_DIR) -c $< -o $@

depend: $(OBJ_DIR)/.depend

$(OBJ_DIR)/.depend: $(SRC)
	@rm -f $(OBJ_DIR)/.depend
	$(CXX) $(CXXFLAGS) -MM $^>>$(OBJ_DIR)/.depend

include $(OBJ_DIR)/.depend

init:
	@mkdir -p $(OBJ_DIR)

clean:
	rm $(OBJECTS) $(EXEC)
