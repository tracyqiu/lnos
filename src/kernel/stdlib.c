#include "stdlib.h"
#include "stdint.h"

//------------------------------------------------------------------------------
char* itoa(int32_t value, char* str, uint32_t base)
//------------------------------------------------------------------------------
{
   int32_t i = 0;
   bool isNegative = false;
   if (value < 0 && base == 10) {
      isNegative = true;
      value = -value;
   }

   do {
      int32_t remainder = value % base;
      str[i++] = (remainder > 9) ? (remainder - 10 + 'A') : (remainder + '0');
      value /= base;
   } while (value != 0);

   switch (base)
   {
   case 10:
      if (isNegative) {
         str[i++] = '-';
      }
      break;
   case 2:
      str[i++] = 'b';
      str[i++] = '0';
      break;
   case 8:
      str[i++] = '0';
      break;
   case 16:
      str[i++] = 'x';
      str[i++] = '0';
   default:
      break;
   }

   for (int j = 0; j < i / 2; ++j) {
      char tmp = str[j];
      str[j] = str[i-j-1];
      str[i-j-1] = tmp;
   }

   str[i] = '\0';

   return str;
}