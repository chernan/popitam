CC		= g++
CFLAGS		= -O3 -Wall -ansi 

SRC_DIR		= ./src 
VPATH		= $(SRC_DIR)

INCLUDE_DIR	= -I ./src 
OBJ_DIR		= ./obj
OUT_DIR		= ./bin
OUT		= $(OUT_DIR)/form.cgi

OBJECTS		= cgi check display error field file html item itemlist lexico main manager map \
		  msg paths popitamform popiparser popiresult pszlist tagfile taxon taxonomy \
		  txtfile util radio radiogroup

RM		= /bin/rm -rf
MKDIR		= /bin/mkdir -p

all:	make_dir $(OUT)

clean:	clean_obj
	$(RM) $(OUT_DIR)

clean_obj:
	$(RM) $(OBJ_DIR)

make_dir:	$(OBJ_DIR) $(OUT_DIR)

$(OUT_DIR):
	$(MKDIR) $@

$(OBJ_DIR):
	$(MKDIR) $@

$(OUT):	$(foreach o,$(OBJECTS),$(OBJ_DIR)/$(o).o)
	$(CC) $(COPTION) -o $(OUT) $^

$(OBJ_DIR)/manager.o: manager.cpp
	$(CC) $(INCLUDE_DIR) $(CFLAGS) $(COPTION) $(PATH_FILE)	-c $< -o $@

$(OBJ_DIR)/%.o: %.cpp
	$(CC) $(INCLUDE_DIR) $(CFLAGS) $(COPTION) $(MACROS) -c $< -o $@
