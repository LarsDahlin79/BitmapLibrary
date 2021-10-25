

CC=gcc
CFLAGS=-Wall -Werror -ansi
EXE=test
DEBUG=debug
OBJ=main.o
LIB=libbitmap.so
INC=-I../include

all: $(LIB)

$(EXE): $(OBJ) $(LIB)
	$(CC) $(CFLAGS) -L./ -o $@ $(OBJ) -lbitmap

$(DEBUG): $(OBJ) bitmap_debug.o
	$(CC) $(CFLAGS) $(INC) -o $@ $(OBJ) bitmap_debug.o

$(OBJ): ../examples/create_bitmap/main.c 
	$(CC) $(CFLAGS) $(INC) -c -o $@ $^

bitmap_debug.o: ../source/bitmap.c
	$(CC) $(CFLAGS) -g $(INC) -c -o $@ $^
bitmap.o: ../source/bitmap.c 
	$(CC) $(CFLAGS) -O2 $(INC) -fpic -c -o $@ $^

$(LIB): bitmap.o
	$(CC) $(CFLAGS) -fpic -shared -o $@ $^

.PHONY: clean indent check install
clean:
	rm $(EXE) $(DEBUG) *.o $(LIB) 2> /dev/null ../source/*~ ../include/*~ *~ ../examples/create_bitmap/*~ ../examples/create_bitmap/*.bmp

indent:
	indent --align-with-spaces --braces-on-if-line --braces-on-func-def-line --cuddle-else --fix-nested-comments --indent-level4 --k-and-r-style -nbap --no-tabs --pointer-align-left --remove-preprocessor-space  ../source/bitmap.c ../include/bitmap.h ../include/error_codes.h ../examples/create_bitmap/main.c

check:
	cppcheck --language=c --enable=warning --enable=style --enable=portability --enable=performance ../examples/create_bitmap/ ../source/

install:
	@echo "TODO"
