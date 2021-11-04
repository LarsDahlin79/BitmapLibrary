
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "error_codes.h"
#include "bitmap.h"

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
    uint8_t id[2];
    uint32_t size_of_file;
    uint32_t unused;

    /* DIB header, Device Independant Bitmap */
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

    /* the actual pixel data */
    uint8_t* bitmap_data;
};

/* private function headers */
/*********************************************/
static uint8_t* read_data_from_file(char* filename);
static error_codes set_pixel_24bit(struct bitmap_definition_t* bitmap,
                                   uint32_t pos, uint32_t red,
                                   uint32_t green, uint32_t blue);
static uint32_t calculate_pos_24bit(uint32_t xpos, uint32_t ypos,
                                    uint32_t width);
static uint16_t get_resolution_int(resolutions_t resolution);
static uint32_t calculate_raw_size(uint32_t width, uint32_t height,
                                   uint32_t resolution);
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
    bitmap->id[0] = 'B';
    bitmap->id[1] = 'M';
    bitmap->data_offset = 54;
    bitmap->unused = 0ul;

    /* Setup the DIB header */
    uint32_t raw_size = calculate_raw_size(bitmap->width,
					   bitmap->height,
					   bitmap->resolution);
    bitmap->size_of_file = raw_size + bitmap->data_offset;
    bitmap->raw_size = raw_size;

    /* header size is always 40 byte in this version of bitmaps */
    bitmap->dib_header_size = 40ul;

    bitmap->width = width;
    bitmap->height = height;
    bitmap->nr_of_colour_planes = 1u;
    bitmap->resolution = get_resolution_int(resolution);
    bitmap->compression_type = 0u;
    bitmap->pixels_horisontal = 11811u;
    bitmap->pixels_vertical = 11811u;
    bitmap->nr_of_colours_in_palette = 0u;
    bitmap->nr_of_important_colours = 0u;

    uint32_t data_size = calculate_raw_size(bitmap->width,
                                            bitmap->height,
                                            bitmap->resolution);

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
    free(bitmap);
}

error_codes bitmap_read_from_file(struct bitmap_definition_t* bitmap,
                                  char* filename) {

    uint8_t* data = read_data_from_file(filename);
    if (NULL == data) {
        return MEMORY_ALLOCATION_FAILED;
    }

    if (data[0] != 'B' || data[1] != 'M') {
        free(data);
        return MAGIC_NUMBER_MISSING;
    }

    memcpy(bitmap->id, data, sizeof(uint8_t) * 2);
    memcpy(&bitmap->size_of_file, data + 2u, sizeof(uint32_t));
    memcpy(&bitmap->unused, data + 6u, sizeof(uint32_t));
    memcpy(&bitmap->data_offset, data + 10u, sizeof(uint16_t));
    memcpy(&bitmap->dib_header_size, data + 14u, sizeof(uint32_t));
    memcpy(&bitmap->width, data + 18u, sizeof(uint32_t));
    memcpy(&bitmap->height, data + 22u, sizeof(uint32_t));
    memcpy(&bitmap->nr_of_colour_planes, data + 26u, sizeof(uint16_t));
    memcpy(&bitmap->resolution, data + 28u, sizeof(uint32_t));
    if (0xFFFF == get_resolution_int(bitmap->resolution)) {
        free(data);
        return UNSUPPORTED_RESOLUTION;
    }

    memcpy(&bitmap->compression_type, data + 30u, sizeof(uint32_t));
    memcpy(&bitmap->raw_size, data + 34u, sizeof(uint32_t));
    memcpy(&bitmap->pixels_horisontal, data + 38u, sizeof(uint32_t));
    memcpy(&bitmap->pixels_vertical, data + 42u, sizeof(uint32_t));
    memcpy(&bitmap->nr_of_colours_in_palette, data + 46u,
           sizeof(uint32_t));
    memcpy(&bitmap->nr_of_important_colours, data + 50u, sizeof(uint32_t));

    if (NULL != bitmap->bitmap_data) {
        free(bitmap->bitmap_data);
    }
    uint32_t data_size = calculate_raw_size(bitmap->width,
                                            bitmap->height,
                                            bitmap->resolution);
    bitmap->bitmap_data = malloc(data_size);
    if (NULL == bitmap->bitmap_data) {
        free(data);
        return MEMORY_ALLOCATION_FAILED;
    }
    memcpy(bitmap->bitmap_data, data + 54u, data_size);

    free(data);
    return NO_ERROR;
}

