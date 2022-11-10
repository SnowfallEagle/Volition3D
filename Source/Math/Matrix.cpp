#include "Math/Matrix.h"

const VMatrix44 VMatrix44::Identity = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1,
};

const VMatrix43 VMatrix43::Identity = {
    1, 0, 0,
    0, 1, 0,
    0, 0, 1,
    0, 0, 0,
};

const VMatrix33 VMatrix33::Identity = {
    1, 0, 0,
    0, 1, 0,
    0, 0, 1,
};

const VMatrix32 VMatrix32::Identity = {
    1, 0,
    0, 1,
    0, 0,
};

const VMatrix22 VMatrix22::Identity = {
    1, 0,
    0, 1,
};

void VMatrix44::Mul(const VMatrix44& A, const VMatrix44& B, VMatrix44& R)
{
    /*
        Result matrix

             -> X
          -         -
        | | . . . . |
        v | . . . . |
          | . . . . |
        Y | . . . . |
          -         -
    
        T is temp for go through
        A's columns and B's rows
     */
    for (i32f Y = 0; Y < 4; ++Y)
    {
        for (i32f X = 0; X < 4; ++X)
        {
            f32 C = 0.0f;
            for (i32f T = 0; T < 4; ++T)
            {
                C += A.C[Y][T] * B.C[T][X];
            }
            R.C[Y][X] = C;
        }
    }
}
