/* dec2base.c                 12.01.25                      jim adams
 *
 * Last updated 07.03.26
 *
 * Converts arbitrarily large decimal integers to hex AND binary using
 * string arithmetic (repeated division by 16). There is no compiled-in
 * upper limit on input size; the only practical ceiling is available
 * memory. Each hex digit expands to a 4-bit group, so the binary output
 * inherits the same unbounded range.
 *
 * Build:  gcc -Wall -std=c99 -m64 -O2 -o dec2base dec2base.c
 *   (or:  make release)
 *
 * Usage:  ./dec2base <decimal_string>
 *
 * Example:
 *   $> ./dec2base 255
 *
 *   Hex:
 *   00ff
 *
 *   Bin:
 *   1111 1111
 *
 * Output wraps at 16 four-char groups (79 columns) per line so large
 * values stay within an 80-column terminal or printout.
 *
 * Verification: the test harness (test-dec2base.sh) has been run against
 * 10,000 random values in the range [0, 2^512], validated against Python's
 * hex() reference. Correctness beyond 2^512 is expected from the algorithm
 * but has not been tested.
 *
 */

#define _POSIX_C_SOURCE 200809L     // We're compiling with -std=c99

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h> // For isdigit

/* Helper function to perform division of a large number represented as a str,
 * by a small integer. 
 *
 * It modifies the number string in-place to be the quotient and returns the
 * remainder.
 */
int divide_string_by_16(char* number_str) {
   int remainder = 0;

   /* Divide by 16 stripping the remainder off ea. sucessive quotient...*/
   for (size_t i = 0; number_str[i] != '\0'; ++i) {
      // conv digit char to a digit...
      int digit = number_str[i] - '0';
      int value = remainder * 10 + digit;
      number_str[i] = (value / 16) + '0';
      remainder = value % 16;
   }
   return remainder;
}

// Helper function to remove leading zeros from a string.
void strip_leading_zeros(char* str) {
   size_t i = 0;
   while (str[i] == '0') {
      i++;
   }
   if (i > 0) {
      memmove(str, str + i, strlen(str + i) + 1);
   }
}

/**
 * Converts an arbitrarily long decimal string to a hexadecimal string.
 *
 * decimal_str The null-terminated decimal string to convert.
 *
 * #define _POSIX_C_SOURCE 200809L
 * 
 * compiling with -std=c99
 *
 * returns a newly allocated hex string (lowercase). The caller is responsible
 * for freeing this memory. Returns NULL on failure.
 **/

char* decimal_to_hex_string_bigint(const char* decimal_str) {
   if (decimal_str == NULL) {
      return NULL;
   }

   if (decimal_str[0] == '\0') {
      fprintf(stderr, "Error: Empty input string.\n");
      return NULL;
   }

   // Validate input: check if all characters are digits
   for(size_t i = 0; decimal_str[i] != '\0'; i++) {
      if (!isdigit((unsigned char)decimal_str[i])) {
         fprintf(stderr, "Error: Input contains non-digit characters: %s\n",\
                 decimal_str);
  
         return NULL;
       }
   }

   // Handle the special case of 0
   if (strcmp(decimal_str, "0") == 0) {
      char* hex_str = (char*)malloc(2);
      if (hex_str) 
         strcpy(hex_str, "0");
       
      return hex_str;
   }

   // Create a mutable copy of the decimal string for division
   char* temp_dec = strdup(decimal_str);
   if (temp_dec == NULL) {
      fprintf(stderr, "Error: Memory allocation failed.\n");
      return NULL;
    }

    // Allocate a buffer for the hex string. A safe upper bound is the number
    // of decimal digits.
    char* hex_str = (char*)malloc(strlen(decimal_str) + 1);
    if (hex_str == NULL) {
       fprintf(stderr, "Error: Memory allocation failed.\n");
       free(temp_dec);
       return NULL;
    }

    size_t hex_pos = 0;
    while (strlen(temp_dec) > 0 && strcmp(temp_dec, "0") != 0) {
       int remainder = divide_string_by_16(temp_dec);
       strip_leading_zeros(temp_dec);

       // Convert remainder (0-15) to a hex character
       if (remainder < 10) {
          hex_str[hex_pos++] = remainder + '0';      // 0-9 ascii 48-57 dec
       } else {
          hex_str[hex_pos++] = remainder - 10 + 'a'; // a-f ascii 97-102 dec
       }
    }
    hex_str[hex_pos] = '\0';
    free(temp_dec);

    // The hex string is generated in reverse order, so we need to reverse it.
    for (size_t i = 0; i < (hex_pos >> 1); ++i) {
       char temp = hex_str[i];
       hex_str[i] = hex_str[hex_pos - 1 - i];
       hex_str[hex_pos - 1 - i] = temp;
    }
    return hex_str;
}

/* Number of 4-char groups per line of output. 16 groups = 79 columns
 * (16*4 chars + 15 spaces), one under the 80-column budget. Applies to
 * both hex (4 hex digits/group) and binary (4 bits/group).
 */
