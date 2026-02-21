/*
  MIT License

  Copyright (c) 2021 LarsDahlin79

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "error_codes.h"
#include "bitmap.h"

struct bitmap_header_t {
    uint8_t id[2];
    uint32_t size_of_file;
    uint32_t unused;
};

/*
 * The size of DIB, Device Indenpendant Header, varios between
 * versions of bitmap formats and resolutions.
 */
struct dib_header_24bit {
    uint32_t data_offset;
    uint32_t dib_header_size;
    uint32_t width;
    uint32_t height;
    uint16_t nr_of_colour_planes;
    uint16_t resolution;
    uint32_t compression_type;
    uint32_t raw_size;
    uint32_t pixels_horisontal;
    uint32_t pixels_vertical;
    uint32_t nr_of_colours_in_palette;
    uint32_t nr_of_important_colours;
};

/**
 * This definition of bitmaps support the so called Windows version.
 * It uses 8 pixels per colour, i.e. 24 bit per pixel and doesn't 
 * have alpha channel.
 * This struct is placed in the c file because users are not meant
 * to modify the members by hand, they shall use the function defined
 * in the header file.
 * This struct is only for internal use, users shall use pointer
 * when handling bitmaps.
 */
struct bitmap_definition_t {
    /* Bitmap header */
    struct bitmap_header_t* bitmap_header;

    /* DIB header, Device Independant Bitmap */
    struct dib_header_24bit* dib_header_24bit;

    /* the actual pixel data */
    uint8_t* bitmap_data;
};

typedef enum {
    LEFT_TO_RIGHT_UP_DOWN,
    LEFT_TO_RIGHT_DOWN_UP,
    RIGHT_TO_LEFT_UP_DOWN,
    RIGHT_TO_LEFT_DOWN_UP,
    UP_DOWN,                    /* used for vertical lines */
    DOWN_UP,                    /* used for vertical lines */
    LEFT_RIGHT,                 /* used for horisontal lines */
    RIGHT_LEFT,                 /* used for horisontal lines */
    NO_DIRECTION,
    DIR_COUNT
} line_directions_t;

/* private function headers */
/*********************************************/
static struct bitmap_header_t* create_bitmap_header();
static struct dib_header_24bit* create_dib_header_24bit(uint32_t width,
                                                        uint32_t height);
static uint8_t* read_data_from_file(char* filename);
static error_codes set_pixel_24bit(struct bitmap_definition_t* bitmap,
                                   uint32_t pos, uint32_t red,
                                   uint32_t green, uint32_t blue);
error_codes get_pixel_24bit(struct bitmap_definition_t* bitmap,
                            uint32_t pos, uint32_t * red,
                            uint32_t * green, uint32_t * blue);
static uint32_t calculate_pos_24bit(uint32_t xpos, uint32_t ypos,
                                    uint32_t width);
static uint16_t get_resolution_int(resolutions_t resolution);
static uint32_t calculate_bitmap_width(uint32_t width,
                                       uint32_t resolution);
static uint32_t calculate_raw_size(uint32_t width, uint32_t height,
                                   uint32_t resolution);
static line_directions_t get_direction(uint32_t x1, uint32_t y1,
                                       uint32_t x2, uint32_t y2);
static float calc_gradient(line_directions_t dir, uint32_t x1, uint32_t y1,
                           uint32_t x2, uint32_t y2);
static int8_t get_x_inc(uint32_t x1, uint32_t y1, uint32_t x2,
                        uint32_t y2);
static float triangle_x_gradient(line_directions_t dir, uint32_t x1,
                                 uint32_t y1, uint32_t x2, uint32_t y2);
static bool line_last_pixel(line_directions_t dir, uint32_t x1, float y1,
                            uint32_t x2, float y2);
static bool triangle_last_line(uint32_t y1, uint32_t y2, bool dir);
static uint32_t get_ellipse_y_coordinate(uint32_t height, uint32_t xpos,
                                         uint32_t width);
/*********************************************/

