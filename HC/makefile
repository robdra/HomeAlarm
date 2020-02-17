INC_DIR = inc
SRC_DIR = src
BIN_DIR = bin
OBJ_DIR = obj

All:
	g++ -I$(INC_DIR) -c $(SRC_DIR)/HeadEnd.cpp $(INC_DIR)/SerialPort.cpp 
	move HeadEnd.o $(OBJ_DIR)
	move SerialPort.o $(OBJ_DIR)

	g++ $(OBJ_DIR)/HeadEnd.o $(OBJ_DIR)/SerialPort.o -o $(BIN_DIR)/Alarm.exe