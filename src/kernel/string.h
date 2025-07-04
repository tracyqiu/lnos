#ifndef STRING_H
#define STRING_H
#include "stdint.h"

char* strcpy(char* destination, const char* source);

char* strncpy(char* destination, const char* source, uint32_t num);

char* strcat(char* destination, const char* source);

int strcmp(const char* str1, const char* str2);


void* memset(void* ptr, uint8_t value, uint32_t num);

void* memcpy(void* destination, const void* source, uint32_t num);

void *memmove(void *dest, const void *src, uint32_t n);

int32_t strlen(const char* str);

#endif