struct bitmap_definition_t* bitmap_create(uint32_t width, uint32_t height,
                                          resolutions_t resolution) {

    /* check that the given resolution is known */
    if (0xFFFF == get_resolution_int(resolution)) {
        return NULL;
    }

    struct bitmap_definition_t* bitmap =
        malloc(sizeof(struct bitmap_definition_t));
    if (NULL == bitmap) {
        return NULL;
    }

    /* Setup the bitmap header */
    bitmap->bitmap_header = create_bitmap_header();
    if (NULL == bitmap->bitmap_header) {
        free(bitmap);
    }

    uint32_t data_size = 0;
    if (RGB24 == resolution) {
        bitmap->dib_header_24bit = create_dib_header_24bit(width, height);


        data_size = calculate_raw_size(bitmap->dib_header_24bit->width,
                                       bitmap->dib_header_24bit->height,
                                       bitmap->dib_header_24bit->
                                       resolution);
    }
    bitmap->bitmap_header->size_of_file = data_size;

    bitmap->bitmap_data = malloc(data_size);
    if (NULL == bitmap->bitmap_data) {
        return NULL;
    }

    return bitmap;
}

void bitmap_destroy(struct bitmap_definition_t* bitmap) {
    if (NULL == bitmap) {
        return;
    }
    if (NULL != bitmap->bitmap_data) {
        free(bitmap->bitmap_data);
    }
    if (NULL != bitmap->bitmap_header) {
        free(bitmap->bitmap_header);
    }
    if (NULL != bitmap->dib_header_24bit) {
	free(bitmap->dib_header_24bit);
    }
    free(bitmap);
}

error_codes bitmap_read_from_file(struct bitmap_definition_t* bitmap,
                                  char* filename) {

    uint8_t* data = read_data_from_file(filename);
    if (NULL == data) {
        return MEMORY_ALLOCATION_FAILED;
    }

    if ('B' != data[0] || 'M' != data[1]) {
        free(data);
        return MAGIC_NUMBER_MISSING;
    }

    /* TODO: Need to create serialize / deserialize functions to handle other formats */
    memcpy(bitmap->bitmap_header->id, data, sizeof(uint8_t) * 2);
    memcpy(&bitmap->bitmap_header->size_of_file, data + 2u,
           sizeof(uint32_t));
    memcpy(&bitmap->bitmap_header->unused, data + 6u, sizeof(uint32_t));
    memcpy(&bitmap->dib_header_24bit->data_offset, data + 10u,
           sizeof(uint16_t));
    memcpy(&bitmap->dib_header_24bit->dib_header_size, data + 14u,
           sizeof(uint32_t));
    memcpy(&bitmap->dib_header_24bit->width, data + 18u, sizeof(uint32_t));
    memcpy(&bitmap->dib_header_24bit->height, data + 22u,
           sizeof(uint32_t));
    memcpy(&bitmap->dib_header_24bit->nr_of_colour_planes, data + 26u,
           sizeof(uint16_t));
    memcpy(&bitmap->dib_header_24bit->resolution, data + 28u,
           sizeof(uint32_t));

    memcpy(&bitmap->dib_header_24bit->compression_type, data + 30u,
           sizeof(uint32_t));
    memcpy(&bitmap->dib_header_24bit->raw_size, data + 34u,
           sizeof(uint32_t));
    memcpy(&bitmap->dib_header_24bit->pixels_horisontal, data + 38u,
           sizeof(uint32_t));
    memcpy(&bitmap->dib_header_24bit->pixels_vertical, data + 42u,
           sizeof(uint32_t));
    memcpy(&bitmap->dib_header_24bit->nr_of_colours_in_palette, data + 46u,
           sizeof(uint32_t));
    memcpy(&bitmap->dib_header_24bit->nr_of_important_colours, data + 50u,
           sizeof(uint32_t));

    if (NULL != bitmap->bitmap_data) {
        free(bitmap->bitmap_data);
    }
    uint32_t data_size =
        calculate_raw_size(bitmap->dib_header_24bit->width,
                           bitmap->dib_header_24bit->height,
                           bitmap->dib_header_24bit->resolution);
    bitmap->bitmap_data = malloc(data_size);
    if (NULL == bitmap->bitmap_data) {
        free(data);
        return MEMORY_ALLOCATION_FAILED;
    }
    /* TODO: Replace constant, beceause the size of the header */
    /* differs between versions of bitmaps */
    memcpy(bitmap->bitmap_data, data + 54u, data_size);

    free(data);
    return NO_ERROR;
}

