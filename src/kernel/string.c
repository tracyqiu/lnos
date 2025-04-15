#include "string.h"

//------------------------------------------------------------------------------
char * strcpy ( char * destination, const char * source )
//------------------------------------------------------------------------------
{
   char* tmp = destination;
   while ((*destination++ = *source++));
   return tmp;
}


//------------------------------------------------------------------------------
char * strcat ( char * destination, const char * source )
//------------------------------------------------------------------------------
{
   char* tmp = destination;
   while (*tmp != '\0') tmp++;
   while (*source != '\0') *tmp++ = *source++;
   *tmp = '\0';
   return destination;
}

//------------------------------------------------------------------------------
int strcmp ( const char * str1, const char * str2 )
//------------------------------------------------------------------------------
{
   while (*str1 && *str2) {
      if (*str1 != * str2) return (*str1 - *str2);
      str1++;
      str2++;
   }
   return (*str1 - *str2);
}