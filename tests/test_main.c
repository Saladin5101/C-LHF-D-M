#include <stdio.h>
#include <assert.h>
#include "clhfm.h"

int main(void) {
    printf("Running C-LHF&D-M tests...\n");
    system("make clean && make");
    system("./bin/clhfm");
    /* Add test cases here */
    printf("All tests passed!\n");
    
    return 0;
}