error_codes bitmap_write_to_file(struct bitmap_definition_t* bitmap,
                                 char* filename) {

    FILE* handler = fopen(filename, "wb");
    /* TODO: Need to create serialize / deserialize functions to handle other formats */
    fwrite(bitmap->bitmap_header->id, 2, sizeof(uint8_t), handler);
    fwrite(&bitmap->bitmap_header->size_of_file, 1, sizeof(uint32_t),
           handler);
    fwrite(&bitmap->bitmap_header->unused, 1, sizeof(uint32_t), handler);
    fwrite(&bitmap->dib_header_24bit->data_offset, 1, sizeof(uint32_t),
           handler);
    fwrite(&bitmap->dib_header_24bit->dib_header_size, 1, sizeof(uint32_t),
           handler);
    fwrite(&bitmap->dib_header_24bit->width, 1, sizeof(uint32_t), handler);
    fwrite(&bitmap->dib_header_24bit->height, 1, sizeof(uint32_t),
           handler);
    fwrite(&bitmap->dib_header_24bit->nr_of_colour_planes, 1,
           sizeof(uint16_t), handler);
    fwrite(&bitmap->dib_header_24bit->resolution, 1, sizeof(uint16_t),
           handler);
    fwrite(&bitmap->dib_header_24bit->compression_type, 1,
           sizeof(uint32_t), handler);
    fwrite(&bitmap->dib_header_24bit->raw_size, 1, sizeof(uint32_t),
           handler);
    fwrite(&bitmap->dib_header_24bit->pixels_horisontal, 1,
           sizeof(uint32_t), handler);
    fwrite(&bitmap->dib_header_24bit->pixels_vertical, 1, sizeof(uint32_t),
           handler);
    fwrite(&bitmap->dib_header_24bit->nr_of_colours_in_palette, 1,
           sizeof(uint32_t), handler);
    fwrite(&bitmap->dib_header_24bit->nr_of_important_colours, 1,
           sizeof(uint32_t), handler);

    uint32_t data_size =
        calculate_raw_size(bitmap->dib_header_24bit->width,
                           bitmap->dib_header_24bit->height,
                           bitmap->dib_header_24bit->resolution);
    fwrite(bitmap->bitmap_data, data_size, sizeof(uint8_t), handler);
    fclose(handler);

    return NO_ERROR;
}

error_codes bitmap_set_pixel(struct bitmap_definition_t* bitmap,
                             uint32_t xpos, uint32_t ypos, uint32_t red,
                             uint32_t green, uint32_t blue,
                             uint32_t alpha) {

    error_codes result = NO_ERROR;

    if (xpos > bitmap_get_width(bitmap)) {
        return X_POSITION_TOO_HIGH;
    }
    if (ypos > bitmap->dib_header_24bit->height) {
        return Y_POSITION_TOO_HIGH;
    }

    if (24 == bitmap->dib_header_24bit->resolution) {
        /* calculate pos, incl. padding */
        uint32_t pos = calculate_pos_24bit(xpos,
                                           bitmap->dib_header_24bit->
                                           height - ypos - 1,
                                           bitmap_get_width(bitmap));
        result = set_pixel_24bit(bitmap, pos, red, green, blue);
    } else {
        result = UNSUPPORTED_RESOLUTION;
    }

    return result;
}

error_codes bitmap_get_pixel(struct bitmap_definition_t* bitmap,
                             uint32_t xpos, uint32_t ypos, uint32_t * red,
                             uint32_t * green, uint32_t * blue,
                             uint32_t * alpha) {

    /* check size of bitmap */
    if (xpos > bitmap_get_width(bitmap)) {
        return X_POSITION_TOO_HIGH;
    }
    if (ypos > bitmap->dib_header_24bit->height) {
        return Y_POSITION_TOO_HIGH;
    }

    if (24 == bitmap->dib_header_24bit->resolution) {
        uint32_t pos = calculate_pos_24bit(xpos,
                                           bitmap->dib_header_24bit->
                                           height - ypos - 1,
                                           bitmap_get_width(bitmap));
        get_pixel_24bit(bitmap, pos, red, green, blue);
    }

    return NO_ERROR;
}

