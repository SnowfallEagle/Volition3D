/* TODO(sean)
    - Put indentity matrices in static variables for each class
 */

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
