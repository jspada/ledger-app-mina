#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "utils.h"

int main()
{
   char buf[32];

   assert(strcmp(value_to_string(buf, sizeof(buf), 0), "0") == 0);
   assert(strcmp(value_to_string(buf, sizeof(buf), 000), "0") == 0);
   assert(strcmp(value_to_string(buf, sizeof(buf), 001), "1") == 0);
   assert(strcmp(value_to_string(buf, sizeof(buf), 1001), "1001") == 0);
   assert(strcmp(value_to_string(buf, sizeof(buf), 10000000), "10000000") == 0);
   assert(strcmp(value_to_string(buf, sizeof(buf), 32), "32") == 0);
   assert(strcmp(amount_to_string(buf, sizeof(buf), 0), "0.0") == 0);
   assert(strcmp(amount_to_string(buf, sizeof(buf), 1), "0.000000001") == 0);
   assert(strcmp(amount_to_string(buf, sizeof(buf), 2), "0.000000002") == 0);
   assert(strcmp(amount_to_string(buf, sizeof(buf), 10), "0.00000001") == 0);
   assert(strcmp(amount_to_string(buf, sizeof(buf), 12), "0.000000012") == 0);
   assert(strcmp(amount_to_string(buf, sizeof(buf), 101010101), "0.101010101") == 0);
   assert(strcmp(amount_to_string(buf, sizeof(buf), 100000000), "0.1") == 0);
   assert(strcmp(amount_to_string(buf, sizeof(buf), 900000000), "0.9") == 0);
   assert(strcmp(amount_to_string(buf, sizeof(buf), 10000000), "0.01") == 0);
   assert(strcmp(amount_to_string(buf, sizeof(buf), 1000000000), "1.0") == 0);
   assert(strcmp(amount_to_string(buf, sizeof(buf), 1100000000), "1.1") == 0);
   assert(strcmp(amount_to_string(buf, sizeof(buf), 1010000000), "1.01") == 0);
   assert(strcmp(amount_to_string(buf, sizeof(buf), 1001000000), "1.001") == 0);
   assert(strcmp(amount_to_string(buf, sizeof(buf), 1000100000), "1.0001") == 0);
   assert(strcmp(amount_to_string(buf, sizeof(buf), 9000000000), "9.0") == 0);
   assert(strcmp(amount_to_string(buf, sizeof(buf), 123000000000), "123.0") == 0);
   assert(strcmp(amount_to_string(buf, sizeof(buf), 123400000000), "123.4") == 0);
   assert(strcmp(amount_to_string(buf, sizeof(buf), 123040000000), "123.04") == 0);
   assert(strcmp(amount_to_string(buf, sizeof(buf), 123450000000), "123.45") == 0);
   assert(strcmp(amount_to_string(buf, sizeof(buf), 10000000001), "10.000000001") == 0);
   assert(strcmp(amount_to_string(buf, sizeof(buf), 1000000001), "1.000000001") == 0);
   assert(strcmp(amount_to_string(buf, sizeof(buf), 1000000000000001), "1000000.000000001") == 0);
   assert(strcmp(amount_to_string(buf, sizeof(buf), 1000001000000001), "1000001.000000001") == 0);
   assert(strcmp(amount_to_string(buf, sizeof(buf), 20000001000000001), "20000001.000000001") == 0);
   assert(strcmp(amount_to_string(buf, sizeof(buf), 150000010000000001), "150000010.000000001") == 0);
   assert(strcmp(amount_to_string(buf, sizeof(buf), 2200000070000000004), "2200000070.000000004") == 0);
   assert(strcmp(amount_to_string(buf, sizeof(buf), 2718281828), "2.718281828") == 0);
   assert(strcmp(amount_to_string(buf, sizeof(buf), 314159265359), "314.159265359") == 0);
   assert(strcmp(amount_to_string(buf, sizeof(buf), 1618033988750000), "1618033.98875") == 0);

   printf("Utils tests completed successfully!\n");

   return 0;
}
