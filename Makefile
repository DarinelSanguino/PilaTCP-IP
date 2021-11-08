TARGET = $(BIN_DIR)/sim_tcp_ip
LIBS = -lpthread -L ./CommandParser -lcli -lrt
OBJS = $(OBJ_DIR)/prueba.o \
	   $(OBJ_DIR)/ListaEnlazadaGenerica.o \
	   $(OBJ_DIR)/Grafico.o \
	   $(OBJ_DIR)/Net.o \
	   $(OBJ_DIR)/Topologias.o \
	   $(OBJ_DIR)/Com.o \
	   $(OBJ_DIR)/Utiles.o \
	   $(OBJ_DIR)/Capa2.o
BIN_DIR = ./bin
OBJ_DIR = ./obj
INC_DIR = ./inc
SRC_DIR = ./src
CFLAGS = -g -lpthread -Wall -I$(INC_DIR)

$(TARGET): $(OBJS) CommandParser/libcli.a
	mkdir -p $(BIN_DIR)
	gcc $(CFLAGS) $(OBJS) -o $(TARGET) $(LIBS)
	
$(OBJ_DIR)/%.o : %.c
	mkdir -p $(OBJ_DIR)
	gcc -c -MD $(CFLAGS) $< -o $@
	
CommandParser/libcli.a:
	(cd CommandParser; make)

-include $(OBJ_DIR)/*.d
	
.PHONY: clean
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
	(cd CommandParser; make clean)
all:
	make
	(cd CommandParser; make)

