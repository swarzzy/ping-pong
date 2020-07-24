#pragma once

#include <cstring>

#define EMPTY_ALLOCATION_SIZE 4 

namespace Containers
{
    template <typename T>
    class Array
    {
    private:
        T* data;
        i32 size;
        i32 allocatedSize;

        void reallocateData()
        {
            allocatedSize *= 2;
            data = (i32*) PlatformReallocate(data, allocatedSize, nullptr);
        }

    public:
        typedef T* Iterator;

        Array(const Array<T>& other) : size(other.size), allocatedSize(other.allocatedSize)
        {
            data = (i32*) PlatformAllocate(sizeof(T) * allocatedSize, 0, nullptr);
            memcpy(data, other.data, sizeof(T) * allocatedSize);
        }

        Array()
        {
            allocatedSize = EMPTY_ALLOCATION_SIZE;
            data = (i32*) PlatformAllocate(sizeof(T) * allocatedSize, 0, nullptr);
            size = 0;
        }

        ~Array()
        {
            PlatformDeallocate(data, nullptr);
        }

        void Insert(const T& elem)
        {
            if (allocatedSize == size)
            {
                reallocateData();
            }

            data[size++] = elem;
        }

        void Expand(i32 count)
        {
            if (size + count < allocatedSize)
            {
                reallocateData();
            }

            size += count;
        }

        void Remove(i32 count)
        {
            size -= count;
            if (size < 0)
            {
                size = 0;
            }
        }

        i32 Size() const
        {
            return size;
        }

        T& operator[](i32 index)
        {
            return data[index];
        }

        T At(i32 index) const
        {
            return data[index];
        }

        Iterator begin()
        {
            return data;
        }

        Iterator end()
        {
            return data + size;
        }
    };
}
