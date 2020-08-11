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
        struct Iterator
        {
        private:
            i32 index;
            Array<T>* array;

        public:
            Iterator() : array(nullptr), index(-1){};
            Iterator(Array<T>* a, i32 ind) : array(a), index(ind){};

            T& operator*()
            {
                assert(index >= 0);
                assert(index < array->Size());

                return array->operator[](index);
            }

            bool operator!=(const Iterator& other) const
            {
                return index != other.index;
            }

            bool operator==(const Iterator& other) const
            {
                return index == other.index;
            }

            void operator++()
            {
                ++index;
            }

            Iterator operator+(i32 x)
            {
                return {array, index + x};
            }

            Iterator operator-(i32 x)
            {
                return {array, index - x};
            }

            T* operator->()
            {
                return &array->operator[](index);
            }

            bool operator==(const T* other) const
            {
                if (array == nullptr)
                {
                    return other == nullptr;
                }
                return &(array->operator[](index)) == other;
            }

            bool operator!=(const T* other) const
            {
                return !(*this == other);
            }
        };

        struct ConstIterator
        {
        private:
            i32 index;
            const Array<T>* array;

        public:
            ConstIterator() : array(nullptr), index(-1){};
            ConstIterator(const Array<T>* a, i32 ind) : array(a), index(ind){};

            const T& operator*() const
            {
                assert(index >= 0);
                assert(index < array->Size());

                return array->operator[](index);
            }

            bool operator!=(ConstIterator other) const
            {
                return index != other.index;
            }

            bool operator==(ConstIterator other) const
            {
                return index == other.index;
            }

            void operator++()
            {
                ++index;
            }

            ConstIterator operator+(i32 x)
            {
                return {array, index + x};
            }

            ConstIterator operator-(i32 x)
            {
                return {array, index - x};
            }

            const T* operator->() const
            {
                return &array->operator[](index);
            }

            bool operator==(const T* other) const
            {
                if (array == nullptr)
                {
                    return other == nullptr;
                }
                return &(array->operator[](index)) == other;
            }

            bool operator!=(const T* other) const
            {
                return !(*this == other);
            }
        };

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
            for (Iterator iterator = this->begin(); iterator != this->end(); ++iterator)
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

        void Remove(i32 count)
        {
            if (count > size)
            {
                count = size;
            }

            for (int i = size - 1; i >= size - count; --i)
            {
                (data + i)->~T();
            }

            size -= count;
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
            return {this, 0};
        }

        Iterator end()
        {
            return {this, size};
        }

        ConstIterator begin() const
        {
            return {this, 0};
        }

        ConstIterator end() const
        {
            return {this, size};
        }
    };
}
