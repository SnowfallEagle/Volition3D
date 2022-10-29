#include <stdio.h>

#include "SpinLock.h"
#include "Types.h"

int main(int Argc, char** Argv)
{
    VPushLock PushLock;

    PushLock.AcquireRead();
    PushLock.ReleaseRead();
    PushLock.AcquireWrite();
    PushLock.ReleaseWrite();

    printf("here\n");
    return 0;
}