error_codes bitmap_fill(struct bitmap_definition_t* bitmap, uint32_t red,
                        uint32_t green, uint32_t blue) {

    /* to keep track of when to add padding */
    uint32_t xpos = 0;
    uint32_t pos = 0;
    uint8_t padding = bitmap_get_width(bitmap) % 4;
    uint32_t data_size = calculate_raw_size(bitmap_get_width(bitmap),
                                            bitmap->dib_header_24bit->
                                            height,
                                            bitmap->dib_header_24bit->
                                            resolution);
    while (pos < data_size) {
        if (24 == bitmap->dib_header_24bit->resolution) {
            set_pixel_24bit(bitmap, pos, red, green, blue);
            pos += 3;
            xpos += 3;
        }

        /* padding at end of each row */
        if (xpos %
            (bitmap_get_width(bitmap) *
             bitmap->dib_header_24bit->resolution / 8) == 0) {
            pos += padding;
            xpos = 0;
        }
    }

    return NO_ERROR;
}

error_codes bitmap_draw_line(struct bitmap_definition_t* bitmap,
                             uint32_t x1, uint32_t y1, uint32_t x2,
                             uint32_t y2, uint32_t red, uint32_t green,
                             uint32_t blue, uint32_t alpha) {

    line_directions_t dir = get_direction(x1, y1, x2, y2);
    int8_t x_inc = get_x_inc(x1, y1, x2, y2);
    float gradient = calc_gradient(dir, x1, y1, x2, y2);
    float fy1 = (float) y1;

    do {
        bitmap_set_pixel(bitmap, x1, (uint32_t) (fy1 + 0.5f),
                         red, green, blue, alpha);
        x1 += x_inc;
        fy1 = fy1 + gradient;
    } while (!line_last_pixel(dir, x1, fy1, x2, (float) y2));

    return 0;
}

error_codes bitmap_draw_rectangle(struct bitmap_definition_t* bitmap,
                                  uint32_t x1, uint32_t y1, uint32_t x2,
                                  uint32_t y2, uint32_t red,
                                  uint32_t green, uint32_t blue,
                                  uint32_t alpha) {

    if (x1 > bitmap_get_width(bitmap) || x2 > bitmap_get_width(bitmap)) {
        return X_POSITION_TOO_HIGH;
    }

    if (y1 > bitmap->dib_header_24bit->height
        || y2 > bitmap->dib_header_24bit->height) {
        return Y_POSITION_TOO_HIGH;
    }

    while (y1 < y2) {
        bitmap_draw_line(bitmap, x1, y1, x2, y1, red, green, blue, alpha);
        y1++;
    }

    return NO_ERROR;
}

error_codes bitmap_draw_triangle(struct bitmap_definition_t* bitmap,
                                 uint32_t x1, uint32_t y1, uint32_t x2,
                                 uint32_t y2, uint32_t x3, uint32_t y3,
                                 uint32_t red, uint32_t green,
                                 uint32_t blue, uint32_t alpha) {

    /* draw a line from coordinates 1 and coordinates 3 */
    line_directions_t dir = get_direction(x1, y1, x3, y3);

    float gradient = triangle_x_gradient(dir, x1, y1, x3, y3);
    uint32_t x = x1;
    bool verticle_dir = y1 < y2;

    /* for each row, draw a line from y2 to the previous created line */
    do {
        bitmap_draw_line(bitmap, x1, y1, x, y1, red, green, blue, alpha);
        x += (gradient + 0.5f);
        if (0 == x) {
            break;
        }

        if (verticle_dir) {
            y1++;
        } else {
            y1--;
        }
    } while (!triangle_last_line(y1, y2, verticle_dir));

    return NO_ERROR;
}

