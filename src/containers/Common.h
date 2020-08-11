#pragma once

namespace Containers
{
    namespace
    {
        template<typename T>
        bool defaultLess(const T& a, const T& b)
        {
            return a < b;
        }
    }
}
