#include <stdio.h>

#include "Assert.h"

int main(int argc, char** argv)
{
    int i = 1;

    ASSERT(i == 2);

    printf("Hello, world!\n");
    return 0;
}
