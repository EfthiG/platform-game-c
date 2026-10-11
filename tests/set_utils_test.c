#include <stdio.h>
#include <stdlib.h>
#include "acutest.h"			

#include "ADTSet.h"
#include "common_types.h"
#include "set_utils.h"

int compare_ints(Pointer a, Pointer b){
   return *(int*)a - *(int*)b;
}
int* create_int(int value) {
	int* pointer = malloc(sizeof(int));		
	*pointer = value;	
	return pointer;
}

void set_utils_test(){
Set set = set_create(compare_ints, free);
for(int i=0 ; i<=10 ; i+=2){
    set_insert(set, create_int(i));
}

Pointer a = set_find_eq_or_greater(set, create_int(2));
TEST_ASSERT( *(int*)a == 2);
a = set_find_eq_or_greater(set, create_int(3));
TEST_ASSERT(  *(int*)a == 4);
a = set_find_eq_or_smaller(set, create_int(8));
TEST_ASSERT(  *(int*)a == 8);
a = set_find_eq_or_smaller(set, create_int(7));
TEST_ASSERT(  *(int*)a == 6);
}


TEST_LIST = {
	{ "set_utils_test", set_utils_test },
	
	{ NULL, NULL } 
};


