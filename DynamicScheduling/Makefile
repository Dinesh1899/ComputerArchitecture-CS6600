CC = g++
OPT = -O3
#OPT = -g
WARN = -Wall
CFLAGS = $(OPT) $(WARN) $(INC) $(LIB)

TARGET = ooosim
SRC_DIR = src
OBJ_DIR = builds


# Mention all cpp files here
SIM_SRC = $(SRC_DIR)/$(TARGET).cpp $(SRC_DIR)/components.cpp $(SRC_DIR)/issue_handler.cpp

SIM_OBJ = $(OBJ_DIR)/$(TARGET).o $(OBJ_DIR)/components.o $(OBJ_DIR)/issue_handler.o


all: $(TARGET)
	@echo "Build File Generation Done!!"


$(TARGET): $(SIM_OBJ)
	$(CC) -o $(TARGET) $(CFLAGS) $(SIM_OBJ) -lm
	@echo "Compilation Done!!"

# Rule to compile .cpp files to .o files in the objects directory
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ_DIR)/*.o ooosim plot

clobber:
	rm -f $(OBJ_DIR)/*.o


