#include <stdio.h>
#include <stdint.h>

#include "bitpack.h"


void printbytes(void *p, unsigned int len) 
{ 
  unsigned int i; 
  unsigned char *cp = (unsigned char *)p; 

  for (i = 0; i < len; i++) { 
    printf("%02X", *cp++); 
  } 
 
  printf("\n"); 
}


int main(int argc, char const *argv[])
{
        (void) argc;
        (void) argv;

        printf("---FIT TEST----\n"); 

        uint64_t test1_n = -4;
        unsigned test1_width = 3;

        printf("Can %ld fit in a %d-bit unsigned? %s \n", test1_n, test1_width,
                Bitpack_fitsu(test1_n, test1_width) ? "Yes" : "No");

        printf("Can %ld fit in a %d-bit signed? %s \n", test1_n, test1_width,
                Bitpack_fitss(test1_n, test1_width) ? "Yes" : "No");

        printf("---GET TEST----\n"); 

        uint64_t test2_data = 0x3f4;
        unsigned test2_width = 6;
        unsigned test2_lsb = 2;

        printf("Unsigned: %ld\n", Bitpack_getu(test2_data, test2_width, test2_lsb));
        printf("Signed: %ld\n", Bitpack_gets(test2_data, test2_width, test2_lsb));

        printf("---NEW TEST----\n"); 

        unsigned test3_width = 8;
        unsigned test3_lsb = 14;
        uint64_t test3_value = 24;

        uint64_t test3_data = Bitpack_newu(0, test3_width, test3_lsb, test3_value);

        uint64_t test3_get = Bitpack_getu(test3_data, test3_width, test3_lsb);

        printf("UNSIGNED: This should be %ld: %ld\n", test3_value, test3_get);

        unsigned test4_width = 8;
        unsigned test4_lsb = 4;
        int64_t test4_value = -100;

        int64_t test4_data = Bitpack_news(0, test4_width, test4_lsb, test4_value);

        int64_t test4_data2 = Bitpack_news(test4_data, test4_width, test4_width+test4_lsb, test4_value);

        int64_t test4_get = Bitpack_gets(test4_data2, test4_width, test4_lsb);

        printbytes(&test4_data2, sizeof(test4_data2));

        printf("SIGNED: This should be %ld: %ld\n", test4_value, test4_get);

        printf("%d\n", 
                Bitpack_getu(Bitpack_newu(0, 2, 1, 3), 2, 4) ==
                Bitpack_getu(0, 2, 4)); 

        return 0;
}