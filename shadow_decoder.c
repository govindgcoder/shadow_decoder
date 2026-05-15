#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>

int isValidChar(int ch) {
    if (isalnum(ch)) return 1;
    if (ch == '+' || ch == '/' || ch == '=') return 1;
    return 0;
}

char* acceptUserInput(char *initial_buffer, size_t *capacity, size_t *length) {
    char *buffer = initial_buffer;
    int ch;
    while ((ch = getchar()) != EOF) {
        if ((*length) + 1 >= *capacity) {
            size_t new_cap = (*capacity) * 2;
            char *temp = buffer;
            temp = realloc(temp, new_cap);
            if (temp == NULL) {
                printf("reallocation failed!!!\n");
                free(buffer);
                return NULL; 
            }
            buffer = temp;
            *capacity = new_cap; 
        }
        if (isValidChar(ch)) buffer[(*length)++] = (char)ch;
    }
    buffer[*length] = '\0';
    return buffer;
}

uint8_t* decodeB64(char *buffer, uint8_t *binary_buffer, size_t *capacity, size_t *binary_length, size_t length) {
    size_t cursor = 0;
    if (length % 4 != 0) { printf("invalid input length!\n"); free(binary_buffer); return NULL; }

    while (cursor < length) {
        char c1 = buffer[cursor], c2 = buffer[cursor + 1], c3 = buffer[cursor + 2], c4 = buffer[cursor + 3];
        cursor += 4;
        
        size_t paddings = (c4 == '=') ? (c3 == '=' ? 2 : 1) : 0;
        uint32_t packed = 0;

        for (int i = 0; i < 4; i++) {
            char val = (i == 0) ? c1 : (i == 1) ? c2 : (i == 2) ? c3 : c4;
            uint16_t b64_val = 0;
            if (val >= 'A' && val <= 'Z') b64_val = (val - 'A');
            else if (val >= 'a' && val <= 'z') b64_val = (val - 'a' + 26);
            else if (val >= '0' && val <= '9') b64_val = (val - '0' + 52);
            else if (val == '+') b64_val = 62;
            else if (val == '/') b64_val = 63;
            else b64_val = 0;
            
            packed = (packed << 6) | b64_val;
        }

        for (int i = 0; i < 3 - (int)paddings; i++) {
            if ((*binary_length) + 2 >= *capacity) {
                size_t new_cap = (*capacity) * 2;
                uint8_t *temp = realloc(binary_buffer, new_cap);
                if (temp == NULL) {free(binary_buffer); return NULL;}
                binary_buffer = temp;
                *capacity = new_cap;
            }
            binary_buffer[(*binary_length)++] = (uint8_t)((packed >> (2 - i) * 8) & 0xFF);
        }
    }
    return binary_buffer;
}

#pragma pack(push, 1)
typedef struct BMPHeader {
    uint16_t type;
    uint32_t size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset;
} BMPHeader;
#pragma pack(pop)

size_t extractPixelDataOffset(uint8_t *binary_buffer, size_t length) {
    if (length < sizeof(BMPHeader)) return 0;
    BMPHeader header;
    memcpy(&header, binary_buffer, sizeof(BMPHeader));
    if (header.type == 0x4D42 && header.offset < length) return (size_t)header.offset;
    return 0;
}

int main(int argc, char *argv[]) {
    size_t capacity = 256, length = 0;
    char *buffer = malloc(capacity);
    if (!buffer) return 1;
    buffer = acceptUserInput(buffer, &capacity, &length);
    if(!buffer) return 1;
    
    size_t bin_cap = 256, bin_len = 0;
    uint8_t *binary_buffer = malloc(bin_cap);
    if (!binary_buffer) { free(buffer); return 1; }
    uint8_t *temp_buffer = binary_buffer;
    binary_buffer = decodeB64(buffer, binary_buffer, &bin_cap, &bin_len, length);
    if (binary_buffer==NULL) { temp_buffer=NULL; free(buffer); return 1; }

    size_t pixel_data_offset = extractPixelDataOffset(binary_buffer, bin_len);

    if (argc < 2) return 1; 
    char *endptr;

    long argv_val = strtol(argv[1], &endptr, 10);

    if (pixel_data_offset > 0 && argv_val >=0) {
        size_t hidden_offset = (size_t) argv_val;
        if (pixel_data_offset + hidden_offset < bin_len) {
            uint8_t *target = binary_buffer + pixel_data_offset + hidden_offset;
            size_t max_len = bin_len - (pixel_data_offset + hidden_offset);
            for(size_t i = 0; i < max_len && target[i] != '\0'; i++) {  
              putchar(target[i]);
            }
        } else {
            fprintf(stderr, "Offset outside buffer\n");
        }
    }

    free(buffer);
    free(binary_buffer);
    return 0;
}
