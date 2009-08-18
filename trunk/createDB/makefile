CC		= g++
CFLAGS		= -O3 -Wall -ansi 

SRC_DIR		= ./src ./src/base ./src/database
VPATH		= ${SRC_DIR}

INCLUDE_DIR	= -I ./src -I ./src/base -I ./src/database
OBJ_DIR		= ./obj
OUT		= ../bin/createDB

OBJECTS		= atomicmass error file pimw tagfile txtfile util \
		  dbentry dbfile dbfilewriter dbptm ptmdata ptmsearchmanager taxon taxonomy \
		  main parserPhenyx parserCamsi parserBase parserSBG
								   
RM		= /bin/rm -rf
MKDIR		= /bin/mkdir -p

all:	make_dir $(OUT)

clean:	clean_obj
	$(RM) $(OUT)
	
clean_obj:
	$(RM) $(OBJ_DIR)

make_dir: $(OBJ_DIR)

$(OBJ_DIR):
	$(MKDIR) $@

$(OUT): $(foreach o,$(OBJECTS),$(OBJ_DIR)/$(o).o) 
	$(CC) -o $(OUT) $^
	/usr/bin/strip -s $(OUT)

$(OBJ_DIR)/%.o: %.cpp
	$(CC) $(INCLUDE_DIR) $(CFLAGS) -c $< -o $@
