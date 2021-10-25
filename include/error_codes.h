
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
