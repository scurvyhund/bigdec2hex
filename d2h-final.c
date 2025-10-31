#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h> // Required for strtol error checking

/**
 * @brief Converts a decimal string to a hexadecimal string.
 *
 * @param decimal_str The null-terminated decimal string to convert.
 * @return A newly allocated hex string (lowercase). The caller is responsible
 *         for freeing this memory. Returns NULL on failure.
 */
char* decimal_to_hex_string(const char* decimal_str) {
    if (decimal_str == NULL) {
        return NULL;
    }

    // Use strtol to convert the decimal string to a long integer.
    char *endptr;
    errno = 0; // Reset errno before call
    long dec_val = strtol(decimal_str, &endptr, 10);

    // Check for conversion errors (no digits converted, out of range).
    if (endptr == decimal_str || *endptr != '\0' || errno == ERANGE) {
        fprintf(stderr, "Error: Invalid decimal number: %s\n", decimal_str);
        return NULL;
    }

    // Allocate a buffer for the hex string. A 64-bit long has at most 16 hex digits.
    char* hex_str = (char*)malloc(17); // 16 digits + null terminator
    if (hex_str == NULL) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return NULL;
    }
    
    // Use snprintf to write the integer as a hex string into the buffer.
    // The "lx" format specifier is for a long in hex (lowercase).
    snprintf(hex_str, 17, "%lx", dec_val);

    return hex_str;
}


/**
 * @brief Formats a hex string with a label, parentheses, leading zero padding, and 4-character spacing.
 *
 * This function takes a null-terminated hexadecimal string, prepends "Hex: (",
 * pads the hex digits with leading zeros so their length is a multiple of 4,
 * inserts a space every 4 characters, and appends a closing ")".
 *
 * @param input_hex The null-terminated input hex string to format.
 * @return A newly allocated, formatted string. The caller is responsible for
 *         freeing this memory with free(). Returns NULL on failure or if input is NULL.
 */
char* format_hex_with_padding(const char* input_hex) {
    if (input_hex == NULL) {
        return NULL;
    }

    size_t input_len = strlen(input_hex);
    if (input_len == 0) {
        char* empty_str = (char*)malloc(8);
        if (empty_str) {
            strcpy(empty_str, "Hex: ()");
        }
        return empty_str;
    }

    size_t remainder = input_len % 4;
    size_t padding_needed = (remainder == 0) ? 0 : (4 - remainder);
    size_t total_hex_chars = input_len + padding_needed;
    size_t num_spaces = (total_hex_chars / 4) - 1;
    size_t visible_len = 6 + total_hex_chars + num_spaces + 1;

    char* formatted_str = (char*)malloc(visible_len + 1);
    if (formatted_str == NULL) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return NULL;
    }

    strcpy(formatted_str, "Hex: (");
    size_t input_pos = 0;
    size_t output_pos = 6;

    for (size_t i = 0; i < total_hex_chars; ++i) {
        char char_to_write;
        if (i < padding_needed) {
            char_to_write = '0';
        } else {
            char_to_write = input_hex[input_pos++];
        }
        formatted_str[output_pos++] = char_to_write;
        if ((i + 1) % 4 == 0 && i < total_hex_chars - 1) {
            formatted_str[output_pos++] = ' ';
        }
    }

    formatted_str[output_pos++] = ')';
    formatted_str[output_pos] = '\0';
    return formatted_str;
}

/**
 * @brief Main function to run the hex formatter from the command line.
 */
int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <decimal_string>\n", argv[0]);
        return 1;
    }

    const char* input_str = argv[1];

    // NEW STEP: Convert the decimal input string to a hex string.
    char* hex_str = decimal_to_hex_string(input_str);
    if (hex_str == NULL) {
        // An error message was already printed by the conversion function.
        return 1;
    }

    // Now, format the resulting hex string.
    char* output_str = format_hex_with_padding(hex_str);
    
    // We're done with the intermediate hex string, so free it.
    free(hex_str);

    if (output_str != NULL) {
        printf("%s\n", output_str);
        free(output_str);
    } else {
        fprintf(stderr, "Error: Failed to format the hex string.\n");
        return 1;
    }

    return 0;
}
