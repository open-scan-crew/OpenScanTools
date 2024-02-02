#include "utils/math/basic_functions.h"      


size_t tls::math::getCeilPowTwo(size_t minSize)
{
    size_t powTwo = 1;
    for (int i = 0; i < 32; i++)
    {
        if (powTwo >= minSize)
            return powTwo;
        else
            powTwo = powTwo << 1;
    }
    return 0;
}