#include "string.h"

//------------------------------------------------------------------------------
char* strcpy(char* destination, const char* source)
//------------------------------------------------------------------------------
{
   char* tmp = destination;
   while ((*destination++ = *source++));
   return tmp;
}

//------------------------------------------------------------------------------
char* strncpy(char* destination, const char* source, uint32_t num)
//------------------------------------------------------------------------------
{
   char* tmp = destination;
   while (num--) *destination++ = *source++;
   return tmp;
}

//------------------------------------------------------------------------------
char* strcat(char* destination, const char* source)
//------------------------------------------------------------------------------
{
   char* tmp = destination;
   while (*tmp != '\0') tmp++;
   while (*source != '\0') *tmp++ = *source++;
   *tmp = '\0';
   return destination;
}

//------------------------------------------------------------------------------
int strcmp(const char* str1, const char* str2)
//------------------------------------------------------------------------------
{
   while (*str1 && *str2) {
      if (*str1 != * str2) return (*str1 - *str2);
      str1++;
      str2++;
   }
   return (*str1 - *str2);
}

//------------------------------------------------------------------------------
void* memset(void* ptr, uint8_t value, uint32_t num)
//------------------------------------------------------------------------------
{
   uint8_t* tmp = ptr;
   while (num--) *tmp++ = value;
   return ptr;
}

//------------------------------------------------------------------------------
void* memcpy(void* destination, const void* source, uint32_t num)
//------------------------------------------------------------------------------
{
   uint8_t* tmp = (uint8_t*)destination;
   uint8_t* src = (uint8_t*)source;
   while (num--) *tmp++ = *src++;
   return destination;
}

//------------------------------------------------------------------------------
void *memmove(void *dest, const void *src, uint32_t n)
//------------------------------------------------------------------------------
{
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;

    if (d == s || n == 0) {
        return dest;
    }

    if (d < s) {
        for (uint32_t i = 0; i < n; i++) {
            d[i] = s[i];
        }
    } else {
        for (uint32_t i = n; i > 0; i--) {
            d[i-1] = s[i-1];
        }
    }

    return dest;
}

//------------------------------------------------------------------------------
int32_t strlen(const char* str)
//------------------------------------------------------------------------------
{
    int32_t len = 0;
    while (str[len]) len++;
    return len;
}