error_codes bitmap_draw_ellipse(struct bitmap_definition_t* bitmap,
                                uint32_t xpos, uint32_t ypos,
                                uint32_t width, uint32_t height,
                                double start_radius, double end_radius,
                                uint32_t red, uint32_t green,
                                uint32_t blue, uint32_t alpha) {

    int32_t current_xpos = (int32_t) width / 2;
    current_xpos *= -1;
    uint32_t xstart = xpos - width / 2;
    uint32_t xend = xpos + width / 2;

    while (xstart < xend) {
        uint32_t current_height = get_ellipse_y_coordinate(height / 2,
                                                           (uint32_t)
                                                           abs
                                                           (current_xpos),
                                                           width / 2);
        uint32_t y1 = ypos - current_height;
        uint32_t y2 = ypos + current_height;
        uint32_t current_ypos = y1;
        while (current_ypos < y2) {
            /* FUTURE VERSION: exclude pixels not within the start & end radius */
            bitmap_set_pixel(bitmap, xstart, current_ypos, red, green,
                             blue, alpha);
            current_ypos++;
        }
        xstart++;
        current_xpos++;
    }

    return NO_ERROR;
}

error_codes bitmap_insert_bitmap(struct bitmap_definition_t* bitmap,
                                 uint32_t dest_x, uint32_t dest_y,
                                 struct bitmap_definition_t* new_bitmap,
                                 uint32_t source_x1, uint32_t source_y1,
                                 uint32_t source_x2, uint32_t source_y2) {

    if ((bitmap_get_width(bitmap) < bitmap_get_width(new_bitmap)) ||
        (bitmap->dib_header_24bit->height <
         new_bitmap->dib_header_24bit->height)) {
        return INCORRECT_SIZE;
    }

    if (dest_x > bitmap_get_width(bitmap)) {
        return X_POSITION_TOO_HIGH;
    }

    if (dest_y > bitmap->dib_header_24bit->height) {
        return Y_POSITION_TOO_HIGH;
    }

    if (bitmap->dib_header_24bit->resolution !=
        new_bitmap->dib_header_24bit->resolution) {
        return RESOLUTION_DIFFER;
    }

    if ((bitmap_get_width(bitmap) <
         (bitmap_get_width(new_bitmap) + source_x1))
        || (bitmap->dib_header_24bit->height <
            (new_bitmap->dib_header_24bit->height + source_y1))) {
        return INCORRECT_SIZE;
    }

    while (source_y1 < source_y2) {
        uint32_t dx = dest_x;
        uint32_t sx = source_x1;
        while (sx < source_x2) {
            uint32_t red, green, blue, alpha;
            bitmap_get_pixel(new_bitmap, sx, source_y1, &red, &green,
                             &blue, &alpha);
            bitmap_set_pixel(bitmap, dx, dest_y, red, green, blue, alpha);
            dx++;
            sx++;
        }
        dest_y++;
        source_y1++;
    }

    return NO_ERROR;
}

uint32_t bitmap_get_height(struct bitmap_definition_t* bitmap) {
    if (NULL != bitmap && NULL != bitmap->dib_header_24bit) {
        return bitmap->dib_header_24bit->height;
    }
    return 0ul;
}

uint32_t bitmap_get_width(struct bitmap_definition_t* bitmap) {
    if (NULL != bitmap && NULL != bitmap->dib_header_24bit) {
        return bitmap->dib_header_24bit->width;
    }
    return 0ul;
}

/************************************************************************/
/********************* PRIVATE FUNCTIONS ********************************/
/************************************************************************/

/*
 * Allocates a bitmap header struct.
 */
static struct bitmap_header_t* create_bitmap_header() {
    struct bitmap_header_t* bitmap_header =
        malloc(sizeof(struct bitmap_header_t));
    if (NULL == bitmap_header) {
        return NULL;
    }

    bitmap_header->id[0] = 'B';
    bitmap_header->id[1] = 'M';
    bitmap_header->size_of_file = 0ul;
    bitmap_header->unused = 0;

    return bitmap_header;
}

