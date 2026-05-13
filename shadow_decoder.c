#include<stdio.h> // for io
#include<stdlib.h> // for memory management and all
#include<stdint.h> // for the int  
#include <ctype.h> // for isalnum()

int isValidChar(int ch) {
    if (isalnum(ch)) {
        return 1; 
    }
    if (ch == '+' || ch == '/' || ch == '=') {
        return 1;
    }
    return 0; 
}  

// accept user input stream and dynamically allocate a buffer to store it
char* acceptUserInput(char *initial_buffer, size_t *capacity, size_t *length){
  char *buffer = initial_buffer;
  int ch;
// accept stream of data 
  while((ch = getchar()) != EOF) {
    if((*length)+1>=*capacity){
      char *temp_buffer = buffer;
      (*capacity)*=2;
      temp_buffer = realloc(temp_buffer, *capacity);
      if (temp_buffer == NULL) {
        printf("reallocation failed!!!\n");
        return 0;
      } else {
        buffer = temp_buffer;
      }
    }
    if(isValidChar(ch)) buffer[(*length)++]=(char) ch;
  }
  
  buffer[*length]='\0';
  return buffer;
}

// to decode the accepted data from b64 to .bmp
uint8_t* decodeB64(char *buffer, uint8_t *binary_buffer, size_t *capacity, size_t *binary_length, size_t length){
  size_t cursor = 0;
  while (cursor < length) {
    
    if(length%4!=0) {printf("invalid input length"); return 0;}

    char c1 = buffer[cursor];
    char c2 = buffer[cursor + 1];
    char c3 = buffer[cursor + 2];
    char c4 = buffer[cursor + 3];    

    cursor+=4;
    char val;
    
    size_t paddings = 0;

    uint8_t packed = 0;
    uint16_t b64_val;
    for(int i=0;i<4;i++){
      switch (i) {
        case 0: val = c1; break;
        case 1: val = c2; break;
        case 2: val = c3; break;
        case 3: val = c4; break;

      }
      
      if ((val>=65 && val<=90)){
          b64_val = (val-65);
        } else if (val>=97 && val<=122){
          b64_val = (val-71);
        } else if (val-'0'>=0 && val-'0'<=9) {
          b64_val = (val-'0'+52);
        } else if (val=='+') {
          b64_val = 62;
        } else if (val=='/') {
          b64_val = 63;
        } else if (val=='=') {
          packed = (packed << 6);
          continue;
        } else {
          b64_val = 0;
        }

      packed = (packed << 6) | b64_val;
    }
    
    for(int i = 0; i<3; i++){
      if((*binary_length)+1>=*capacity){
        uint8_t *temp_buffer = binary_buffer;
        (*capacity)*=2;
        temp_buffer = realloc(temp_buffer, *capacity);
        if (temp_buffer == NULL) {
          printf("reallocation failed!!!\n");
          return 0;
        } else {
          binary_buffer = temp_buffer;
        }
      }
      binary_buffer[(*binary_length)++]= (packed >> (2-i)*8) & 0xFF;
    }
  }
  
  // gotta terminate
  binary_buffer[*binary_length]='\0';
  return binary_buffer;
}

// struct for the BMPHeaderr
#pragma pack(1)
typedef struct BMPHeader {
  uint16_t type;
  uint8_t size;
  uint16_t reserved1;
  uint16_t reserved2;
  size_t offset;
} BMPHeader;

// extract offset from header of .bmp
size_t extractPixelDataOffset (uint8_t *binary_buffer, size_t length) {
  BMPHeader *header = (BMPHeader*) binary_buffer;
  if(header->type == 0x4D42 && header->offset<length) return header->offset;
  return 0;
}

int main(int argc, char *argv[]){
  size_t capacity = 256;
  char *buffer = malloc(capacity);
  size_t length = 0;

  buffer = acceptUserInput(buffer, &capacity, &length);
  if(buffer==NULL) return 0;
  
  size_t binary_capacity=256;
  uint8_t *binary_buffer = malloc(binary_capacity);
  size_t binary_length = 0;
  
  binary_buffer = decodeB64(buffer, binary_buffer, &binary_capacity, &binary_length, length);
  if (binary_buffer==NULL){free(buffer); return 0;}

  size_t pixel_data_offset = extractPixelDataOffset(binary_buffer, binary_length);

  size_t hidden_offset = 0;

  if(argc>1) hidden_offset = atoi(argv[1]);

  if (pixel_data_offset>0){
    if (pixel_data_offset + hidden_offset >= binary_length) {
      uint8_t *target = binary_buffer+pixel_data_offset+hidden_offset;
      size_t max_len = binary_length - (pixel_data_offset + hidden_offset);

      for(size_t i = 0; i < max_len && target[i] != '\0'; i++) {  
        putchar(target[i]);
      }
    } else {
      printf("Offset outside buffer");
    }
  }
  
  free(buffer);
  free(binary_buffer);
  return 0;
}
