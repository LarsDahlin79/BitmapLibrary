

CC=gcc
EXE=test
DEBUG=debug
OBJ=main.o
LIB=bitmap.a
INC_DIR=../../include
ARCHIVE=ar
SRC_DIR=../../source
EXAMPLE_DIR=../../examples
CFLAGS=-Wall -Werror -ansi -g

all: $(LIB) $(TEST_STAT)

$(EXE): $(OBJ) $(LIB)
	$(CC) $(CFLAGS) -I$(INC_DIR) -lm -o $@ $(OBJ) $(LIB)

$(DEBUG): $(OBJ) bitmap_debug.o
	$(CC) $(CFLAGS) -I$(INC_DIR) -lm -o $@ $(OBJ) bitmap_debug.o

$(OBJ): $(EXAMPLE_DIR)/create_bitmap/main.c 
	$(CC) $(CFLAGS) -I$(INC_DIR) -c -o $@ $^

bitmap_debug.o: $(SRC_DIR)/bitmap.c
	$(CC) $(CFLAGS) -g -I$(INC_DIR) -c -o $@ $^

bitmap.o: $(SRC_DIR)/bitmap.c 
	$(CC) $(CFLAGS) -I$(INC_DIR) -c -o $@ $^

$(LIB): bitmap.o
	$(ARCHIVE) rcs $@ $^
	ranlib $(LIB)

.PHONY: clean indent check install
clean:
	rm $(EXE) $(DEBUG) *.o $(LIB) 2> /dev/null ../source/*~ ../include/*~ *~ ../examples/create_bitmap/*~ ../examples/create_bitmap/*.bmp

indent:
	indent --align-with-spaces --braces-on-if-line --braces-on-func-def-line --cuddle-else --fix-nested-comments --indent-level4 --k-and-r-style -nbap --no-tabs --pointer-align-left --remove-preprocessor-space  $(SRC_DIR)/bitmap.c $(INC_DIR)/bitmap.h $(INC_DIR)/error_codes.h $(EXAMPLE_DIR)/create_bitmap/main.c

check:
	cppcheck --language=c --enable=warning --enable=style --enable=portability --enable=performance ../examples/create_bitmap/ ../source/

install:
	cp libbitmap.so /usr/local/lib/
	cp ../include/*.h /usr/local/include/