error_codes bitmap_write_to_file(struct bitmap_definition_t* bitmap,
                                 char* filename) {

    FILE* handler = fopen(filename, "wb");
    fwrite(bitmap->id, 2, sizeof(uint8_t), handler);
    fwrite(&bitmap->size_of_file, 1, sizeof(uint32_t), handler);
    fwrite(&bitmap->unused, 1, sizeof(uint32_t), handler);
    fwrite(&bitmap->data_offset, 1, sizeof(uint32_t), handler);
    fwrite(&bitmap->dib_header_size, 1, sizeof(uint32_t), handler);
    fwrite(&bitmap->width, 1, sizeof(uint32_t), handler);
    fwrite(&bitmap->height, 1, sizeof(uint32_t), handler);
    fwrite(&bitmap->nr_of_colour_planes, 1, sizeof(uint16_t), handler);
    fwrite(&bitmap->resolution, 1, sizeof(uint16_t), handler);
    fwrite(&bitmap->compression_type, 1, sizeof(uint32_t), handler);
    fwrite(&bitmap->raw_size, 1, sizeof(uint32_t), handler);
    fwrite(&bitmap->pixels_horisontal, 1, sizeof(uint32_t), handler);
    fwrite(&bitmap->pixels_vertical, 1, sizeof(uint32_t), handler);
    fwrite(&bitmap->nr_of_colours_in_palette, 1, sizeof(uint32_t),
           handler);
    fwrite(&bitmap->nr_of_important_colours, 1, sizeof(uint32_t), handler);

    uint32_t data_size = calculate_raw_size(bitmap->width,
                                            bitmap->height,
                                            bitmap->resolution);
    fwrite(bitmap->bitmap_data, data_size, sizeof(uint8_t), handler);
    fclose(handler);

    return NO_ERROR;
}

error_codes bitmap_set_pixel(struct bitmap_definition_t* bitmap,
                             uint32_t xpos, uint32_t ypos, uint32_t red,
                             uint32_t green, uint32_t blue,
                             uint32_t alpha) {

    error_codes result = NO_ERROR;

    if (xpos > bitmap->width) {
        return X_POSITION_TOO_HIGH;
    }
    if (ypos > bitmap->height) {
        return Y_POSITION_TOO_HIGH;
    }

    if (bitmap->resolution == 24) {
        /* calculate pos, incl. padding */
        uint32_t pos = calculate_pos_24bit(xpos,
                                           bitmap->height - ypos - 1,
                                           bitmap->width);
        result = set_pixel_24bit(bitmap, pos, red, green, blue);
    } else {
        result = UNSUPPORTED_RESOLUTION;
    }

    return result;
}

error_codes bitmap_fill(struct bitmap_definition_t* bitmap, uint32_t red,
                        uint32_t green, uint32_t blue) {

    /* to keep track of when to add padding */
    uint32_t xpos = 0;
    uint32_t pos = 0;
    uint8_t padding = bitmap->width % 4;
    uint32_t data_size = calculate_raw_size(bitmap->width,
                                            bitmap->height,
                                            bitmap->resolution);
    while (pos < data_size) {
        if (bitmap->resolution == 24) {
            set_pixel_24bit(bitmap, pos, red, green, blue);
            pos += 3;
            xpos += 3;
        }

        /* padding at end of each row */
        if (xpos % (bitmap->width * bitmap->resolution / 8) == 0) {
            pos += padding;
            xpos = 0;
        }
    }

    return NO_ERROR;
}

/************************************************************************/
/********************* PRIVATE FUNCTIONS ********************************/
/************************************************************************/

/*
 * Read all the data from a given file, into an dynamically allocated buffer. 
 * This buffer must be deallocated by the caller after the data has been used.
 */
static uint8_t* read_data_from_file(char* filename) {

    /* get the size of the file */
    FILE* handler = fopen(filename, "rb");
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

    /* set blue colour */
    bitmap->bitmap_data[pos] = blue & 0xff;

    /* set green colour */
    bitmap->bitmap_data[pos + 1] = green & 0xff;

    /* set red colour */
    bitmap->bitmap_data[pos + 2] = red & 0xff;

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
 * Returns the size of the pixel data.
 */
static uint32_t calculate_raw_size(uint32_t width, uint32_t height,
                                   uint32_t resolution) {
    uint8_t padding = width % 4;
    uint32_t raw_size = (width * (resolution / 8) + padding) * height;
    return raw_size;
}
