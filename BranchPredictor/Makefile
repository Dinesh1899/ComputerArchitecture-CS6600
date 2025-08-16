CC = g++
OPT = -g
#OPT = -g
#WARN = -Wall
#CFLAGS = $(OPT) $(WARN) $(INC) $(LIB)
CFLAGS = $(OPT) $(INC) $(LIB)

# List all your .cc files here (source files, excluding header files)
SIM_SRC = bpsim.cpp
TARGET = bpsim

# List corresponding compiled object files here (.o files)
OBJ_DIR = builds
SIM_OBJ = $(OBJ_DIR)/bpsim.o
 
#################################

# default rule

all: sim
	@echo "my work is done here..."


# rule for making sim

sim: $(SIM_OBJ)
	$(CC) -o $(TARGET) $(CFLAGS) $(SIM_OBJ) -lm
	@echo "-----------DONE WITH SIM-----------"


# generic rule for converting any .cc file to any .o file
# Rule to compile source files into object files
$(OBJ_DIR)/%.o: %.cpp
	$(CXX) -c $< -o $@


# type "make clean" to remove all .o files plus the sim binary

clean:
	rm -f *.o $(OBJ_DIR)/*.o $(TARGET)


# type "make clobber" to remove all .o files (leaves sim binary)

clobber:
	rm -f *.o $(OBJ_DIR)/*.o

