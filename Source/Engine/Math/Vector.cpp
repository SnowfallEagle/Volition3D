#include "Engine/Math/Matrix.h"
#include "Engine/Math/Vector.h"

void VVector4::MulMat44(const VVector4& A, const VMatrix44& M, VVector4& R)
{
    for (i32f X = 0; X < 4; ++X)
    {
        f32 C = 0.0f;

        for (i32f Y = 0; Y < 4; ++Y)
        {
            C += A.C[Y] * M.C[Y][X];
        }

        R.C[X] = C;
    }
}