static struct dib_header_24bit* create_dib_header_24bit(uint32_t width,
                                                        uint32_t height) {

    struct dib_header_24bit* header;

    header = malloc(sizeof(struct dib_header_24bit));
    if (NULL == header) {
        return NULL;
    }

    /* Offset is size of Bitmap header and DIB header */
    header->data_offset = 54ul;

    /* Setup the DIB header */
    uint32_t raw_size = calculate_raw_size(width,
                                           height,
                                           24);
    header->raw_size = raw_size;

    /* Header size is always 40 byte in this version of bitmaps */
    header->dib_header_size = 40ul;

    header->width = width;
    header->height = height;
    header->nr_of_colour_planes = 1u;
    header->resolution = 24;
    header->compression_type = 0u;
    header->pixels_horisontal = 11811u;
    header->pixels_vertical = 11811u;
    header->nr_of_colours_in_palette = 0u;
    header->nr_of_important_colours = 0u;


    return header;
}

/*
 * Read all the data from a given file, into an dynamically allocated buffer. 
 * This buffer must be deallocated by the caller after the data has been used.
 */
static uint8_t* read_data_from_file(char* filename) {

    /* get the size of the file */
    FILE* handler = fopen(filename, "rb");
    if (NULL == handler) {
        fprintf(stderr, "Error opening file\n");
        return NULL;
    }
    fseek(handler, 0L, SEEK_END);
    long int size = ftell(handler);
    rewind(handler);

    uint8_t* data = malloc(size);
    if (NULL == data) {
        fclose(handler);
        return NULL;
    }

    fread(data, 1, size, handler);
    fclose(handler);
    return data;
}

/**
 * Set all the colours of a pixel in the given position. 
 */
static error_codes set_pixel_24bit(struct bitmap_definition_t* bitmap,
                                   uint32_t pos, uint32_t red,
                                   uint32_t green, uint32_t blue) {

    bitmap->bitmap_data[pos] = blue & 0xff;
    bitmap->bitmap_data[pos + 1] = green & 0xff;
    bitmap->bitmap_data[pos + 2] = red & 0xff;

    return NO_ERROR;
}

error_codes get_pixel_24bit(struct bitmap_definition_t* bitmap,
                            uint32_t pos, uint32_t * red,
                            uint32_t * green, uint32_t * blue) {

    *blue = (uint32_t) (bitmap->bitmap_data[pos]);
    *green = (uint32_t) (bitmap->bitmap_data[pos + 1]);
    *red = (uint32_t) (bitmap->bitmap_data[pos + 2]);

    return NO_ERROR;
}

/**
 * Calculates the position in the array, including padding
 * position 0,0 is in the lower left corner, when it should be upper left.
 * Only valid for 24bit bitmaps.
 */
static uint32_t calculate_pos_24bit(uint32_t xpos, uint32_t ypos,
                                    uint32_t width) {

    /* Padding increases the size of the row to a multiple of 4 */
    uint8_t padding = width % 4;
    uint8_t bytes_per_pixel = 24 / 8;
    uint32_t pos =
        (width * bytes_per_pixel + padding) * ypos +
        xpos * bytes_per_pixel;
    return pos;
}

/**
 * This version only supports 24bit bitmaps.
 */
static uint16_t get_resolution_int(resolutions_t resolution) {

    switch (resolution) {
    case RGB24:
        return 24;
    default:
        return 0xFFFF;
    }

}

/**
 * Returns the width of the bitmap, including padding.
 */
static uint32_t calculate_bitmap_width(uint32_t width, uint32_t resolution) {
    uint8_t padding = width % 4;
    uint32_t bitmap_width = (width * (resolution / 8) + padding);
    return bitmap_width;
}

/**
 * Returns the size of the pixel data.
 */
static uint32_t calculate_raw_size(uint32_t width, uint32_t height,
                                   uint32_t resolution) {
    uint32_t width_padding = calculate_bitmap_width(width, resolution);
    uint32_t raw_size = width_padding * height;
    return raw_size;
}

/**
 * Returns the direction between two given points.
 */
