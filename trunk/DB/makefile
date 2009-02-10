CC		= g++
CFLAGS		= -O3 -Wall -ansi 

SRC_DIR		= ./src ../src/DIGEST/base ../src/DIGEST/database
VPATH		= ${SRC_DIR}

INCLUDE_DIR 	= -I . -I ../src/DIGEST/base -I ../src/DIGEST/database
OBJ_DIR		= ./obj
OUT_DIR		= ./bin
OUT		= $(OUT_DIR)/createDB

OBJECTS		= atomicmass MATerror file pimw util \
		  dbentry dbfile dbptm dbentry dbfile dbptm dbfilewriter ptmdata ptmsearchmanager taxon taxonomy \
		  main parser tagfile txtfile file 

RM		= /bin/rm -rf
MKDIR		= /bin/mkdir -p

all:	make_dir $(OUT)

clean:	clean_obj
	$(RM) $(OUT_DIR)

clean_obj:
	$(RM) $(OBJ_DIR)

make_dir: $(OBJ_DIR) $(OUT_DIR)

$(OBJ_DIR) $(OUT_DIR):
	$(MKDIR) $@

$(OUT): $(foreach o,$(OBJECTS),$(OBJ_DIR)/$(o).o) 
	$(CC) -o $(OUT) $^

$(OBJ_DIR)/%.o: %.cpp
	$(CC) $(INCLUDE_DIR) $(CFLAGS) -c $< -o $@
