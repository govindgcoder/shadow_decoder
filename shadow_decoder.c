#include<stdio.h>

int main(){
  size_t capacity = 256;
  char *buffer = malloc(capacity);
  size_t length = 0;

  while((ch = getchar()) != EOF) {
    if(length+1>=capacity){
      capacity*=2;
      buffer = realloc(buffer, capacity);
    }
    buffer[length++]=(char) ch;
  }

  return 0
}
