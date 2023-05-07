#include "IInterpolator.h"

namespace Volition
{

static void EmptyFunc(IInterpolator*) {}
static void EmptyFunc(IInterpolator*, i32, i32, i32) {}
static void EmptyFunc(IInterpolator*, i32) {}

IInterpolator::IInterpolator()
{
    Start = EmptyFunc;

    ComputeYStartsAndDeltasLeft = EmptyFunc;
    ComputeYStartsAndDeltasRight = EmptyFunc;

    ComputeXStartsAndDeltas = EmptyFunc;
    SwapLeftRight = EmptyFunc;
    ProcessPixel = EmptyFunc;

    InterpolateX = EmptyFunc;
    InterpolateYLeft = EmptyFunc;
    InterpolateYRight = EmptyFunc;
}

}
