#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h> // For isdigit

// Helper function to perform division of a large number represented as a str,
// by a small integer.
// It modifies the number string in-place to be the quotient and returns the
// remainder.
int divide_string_by_16(char* number_str) {
   int remainder = 0;
   for (int i = 0; number_str[i] != '\0'; ++i) {
      int digit = number_str[i] - '0';
      int value = remainder * 10 + digit;
      number_str[i] = (value / 16) + '0';
      remainder = value % 16;
   }
   return remainder;
}

// Helper function to remove leading zeros from a string.
void strip_leading_zeros(char* str) {
   int i = 0;
   while (str[i] == '0') {
      i++;
   }
   if (i > 0) {
      memmove(str, str + i, strlen(str + i) + 1);
   }
}

/**
 * @brief Converts an arbitrarily long decimal string to a hexadecimal string.
 *
 * @param decimal_str The null-terminated decimal string to convert.
 * @return A newly allocated hex string (lowercase). The caller is responsible
 *         for freeing this memory. Returns NULL on failure.
 */
char* decimal_to_hex_string_bigint(const char* decimal_str) {
   if (decimal_str == NULL) {
      return NULL;
   }

   // Validate input: check if all characters are digits
   for(int i = 0; decimal_str[i] != '\0'; i++) {
      if (!isdigit(decimal_str[i])) {
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

    int hex_pos = 0;
    while (strlen(temp_dec) > 0 && strcmp(temp_dec, "0") != 0) {
       int remainder = divide_string_by_16(temp_dec);
       strip_leading_zeros(temp_dec);

       // Convert remainder (0-15) to a hex character
       if (remainder < 10) {
           hex_str[hex_pos++] = remainder + '0';
       } else {
           hex_str[hex_pos++] = remainder - 10 + 'a';
       }
    }
    hex_str[hex_pos] = '\0';
    free(temp_dec);

    // The hex string is generated in reverse order, so we need to reverse it.
    for (int i = 0; i < hex_pos / 2; ++i) {
       char temp = hex_str[i];
       hex_str[i] = hex_str[hex_pos - 1 - i];
       hex_str[hex_pos - 1 - i] = temp;
    }

    return hex_str;
}


// The format_hex_with_padding func remains the same as last correct version.
char* format_hex_with_padding(const char* input_hex) {
    
  if (input_hex == NULL) return NULL;
   
     size_t input_len = strlen(input_hex);
     if (input_len == 0) { 
        char* empty_str = (char*)malloc(8); 
      
     if(empty_str) strcpy(empty_str, "Hex: ()"); 
        return empty_str; 
  }
  
   size_t remainder = input_len % 4;
   size_t padding_needed = (remainder == 0) ? 0 : (4 - remainder);
   size_t total_hex_chars = input_len + padding_needed;
   size_t num_spaces = (total_hex_chars / 4) - 1;
   size_t visible_len = 6 + total_hex_chars + num_spaces + 1;

   char* formatted_str = (char*)malloc(visible_len + 1);
   if (formatted_str == NULL) { 
      fprintf(stderr, "Error: Memory allocation, failed.\n"); 
      return NULL; 
   }

   strcpy(formatted_str, "Hex: (");
   size_t input_pos = 0;
   size_t output_pos = 6;
   for (size_t i = 0; i < total_hex_chars; ++i) {
      char char_to_write = (i < padding_needed) ? '0' : input_hex[input_pos++];
      formatted_str[output_pos++] = char_to_write;

      if ((i + 1) % 4 == 0 && i < total_hex_chars - 1) {
         formatted_str[output_pos++] = ' ';
      }
   }
   formatted_str[output_pos++] = ')';
   formatted_str[output_pos] = '\0';
   return formatted_str;
}

// The main function is updated to use the new big integer conversion.
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

   char* output_str = format_hex_with_padding(hex_str);
   free(hex_str);

   if (output_str != NULL) {
      printf("\n%s\n\n", output_str);
      free(output_str);
   } else {
      fprintf(stderr, "Error: Failed to format the hex string.\n");
      return 1;
   }

   return 0;
}
