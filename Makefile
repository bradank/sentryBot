OBJ_DIR=obj
SRC_DIR=source
CC=g++
CFLAGS=-I/usr/local/include/opencv4
LIBS=-lopencv_core -lopencv_imgcodecs -lopencv_imgproc -lopencv_highgui -lX11

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(SRC_DIR)/%.h
	$(CC) -c -o $@ $< $(CFLAGS)

testBot: $(OBJ_DIR)/testBot.o
		$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

clean:
		rm *.o