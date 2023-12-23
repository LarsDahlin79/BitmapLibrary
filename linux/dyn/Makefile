

CC=gcc
CFLAGS=-Wall -Werror -ansi -g
EXE=test
DEBUG=debug
OBJ=main.o
LIB=libbitmap.so
INC=../../include
SRC=../../source
EXAMPLES=../../examples

all: $(LIB)

$(EXE): $(OBJ) $(LIB)
	$(CC) $(CFLAGS) -L./ -o $@ $(OBJ) -lbitmap

$(DEBUG): $(OBJ) bitmap_debug.o
	$(CC) $(CFLAGS) -I$(INC) -o $@ $(OBJ) bitmap_debug.o

$(OBJ): ../../examples/create_bitmap/main.c 
	$(CC) $(CFLAGS) -I$(INC) -c -o $@ $^

bitmap_debug.o: ../../source/bitmap.c
	$(CC) $(CFLAGS) -g -I$(INC) -c -o $@ $^
bitmap.o: ../../source/bitmap.c 
	$(CC) $(CFLAGS) -O2 -I$(INC) -fpic -c -o $@ $^

$(LIB): bitmap.o
	$(CC) $(CFLAGS) -fpic -shared -o $@ $^

.PHONY: clean indent check install
clean:
	rm $(EXE) $(DEBUG) *.o $(LIB) 2> /dev/null ../../source/*~ ../../include/*~ *~ ../../examples/create_bitmap/*~ ../../examples/create_bitmap/*.bmp

indent:
	indent --align-with-spaces --braces-on-if-line --braces-on-func-def-line --cuddle-else --fix-nested-comments --indent-level4 --k-and-r-style -nbap --no-tabs --pointer-align-left --remove-preprocessor-space  $(SRC)/bitmap.c $(INC)/bitmap.h $(INC)/error_codes.h $(EXAMPLES)/create_bitmap/main.c

check:
	cppcheck --language=c --enable=warning --enable=style --enable=portability --enable=performance ../../examples/create_bitmap/ ../../source/

install:
	cp $(LIB) /usr/local/lib
	cp $(INC)/bitmap.h /usr/local/include
	cp $(INC)/error_codes.h /usr/local/include
