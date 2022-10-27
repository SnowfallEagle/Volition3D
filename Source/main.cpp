#include <stdio.h>

#include "Assert.h"
#include "SpinLock.h"

int main(int argc, char** argv)
{
    VSpinLock SpinLock;
    TScopedLock<decltype(SpinLock)> ScopedLock(SpinLock);

    // TODO(sean): Fix bug in VReentrantLock32
    VReentrantLock32 Reentrant;
    Reentrant.Acquire();
    Reentrant.Acquire();
    Reentrant.Release();
    Reentrant.Release();
    printf("here\n");
    return 0;
}
