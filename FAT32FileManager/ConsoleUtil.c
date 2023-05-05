#include <stdlib.h>
#include <float.h>
#include <stdint.h>
#include <math.h>
#include "ConsoleUtil.h"

char *get_line_dynamic()
{
    int c;                                           // to store the current character
    char *string = (char *)malloc(sizeof(char) * 8); // allocate the string in blocks of 8

    string[0] = '\0'; // initiate the string to be empty
    int string_length = 0;

    // continually get the input until the user hits enter
    while ((c = getchar()) != '\n' && c != EOF)
    { // reallocate every 8 characters to prevent allocating memory continually (allocation is expensive)
        if ((string_length + 1) % 8 == 0)
            string = realloc(string, (string_length + 8) * sizeof(char)); // reallocating memory
        string[string_length] = (char)c;                                  // get the actual character
        string[string_length + 1] = '\0';                                 // inserting null character at the end
        string_length++;
    }

    return string;
}

uint8_t string_to_long(char *buffer, int64_t *result)
{
    char *nextPtr = NULL;
    int64_t number = strtoll(buffer, &nextPtr, 10);

    if (buffer[0] == '\0' || // input was empty
        nextPtr == buffer || // invalid number
        nextPtr[0] != '\0')  // contained more than a number
    {
        debug("input was not a number");
        return EXIT_FAILURE;
    }
    else if (number >= INT64_MAX || number <= INT64_MIN)
    {
        debug("input was too large to store as a long");
        return EXIT_FAILURE;
    }

    *result = number;
    return EXIT_SUCCESS;
}

uint8_t string_to_int(char *buffer, int32_t *result)
{ // just cast 64-bit int to 32-bit as long as in range
    int64_t number = 0;
    if (!string_to_long(buffer, &number))
    {
        if (number < INT32_MAX && number > INT32_MIN)
        {
            *result = (int32_t)number;
            return EXIT_SUCCESS;
        }
        else
        {
            debug("input was too large to store as int");
        }
    }
    return EXIT_FAILURE;
}

uint8_t string_to_double(char *buffer, double *result)
{
    char *nextPtr = NULL;
    double number = strtod(buffer, &nextPtr);
    if (buffer[0] == '\0' || // input was empty
        nextPtr == buffer || // invalid number
        nextPtr[0] != '\0')  // contained more than a number
    {
        debug("input was not a number");
        return EXIT_FAILURE;
    }
    // if absolute value is not +/-inf & > +/- DBL_MIN
    if ((fabs(number) == HUGE_VAL) ||
        (fabs(number) <= DBL_MIN))
    {
        debug("input was too large to store as a double");
        return EXIT_FAILURE;
    }

    *result = number;
    return EXIT_SUCCESS;
}

uint8_t string_to_float(char *buffer, float *result)
{
    char *nextPtr = NULL;
    float number = strtof(buffer, &nextPtr);
    if (buffer[0] == '\0' || // input was empty
        nextPtr == buffer || // invalid number
        nextPtr[0] != '\0')  // contained more than a number
    {
        debug("input was not a number");
        return EXIT_FAILURE;
    }
    else if (number >= FLT_MAX || number <= -FLT_MAX)
    {
        debug("input was too large to store as a double");
        return EXIT_FAILURE;
    }

    *result = number;
    return EXIT_SUCCESS;
}
