#include "Math/Matrix.h"
#include "Math/Vector.h"

void VVector4D::MulMat44(const VVector4D& A, const VMatrix44& M, VVector4D& R)
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
