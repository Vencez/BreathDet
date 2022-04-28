CFLAGS=-g -Wall `pkg-config --cflags --libs gtkmm-plplot-2.0`

SRC_DIR=src
OBJ_DIR=obj

GUI_SRC= $(SRC_DIR)/base.cpp $(SRC_DIR)/data_processing.cpp $(SRC_DIR)/input_handler.cpp

GUI_OBJ= obj/base.o obj/breath_plot.o obj/main.o
CMD_OBJ= obj/input_handler.o obj/data_processing.o

all: breath_det

breath_det: $(GUI_OBJ) $(CMD_OBJ) 
	g++ $(CFLAGS) -o breath_det $(GUI_OBJ) $(CMD_OBJ)

$(OBJ_DIR)/breath_plot.o: $(SRC_DIR)/breath_plot.cpp
	g++ $(CFLAGS) -c $(SRC_DIR)/breath_plot.cpp

$(OBJ_DIR)/base.o: $(GUI_SRC)
	g++ $(CFLAGS) -c $(GUI_SRC)
	mv *.o obj/

obj/main.o: $(SRC_DIR)/main.cpp
	g++ $(CFLAGS) -c $(SRC_DIR)/main.cpp
	mv *.o obj/

clean:
	rm -f obj/*.o breath_det
