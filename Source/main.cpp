#include <stdio.h>

#include "Assert.h"
#include "SpinLock.h"

int main(int argc, char** argv)
{
    VPushLock PushLock;

    PushLock.AcquireRead();
    PushLock.ReleaseRead();
    PushLock.AcquireWrite();
    PushLock.ReleaseWrite();

    printf("here\n");
    return 0;
}
