#pragma once

#include "Contact.h"

#include "../containers/Array.h"
#include "../containers/PriorityQueue.h"

namespace Physics
{
    class Solver
    {
    private:
        Containers::Array<ConvexBody> Bodies;
        Containers::PriorityQueue<Contact> Contacts;

        f32 time = 0.0f;

    public:
        void Advance(f32 timeDelta)
        {
            time += timeDelta;

            Contact top = Contacts.Top();
            while (top.GetTOI() >= 0.0 && top.GetTOI() < time)
            {
                top.Update();
                Contacts.RemoveTop();
                Contacts.Insert(top);
                top = Contacts.Top();
            }

            for (ConvexBody& b : Bodies)
            {
                b.AdvanceToTime(time);
            }
        }

        void AddBody(const ConvexBody& b)
        {
            Bodies.Insert(b);
            for (int i = 0; i < Bodies.Size() - 1; ++i)
            {
                Contacts.Insert(Contact(&Bodies[i], &Bodies.Back()));
            }
        }

        void DebugDraw(RenderQueue* queue) const
        {
            for (const ConvexBody& b : Bodies)
            {
                b.Draw(queue);
            }

            Contacts.Top().Draw(queue);
        }
    };
}


