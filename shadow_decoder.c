#include<stdio.h> // for io
#include<stdlib.h> // for memory management and all

// accept user input stream and dynamically allocate a buffer to store it
char* acceptUserInput(char *initial_buffer, size_t capacity, size_t *length){
  char *buffer = initial_buffer;
  int ch;
// accept stream of data 
  while((ch = getchar()) != EOF) {
    if(*length+1>=capacity){
      char *temp_buffer = buffer;
      capacity*=2;
      temp_buffer = realloc(temp_buffer, capacity);
      if (temp_buffer == NULL) {
        printf("reallocation failed!!!\n");
        return 0;
      } else {
        buffer = temp_buffer;
      }
    }
    buffer[(*length)++]=(char) ch;
  }

  return buffer;
}

// to decode the accepted data from b64 to .bmp
char* decodeB64(char *buffer, char *binary_buffer, size_t *capacity, size_t *binary_length, size_t length){
  size_t cursor = 0;
  while (cursor < length) {
    char c1 = buffer[cursor];
    char c2 = buffer[cursor + 1];
    char c3 = buffer[cursor + 2];
    char c4 = buffer[cursor + 3];    

    cursor+=4;
    char val;
    uint32_t packed = 0;
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
        } else {
          b64_val = 0;
        }

      packed = (packed << 6) | b64_val;
    }
    
    for(int i = 0; i<3; i++){
      if((*binary_length)+8>=capacity){
        char *temp_buffer = binary_buffer;
        (*capacity)*=2;
        temp_buffer = realloc(temp_buffer, *capacity);
        if (temp_buffer == NULL) {
          printf("reallocation failed!!!\n");
          return 0;
        } else {
          binary_buffer = temp_buffer;
        }
      }
      binary_buffer[*binary_length++]= (packed >> (2-i)*8) & 0xFF;
    }
  }
  return binary_buffer;
}

int main(){
  size_t capacity = 256;
  char *buffer = malloc(capacity);
  size_t length = 0;

  buffer = acceptUserInput(buffer, capacity, &length);
  
  size_t binary_capacity=256;
  char *binary_buffer = malloc(capacity);
  size_t binary_length = 0;
  
  binary_buffer = decodeB64(buffer, binary_buffer, &binary_capacity, &binary_length, length);
  
  free(buffer);
  free(binary_buffer);
  return 0;
}
