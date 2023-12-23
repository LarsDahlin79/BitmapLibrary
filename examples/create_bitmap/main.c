


/*
 * Test program for bitmap library. Create a 10x10 bitmap and set 
 * some pixels with different colours.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <error_codes.h>
#include <bitmap.h>

int main(int argc, char** argv) {

    struct bitmap_definition_t* new_bitmap = bitmap_create(10, 10, RGB24);
    bitmap_fill(new_bitmap, 0xff, 0xff, 0xff);
    bitmap_set_pixel(new_bitmap, 3, 3, 0xff, 0x00, 0x00, 0x00);
    bitmap_set_pixel(new_bitmap, 4, 4, 0x00, 0xff, 0x00, 0x00);
    bitmap_set_pixel(new_bitmap, 5, 5, 0x00, 0x00, 0xff, 0x00);
    bitmap_set_pixel(new_bitmap, 3, 5, 0xff, 0x00, 0xff, 0x00);
    bitmap_set_pixel(new_bitmap, 5, 3, 0x00, 0xff, 0xff, 0x00);

    uint32_t red, green, blue, alpha;
    bitmap_get_pixel(new_bitmap, 3, 3, &red, &green, &blue, &alpha);
    if (red != 0xff || green != 0x00 || blue != 0x00){
	fprintf(stderr, "Error reading pixel\n");
    }

    bitmap_write_to_file(new_bitmap,
                         "../../examples/create_bitmap/new_bitmap.bmp");
    bitmap_destroy(new_bitmap);
    new_bitmap = NULL;

    struct bitmap_definition_t* bw_bitmap = bitmap_create(16, 16, RGB24);
    bitmap_fill(bw_bitmap, 0xff, 0xff, 0xff);
    uint8_t ypos = 0;
    while (ypos <= 15) {
        uint8_t xpos = ypos % 2;
        while (xpos <= 15) {
            bitmap_set_pixel(bw_bitmap, xpos, ypos, 0x00, 0x00, 0x00,
                             0x00);
            xpos += 2;
        }
        ypos++;
    }
    bitmap_write_to_file(bw_bitmap,
                         "../../examples/create_bitmap/bw_bitmap.bmp");
    bitmap_destroy(bw_bitmap);
    bw_bitmap = NULL;

    struct bitmap_definition_t* line_bitmap =
        bitmap_create(125, 100, RGB24);
    bitmap_fill(line_bitmap, 0x00, 0x00, 0x00);
    bitmap_draw_line(line_bitmap, 62, 10, 62, 50, 0x7f, 0xff, 0xff, 0x00);
    bitmap_draw_line(line_bitmap, 10, 10, 62, 50, 0xff, 0x7f, 0xff, 0x00);
    bitmap_draw_line(line_bitmap, 10, 90, 62, 50, 0xff, 0xff, 0x7f, 0x00);
    bitmap_draw_line(line_bitmap, 115, 10, 62, 50, 0x7f, 0x7f, 0xff, 0x00);
    bitmap_draw_line(line_bitmap, 115, 90, 62, 50, 0xff, 0x7f, 0x7f, 0x00);
    bitmap_draw_line(line_bitmap, 62, 90, 62, 50, 0x7f, 0xff, 0x7f, 0x00);
    bitmap_draw_line(line_bitmap, 10, 50, 62, 50, 0x10, 0x20, 0x30, 0x00);
    bitmap_draw_line(line_bitmap, 115, 50, 62, 50, 0x40, 0x50, 0x60, 0x00);
    bitmap_set_pixel(line_bitmap, 62, 50, 0xff, 0xff, 0xff, 0x00);
    bitmap_write_to_file(line_bitmap,
                         "../../examples/create_bitmap/line_bitmap.bmp");

    bitmap_destroy(line_bitmap);
    line_bitmap = NULL;

    struct bitmap_definition_t* arrow_bitmap =
        bitmap_create(150, 100, RGB24);
    bitmap_fill(arrow_bitmap, 0x00, 0x00, 0x00);
    bitmap_draw_rectangle(arrow_bitmap, 30, 30, 95, 60, 0xff, 0xff, 0xff,
                          0x00);
    bitmap_draw_triangle(arrow_bitmap, 95, 20, 95, 45, 125, 45, 0xff, 0xff,
                         0xff, 0x00);
    bitmap_draw_triangle(arrow_bitmap, 95, 70, 95, 44, 115, 44, 0xff, 0xff,
                         0xff, 0x00);
    bitmap_write_to_file(arrow_bitmap,
                         "../../examples/create_bitmap/arrow_bitmap.bmp");
    bitmap_destroy(arrow_bitmap);
    arrow_bitmap = NULL;

    struct bitmap_definition_t* ellipse_bitmap =
        bitmap_create(750, 705, RGB24);
    bitmap_fill(ellipse_bitmap, 0x00, 0x00, 0x00);
    bitmap_draw_ellipse(ellipse_bitmap, 200, 150, 100, 100, 0, 2 * 3.14f,
                        0xFF, 0xFF, 0xFF, 0x00);
    bitmap_draw_ellipse(ellipse_bitmap, 200, 450, 200, 100, 0, 2 * 3.14f,
                        0xFF, 0xFF, 0x00, 0x00);
    bitmap_draw_ellipse(ellipse_bitmap, 400, 150, 100, 200, 0, 2 * 3.14f,
                        0xFF, 0x00, 0x00, 0x00);
    bitmap_write_to_file(ellipse_bitmap,
                         "../../examples/create_bitmap/ellipse_bitmap.bmp");
    bitmap_destroy(ellipse_bitmap);
    ellipse_bitmap = NULL;

    /* test insert bitmap */
    /**********************************************/
    struct bitmap_definition_t* original_bitmap =
	bitmap_create(10, 10, RGB24);
    bitmap_fill(original_bitmap, 0x00, 0x00, 0x00);

    struct bitmap_definition_t* small_bitmap =
	bitmap_create(5, 5, RGB24);
    bitmap_fill(small_bitmap, 0xFF, 0xFF, 0xFF);

    bitmap_insert_bitmap(original_bitmap, 1, 2, small_bitmap, 0, 0, 5, 5);

    bitmap_write_to_file(original_bitmap,
                         "../../examples/create_bitmap/insert_bitmap.bmp");

    bitmap_destroy(original_bitmap);
    original_bitmap = NULL;

    bitmap_destroy(small_bitmap);
    small_bitmap = NULL;
    /**********************************************/

    /* test copy bitmap */
    /**********************************************/
    struct bitmap_definition_t* copy_bitmap =
	bitmap_create(10, 10, RGB24);
    error_codes error =  bitmap_read_from_file(copy_bitmap,
					       "../../examples/create_bitmap/new_bitmap.bmp");
    if (NO_ERROR != error){
	fprintf (stderr, "Error reading bitmap from file %d\n", error);
	bitmap_destroy(copy_bitmap);
	return 0;
    }
    bitmap_write_to_file(copy_bitmap,
                         "../../examples/create_bitmap/copy_bitmap.bmp");
    bitmap_destroy(copy_bitmap);
    /**********************************************/

    return 0;
}
