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

#ifndef BITMAP_H
#define BITMAP_H

#define MAJOR_VERSION 1
#define MINOR_VERSION 0

/* This struct is defined in the source file. It shall not be access by users. */
struct bitmap_definition_t;

typedef enum {
    RGB1,
    RGB2,
    RGB16,
    RGB24,
    RGBA24,                     /* include an 8-bit alpha channel */
    RGB_COUNT
} resolutions_t;

/**
 * Allocates a bitmap object with default values. If the allocation
 * failes, NULL is returned.
 * @param width The width of the bitmap in pixels.
 * @param height The height of the bitmap in pixels.
 * @param resolution The resolution of the bitmap. Can be either 1, 4, 24 or 32 bits. Current version only supports 24 bit though, but future version will support more.
 * @return A pointer to a bitmap, or NULL if allocation failed.
 */
struct bitmap_definition_t* bitmap_create(uint32_t width, uint32_t height,
                                          resolutions_t resolution);

/**
 * Deallocates the given bitmap. After this function is called, the 
 * pointer should be set to NULL.
 * @param bitmap The bitmap to deallocate.
 */
void bitmap_destroy(struct bitmap_definition_t* bitmap);

/**
 * Reads the content of a file and copies the data to the given object.
 * Bitmap is expected to already be allocated.
 * @param bitmap The bitmap to read the data to.
 * @param filename The name of the file to read data from.
 * @return NO_ERROR if ok, an error code if anything went wrong.
 */
error_codes bitmap_read_from_file(struct bitmap_definition_t* bitmap,
                                  char* filename);

/**
 * Writes the given object to a file with the given name.
 * @param bitmap The bitmap to save.
 * @param filename The name of the file.
 * @return NO_ERROR if ok, an error code if anything went wrong.
 */
error_codes bitmap_write_to_file(struct bitmap_definition_t* bitmap,
                                 char* filename);

/**
 * Sets every pixels in the given bitmap with the given colour 
 * @param bitmap The bitmap to write data to.
 * @param red The value of red colour.
 * @param green The value of green colour.
 * @param blue The value of blue colour.
 * @return NO_ERROR if ok, an error code if anything went wrong.
 */
error_codes bitmap_fill(struct bitmap_definition_t* bitmap, uint32_t red,
                        uint32_t green, uint32_t blue);

/**
 * Sets a given pixel with the given colours.
 * @param bitmap The bitmap to set a pixel in.
 * @param xpos The row number of place the pixel in.
 * @param ypos The line number to place the pixel in.
 * @param red The value of red colour.
 * @param green The value of green colour.
 * @param blue The value of blue colour.
 * @param alpha The value of the alpha channel. For future versions.
 * @return NO_ERROR if ok, an error code if anything went wrong.
 */
error_codes bitmap_set_pixel(struct bitmap_definition_t* bitmap,
                             uint32_t xpos, uint32_t ypos, uint32_t red,
                             uint32_t green, uint32_t blue,
                             uint32_t alpha);

/**
 * Draws a line between the two given coordinates.
 * @param bitmap The bitmap to draw the figure in.
 * @param x1 The first x coordinate.
 * @param y1 The first y coordinate.
 * @param x2 The second x coordinate.
 * @param y2 The second y coordinate.
 * @param red The value of red colour.
 * @param green The value of green colour.
 * @param blue The value of blue colour.
 * @param alpha The value of the alpha channel. For future versions.
 * @return NO_ERROR if ok, an error code if anything went wrong.
 */
error_codes bitmap_draw_line(struct bitmap_definition_t* bitmap,
                             uint32_t x1, uint32_t y1, uint32_t x2,
                             uint32_t y2, uint32_t red, uint32_t green,
                             uint32_t blue, uint32_t alpha);


/**
 * Draws a filled rectangle between the two given coordinates.
 * @param bitmap The bitmap to draw the figure in.
 * @param x1 The first x coordinate.
 * @param y1 The first y coordinate.
 * @param x2 The second x coordinate.
 * @param y2 The second y coordinate.
 * @param red The value of red colour.
 * @param green The value of green colour.
 * @param blue The value of blue colour.
 * @param alpha The value of the alpha channel. For future versions.
 * @return NO_ERROR if ok, an error code if anything went wrong.
 */ error_codes bitmap_draw_rectangle(struct bitmap_definition_t* bitmap,
                                      uint32_t x1, uint32_t y1,
                                      uint32_t x2, uint32_t y2,
                                      uint32_t red, uint32_t green,
                                      uint32_t blue, uint32_t alpha);

/**
 * Draws a filled triangle between the three given coordinates. One of
 * the corners is always 90 degrees.
 * @param bitmap The bitmap to draw the figure in.
 * @param x1 The first x coordinate.
 * @param y1 The first y coordinate.
 * @param x2 The second x coordinate.
 * @param y2 The second y coordinate.
 * @param x3 The third x coordinate.
 * @param y3 The third y coordinate.
 * @param red The value of red colour.
 * @param green The value of green colour.
 * @param blue The value of blue colour.
 * @param alpha The value of the alpha channel. For future versions.
 * @return NO_ERROR if ok, an error code if anything went wrong.
 */
error_codes bitmap_draw_triangle(struct bitmap_definition_t* bitmap,
                                 uint32_t x1, uint32_t y1, uint32_t x2,
                                 uint32_t y2, uint32_t x3, uint32_t y3,
                                 uint32_t red, uint32_t green,
                                 uint32_t blue, uint32_t alpha);

/**
 * Draws a filled ellipse on the given coordinates and given size.
 * @param bitmap The bitmap to draw the figure in.
 * @param xpos The x coordinate.
 * @param ypos The y coordinate.
 * @param width The with of the ellipse.
 * @param height The height of the ellipse.
 * @param red The value of red colour.
 * @param green The value of green colour.
 * @param blue The value of blue colour.
 * @param alpha The value of the alpha channel. For future versions.
 * @return NO_ERROR if ok, an error code if anything went wrong.
 */
error_codes bitmap_draw_ellipse(struct bitmap_definition_t* bitmap,
				uint32_t xpos, uint32_t ypos, uint32_t width,
				uint32_t height, double start_radius, double end_radius,
				uint32_t red, uint32_t green,
				uint32_t blue, uint32_t alpha);

#endif                          /* BITMAP_H */
