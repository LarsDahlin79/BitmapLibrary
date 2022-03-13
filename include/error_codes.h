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

#ifndef ERROR_CODES_H
#define ERROR_CODES_H

/*
 * Definition of all error codes.
 */
typedef enum {
    NO_ERROR,
    NOT_IMPLEMENTED_YET,
    MEMORY_ALLOCATION_FAILED,
    UNSUPPORTED_RESOLUTION,
    X_POSITION_TOO_HIGH,
    Y_POSITION_TOO_HIGH,
    MAGIC_NUMBER_MISSING,
    ERROR_COUNT
} error_codes;

#endif                          /* ERROR_CODES_H */
