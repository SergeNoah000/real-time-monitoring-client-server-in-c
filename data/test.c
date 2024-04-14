#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Encode UTF-8 string to ASCII
char *utf8_to_ascii(const char *utf8_str) {
    size_t utf8_len = strlen(utf8_str);
    char *ascii_str = (char *)malloc(3 * utf8_len + 1); // Allocate memory for ASCII string
    if (ascii_str == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    size_t ascii_index = 0;
    for (size_t i = 0; i < utf8_len; i++) {
        // Encode non-ASCII characters as escape sequences in ASCII string
        if (utf8_str[i] >= 0 && utf8_str[i] <= 127) {
            ascii_str[ascii_index++] = utf8_str[i];
        } else {
            sprintf(&ascii_str[ascii_index], "\\x%02X", (unsigned char)utf8_str[i]); // Encode non-ASCII character
            ascii_index += 4; // Move to next position
        }
    }
    ascii_str[ascii_index] = '\0'; // Null-terminate ASCII string
    return ascii_str;
}

// Decode ASCII string to UTF-8
char *ascii_to_utf8(const char *ascii_str) {
    size_t ascii_len = strlen(ascii_str);
    char *utf8_str = (char *)malloc(ascii_len + 1); // Allocate memory for UTF-8 string
    if (utf8_str == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    size_t utf8_index = 0;
    for (size_t i = 0; i < ascii_len; i++) {
        if (ascii_str[i] == '\\' && ascii_str[i + 1] == 'x') {
            char hex_str[3];
            hex_str[0] = ascii_str[i + 2];
            hex_str[1] = ascii_str[i + 3];
            hex_str[2] = '\0';
            int code = strtol(hex_str, NULL, 16);
            utf8_str[utf8_index++] = code;
            i += 3; // Skip the escape sequence
        } else {
            utf8_str[utf8_index++] = ascii_str[i];
        }
    }
    utf8_str[utf8_index] = '\0'; // Null-terminate UTF-8 string
    return utf8_str;
}

int main() {
    const char *utf8_str = "fichier_avec_accents_éà.txt et ê";
    printf("Original UTF-8 string: %s\n", utf8_str);

    char *ascii_str = utf8_to_ascii(utf8_str);
    printf("Encoded ASCII string: %s\n", ascii_str);

    const char *ascii_str2 = "fichier_avec_accents_\xC3\xA9\xC3\xA0.txt";
    char *utf8_str2 = ascii_to_utf8(ascii_str);
    printf("Decoded UTF-8 string: %s\n", utf8_str2);
    free(utf8_str2);
    free(ascii_str);

    return 0;
}