#define GROUPS_PER_LINE 16

/* Formats the lowercase hex string as space-separated 4-digit groups under
 * a "Hex:" label, wrapped at GROUPS_PER_LINE groups per line so no line
 * exceeds 80 columns. Leading zeros pad the value out to a full 4-digit
 * group.
 *
 * Returns a newly allocated string (caller frees) or NULL on failure.
 */
char* format_hex_with_padding(const char* input_hex) {
   if (input_hex == NULL) return NULL;

   size_t input_len = strlen(input_hex);
   if (input_len == 0) {
      char* empty_str = (char*)malloc(8);
      if (empty_str) strcpy(empty_str, "Hex:\n");
      return empty_str;
   }

   size_t remainder = input_len % 4;
   size_t padding_needed = (remainder == 0) ? 0 : (4 - remainder);
   size_t total_hex_chars = input_len + padding_needed;   // multiple of 4
   size_t num_groups = total_hex_chars / 4;

   /* "Hex:\n" = 5, total_hex_chars digits, up to (num_groups-1)
    * separators, NUL = 1. The extra margin keeps this safe.
    */
   char* out = (char*)malloc(5 + total_hex_chars + num_groups + 2);
   if (out == NULL) {
      fprintf(stderr, "Error: Memory allocation, failed.\n");
      return NULL;
   }

   strcpy(out, "Hex:\n");
   size_t pos = 5;
   size_t input_pos = 0;
   size_t group = 0;
   for (size_t i = 0; i < total_hex_chars; ++i) {
      out[pos++] = (i < padding_needed) ? '0' : input_hex[input_pos++];

      if ((i + 1) % 4 == 0) {              // finished a 4-digit group
         group++;
         if (group < num_groups) {         // separator before next group
            if (group % GROUPS_PER_LINE == 0)
               out[pos++] = '\n';          // wrap: 16 groups per line
            else
               out[pos++] = ' ';
         }
      }
   }
   out[pos] = '\0';
   return out;
}

/* Converts the lowercase hex string to a spaced binary string, wrapped at
 * BIN_GROUPS_PER_LINE four-bit groups per line so no line exceeds 80 cols.
 * Each hex digit maps to exactly one 4-bit group, so there is no padding
 * math and the result inherits the bignum property of the hex string.
 *
 * Returns a newly allocated string (caller frees) or NULL on failure.
 */
char* format_bin_with_padding(const char* input_hex) {
   static const char* bin_str[16] = {
      "0000","0001","0010","0011","0100","0101","0110","0111",
      "1000","1001","1010","1011","1100","1101","1110","1111"
   };

   if (input_hex == NULL) return NULL;

   size_t n = strlen(input_hex);            // hex digits = 4-bit groups
   if (n == 0) {
      char* empty_str = (char*)malloc(8);
      if (empty_str) strcpy(empty_str, "Bin: ()");
      return empty_str;
   }

   /* "Bin:\n" = 5, 4 bit chars per group * n, (n-1) separators, NUL = 1.
    * 5n + 8 leaves a safe margin.
    */
   char* out = (char*)malloc(5 * n + 8);
   if (out == NULL) {
      fprintf(stderr, "Error: Memory allocation failed.\n");
      return NULL;
   }

   strcpy(out, "Bin:\n");
   size_t pos = 5;
   for (size_t i = 0; i < n; i++) {
      char c = input_hex[i];
      int nib = (c <= '9') ? c - '0' : c - 'a' + 10;   // lowercase hex
      memcpy(out + pos, bin_str[nib], 4);
      pos += 4;

      if (i < n - 1) {                      // separator after this group
         if ((i + 1) % GROUPS_PER_LINE == 0)
            out[pos++] = '\n';              // wrap: 64 bits per line
         else
            out[pos++] = ' ';
      }
   }
   out[pos] = '\0';
   return out;
}

// Prints the value in both hex and binary, each under its own label.
int main(int argc, char *argv[]) {
   if (argc != 2) {
      fprintf(stderr, "Usage: %s <decimal_string>\n", argv[0]);
      return 1;
   }
   const char* input_str = argv[1];

   char* hex_str = decimal_to_hex_string_bigint(input_str);
   if (hex_str == NULL) {
      return 1; // Error already printed
   }

   char* hex_out = format_hex_with_padding(hex_str);
   if (hex_out == NULL) {
      fprintf(stderr, "Error: Failed to format the hex string.\n");
      free(hex_str);
      return 1;
   }

   char* bin_out = format_bin_with_padding(hex_str);
   free(hex_str);
   if (bin_out == NULL) {
      fprintf(stderr, "Error: Failed to format the bin string.\n");
      free(hex_out);
      return 1;
   }

   printf("\n%s\n\n%s\n\n", hex_out, bin_out);
   free(hex_out);
   free(bin_out);
   return 0;
}