static line_directions_t get_direction(uint32_t x1, uint32_t y1,
                                       uint32_t x2, uint32_t y2) {
    if (x1 < x2 && y1 < y2) {
        return LEFT_TO_RIGHT_UP_DOWN;
    } else if (x1 < x2 && y1 > y2) {
        return LEFT_TO_RIGHT_DOWN_UP;
    } else if (x1 > x2 && y1 < y2) {
        return RIGHT_TO_LEFT_UP_DOWN;
    } else if (x1 > x2 && y1 > y2) {
        return RIGHT_TO_LEFT_DOWN_UP;
    } else if (x1 == x2 && y1 < y2) {
        return UP_DOWN;
    } else if (x1 == x2 && y1 > y2) {
        return DOWN_UP;
    } else if (x1 < x2 && y1 == y2) {
        return LEFT_RIGHT;
    } else if (x1 > x2 && y1 == y2) {
        return RIGHT_LEFT;
    } else {
        return NO_DIRECTION;
    }
}

static float triangle_x_gradient(line_directions_t dir, uint32_t x1,
                                 uint32_t y1, uint32_t x2, uint32_t y2) {
    float fx1 = (float) x1;
    float fx2 = (float) x2;
    float fy1 = (float) y1;
    float fy2 = (float) y2;
    float gradient = 1.0f;
    if (y2 != y1) {
        gradient = (fx2 - fx1) / (fy2 - fy1);
    } else if (x1 < x2) {
        return 1.0f;
    } else if (x1 > x2) {
        return -1.0f;
    }

    if (LEFT_TO_RIGHT_DOWN_UP == dir || DOWN_UP == dir) {
        gradient *= -1;
    }
    return gradient;
}

static float calc_gradient(line_directions_t dir, uint32_t x1, uint32_t y1,
                           uint32_t x2, uint32_t y2) {
    float fx1 = (float) x1;
    float fx2 = (float) x2;
    float fy1 = (float) y1;
    float fy2 = (float) y2;
    float gradient = 1.0f;
    if (x2 != x1) {
        gradient = (fy2 - fy1) / (fx2 - fx1);
    } else if (y1 < y2) {
        return 1.0f;
    } else if (y1 > y2) {
        return -1.0f;
    }

    if (RIGHT_TO_LEFT_UP_DOWN == dir || RIGHT_TO_LEFT_DOWN_UP == dir ||
        DOWN_UP == dir) {
        gradient *= -1;
    }
    return gradient;
}

static int8_t get_x_inc(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2) {
    if (x1 < x2) {
        return 1;
    } else if (x1 > x2) {
        return -1;
    } else {
        return 0;
    }
}

/* draw_line helper function */
static bool line_last_pixel(line_directions_t dir, uint32_t x1, float fy1,
                            uint32_t x2, float fy2) {
    switch (dir) {
    case LEFT_TO_RIGHT_UP_DOWN:
    case LEFT_TO_RIGHT_DOWN_UP:
    case LEFT_RIGHT:
        return (x1 >= x2);
    case RIGHT_TO_LEFT_UP_DOWN:
    case RIGHT_TO_LEFT_DOWN_UP:
    case RIGHT_LEFT:
        return (x1 <= x2);
    case UP_DOWN:
        return (fy1 >= fy2);
    case DOWN_UP:
        return (fy1 <= fy2);
    case NO_DIRECTION:
    case DIR_COUNT:
    default:
        return true;
    }
}

static bool triangle_last_line(uint32_t y1, uint32_t y2, bool dir) {
    bool last_line = false;
    if (dir) {
        last_line = y1 >= y2;
    } else {
        last_line = y1 <= y2;
    }
    return last_line;
}

static uint32_t get_ellipse_y_coordinate(uint32_t height, uint32_t xpos,
                                         uint32_t width) {
    /* sqrt(height^2 * (1 - xpos^2/width^2)) */
    double x2 = xpos * xpos;
    double w2 = width * width;
    double h2 = height * height;
    double root = abs(h2 * (1 - x2 / w2));
    double y2 = sqrt(root);
    return (uint32_t) (y2 + 0.5f);
}
