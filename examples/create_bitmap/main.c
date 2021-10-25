


/*
 * Test program for bitmap library. Create a 10x10 bitmap and set 
 * some pixels with different colours.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "error_codes.h"
#include "bitmap.h"

int main(int argc, char** argv) {

    struct bitmap_definition_t* new_bitmap = bitmap_create(10, 10, RGB24);
    bitmap_fill(new_bitmap, 0xff, 0xff, 0xff);
    bitmap_set_pixel(new_bitmap, 3, 3, 0xff, 0x00, 0x00, 0x00);
    bitmap_set_pixel(new_bitmap, 4, 4, 0x00, 0xff, 0x00, 0x00);
    bitmap_set_pixel(new_bitmap, 5, 5, 0x00, 0x00, 0xff, 0x00);
    bitmap_set_pixel(new_bitmap, 3, 5, 0xff, 0x00, 0xff, 0x00);
    bitmap_set_pixel(new_bitmap, 5, 3, 0x00, 0xff, 0xff, 0x00);
    bitmap_write_to_file(new_bitmap,
                         "../examples/create_bitmap/new_bitmap.bmp");
    bitmap_destroy(new_bitmap);
    new_bitmap = NULL;

    return 0;
}
