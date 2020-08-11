#pragma once

#include "Array.h"
#include "Common.h"

namespace Containers
{
    template<typename T>
    class PriorityQueue
    {
    private:
        Array<T> data;

        inline i32 getParentIndex(i32 currentIndex) const
        {
            return (currentIndex - 1) >> 1;
        }

        inline i32 getFirstChildIndex(i32 currentIndex) const
        {
            return (currentIndex << 1) + 1;
        }

        inline i32 getSecondChildIndex(i32 currentIndex) const
        {
            return (currentIndex << 1) + 2;
        }

        bool (*lessThan)(const T& a, const T& b);

    public:
        explicit PriorityQueue(bool (*lessFunction)(const T&, const T&) = (&defaultLess<T>)) : lessThan(lessFunction) {};

        void Insert(T elem)
        {
            i32 currentIndex = data.Size();
            i32 parrentIndex = getParentIndex(currentIndex);

            while (currentIndex > 0 && lessThan(data[parrentIndex], elem))
            {
                if (currentIndex == data.Size())
                {
                    data.Insert(data[parrentIndex]);
                }
                else
                {
                    data[currentIndex] = data[parrentIndex];
                }

                currentIndex = parrentIndex;
                parrentIndex = getParentIndex(currentIndex);
            }

            if (currentIndex == data.Size())
            {
                data.Insert(elem);
            }
            else
            {
                data[currentIndex] = elem;
            }
        }

        i32 Size() const
        {
            return data.Size();
        }

        const T& Top() const
        {
            assert(Size());

            return data[0];
        }

        void RemoveTop()
        {
            if (!data.Size())
            {
                return;
            }

            T elem = data[data.Size() - 1];
            data.Remove(1);

            i32 currentIndex = 0;
            
            while (true)
            {
                i32 nextIndex = 0;
                i32 firstChildIndex = getFirstChildIndex(currentIndex);

                if (firstChildIndex + 1 < data.Size() && lessThan(data[firstChildIndex], data[firstChildIndex + 1]))
                {
                    nextIndex = firstChildIndex + 1;
                }
                else if (firstChildIndex < data.Size())
                {
                    nextIndex = firstChildIndex;
                }

                if (nextIndex != 0 && lessThan(elem, data[nextIndex]))
                {
                    data[currentIndex] = data[nextIndex];
                    currentIndex = nextIndex;
                }
                else
                {
                    data[currentIndex] = elem;
                    return;
                }
            }
        }
    };
} 
