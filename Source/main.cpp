#include <stdio.h>

#include "Assert.h"

int main(int argc, char** argv)
{
    STATIC_ASSERT(1 == 2);

    printf("Hello, world!\n");
    return 0;
}
