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
        i32 size = 0;
        i32 allocatedSize = 0;

        void reallocateData()
        {
            allocatedSize *= 2;
            data = (T*) PlatformReallocate(data, allocatedSize * sizeof(T), nullptr);
        }

    public:
        typedef T* Iterator;
        typedef const T* ConstIterator;

        Array(const Array<T>& other) : size(other.size), allocatedSize(other.allocatedSize)
        {
            if (allocatedSize)
            {
                data = (T*) PlatformAllocate(sizeof(T) * allocatedSize, 0, nullptr);
                memcpy(data, other.data, sizeof(T) * allocatedSize);
            }
        }

        Array()
        {
            data = (T*) PlatformAllocate(sizeof(T) * EMPTY_ALLOCATION_SIZE, 0, nullptr);
            if (data)
            {
                allocatedSize = EMPTY_ALLOCATION_SIZE;
            }

            size = 0;
        }

        ~Array()
        {
            for (T* iterator = this->begin(); iterator != this->end(); ++iterator)
            {
                iterator->~T();
            }

            if (allocatedSize)
            {
                PlatformDeallocate(data, nullptr);
            }
        }

        Array<T>& operator=(const Array<T>& other)
        {
            if (allocatedSize)
            {
                data = (T*) PlatformReallocate(data, other.allocatedSize * sizeof(T), nullptr);
            }
            else
            {
                data = (T*) PlatformAllocate(sizeof(T) * other.allocatedSize, 0, nullptr);
            }

            size = other.size;
            allocatedSize = other.allocatedSize;

            memcpy(data, other.data, sizeof(T) * allocatedSize);

            return *this;
        }

        void Insert(const T& elem)
        {
            if (allocatedSize == size)
            {
                reallocateData();
            }

            new(data + size) T(elem);
            ++size;
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

        const T& operator[](i32 index) const
        {
            return data[index];
        }

        T& Back()
        {
            return data[size - 1];
        }

        const T& Back() const
        {
            return data[size - 1];
        }

        Iterator begin()
        {
            return data;
        }

        Iterator end()
        {
            return data + size;
        }

        ConstIterator begin() const
        {
            return data;
        }

        ConstIterator end() const
        {
            return data + size;
        }
    };
}
