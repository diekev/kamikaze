CXX = g++
CXXFLAGS = -std=c++11 -g -Wall -Wno-error=unused-function \
-Wextra -Wno-missing-field-initializers -Wno-sign-compare -Wno-type-limits  \
-Wno-unknown-pragmas -Wno-unused-parameter -Wno-ignored-qualifiers          \
-Wmissing-format-attribute -Wno-delete-non-virtual-dtor                     \
-Wsizeof-pointer-memaccess -Wformat=2 -Wno-format-nonliteral -Wno-format-y2k\
-fstrict-overflow -Wstrict-overflow=2 -Wno-div-by-zero -Wwrite-strings      \
-Wlogical-op -Wundef -DDEBUG_THREADS -Wnonnull -Wstrict-aliasing=2          \
-fno-omit-frame-pointer -Wno-error=unused-result -Wno-error=clobbered       \
-fstack-protector-all --param=ssp-buffer-size=4 -Wno-maybe-uninitialized    \
-Wunused-macros -Wmissing-include-dirs -Wuninitialized -Winit-self          \
-Wtype-limits -fno-common -fno-nonansi-builtins -Wformat-extra-args         \
-Wno-error=unused-local-typedefs -DWARN_PEDANTIC -Winit-self -Wdate-time    \
-Warray-bounds -Werror -fdiagnostics-color=always -fsanitize=address

LDLIBS = -lGL -lglut -lGLEW -lopenvdb -lHalf -ltbb -lblosc -lz -fsanitize=address

OBJ_DIR = bin
LIB_DIR = -L/usr/lib -L/opt/lib/openvdb/lib -L/opt/lib/openexr/lib -L/opt/lib/blosc/lib
INC_DIR = -I/usr/include -I/opt/lib/openvdb/include -I/opt/lib/openexr/include

SRC = main.cc GLSLShader.cc utils.cc viewer.cc volume.cc
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
