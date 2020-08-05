#pragma once

#include "../Math.h"
#include "Body.h"

#include <iostream>

namespace Physics
{
    f32 CONTACT_DISTANCE_EPSILON = 0.05;

    class Contact
    {
    private:
        Contact();

        Body* firstBody;
        Body* secondBody;

        BodyFeature firstCurrentFeature;
        BodyFeature secondCurrentFeature;

        f32 TOI; // Time Of Impact
        f32 distance;
        f32 relativeSpeed;

        // TODO: refactor me!!!
        bool tryUpdateCurrentFeature(const Body* const currentBody, BodyFeature& currentFeature,
                                  const Body* const otherBody, BodyFeature& otherFeature)
        {
            bool somethingUpdated = false;
            while (true)
            {
                if (otherFeature.isPoint)
                {
                    const v3 otherPoint = otherBody->GetFirstPointPosition(otherFeature);

                    while (currentBody->TryUpdateToNextFeature(currentFeature, otherPoint))
                    {
                        somethingUpdated = true;
                    }
                    while (currentBody->TryUpdateToPreviousFeature(currentFeature, otherPoint))
                    {
                        somethingUpdated = true;
                    }

                    const v3 thisPoint = currentFeature.isPoint ?
                        currentBody->GetFirstPointPosition(currentFeature) : currentBody->GetPointNearestToOther(currentFeature, otherPoint);
                    v3 contactVector = otherPoint - thisPoint;
                    distance = Length(contactVector);
                    contactVector /= distance;
                    relativeSpeed = Dot(contactVector.xy, currentBody->GetVelocity()) +
                                    Dot(-contactVector.xy, otherBody->GetVelocity());
                    return somethingUpdated;
                }

                while (true)
                {
                    if (currentFeature.isPoint)
                    {
                        const v3 currentPoint = currentBody->GetFirstPointPosition(currentFeature);
                        const v3 nearestPoint = 
                            otherBody->GetPointNearestToOther(otherFeature, currentPoint);

                        if (currentBody->TryUpdateToNextFeature(currentFeature, nearestPoint) ||
                            currentBody->TryUpdateToPreviousFeature(currentFeature, nearestPoint))
                        {
                            somethingUpdated = true;
                            continue;
                        }
                        else
                        {
                            v3 contactVector = nearestPoint - currentPoint;
                            distance = Length(contactVector);
                            contactVector /= distance;
                            relativeSpeed = Dot(contactVector.xy, currentBody->GetVelocity()) +
                                    Dot(-contactVector.xy, otherBody->GetVelocity());

                            return somethingUpdated;
                        }
                    }
                    else
                    {
                        const v3 firstOtherPoint = otherBody->GetFirstPointPosition(otherFeature);
                        const v3 secondOtherPoint = otherBody->GetSecondPointPosition(otherFeature);
                        const v3 firstThisPoint = currentBody->GetFirstPointPosition(currentFeature);
                        const v3 secondThisPoint = currentBody->GetSecondPointPosition(currentFeature);

                        v3 thisFeatureVector = secondThisPoint - firstThisPoint;
                        v3 otherFeatureVector = secondOtherPoint - firstOtherPoint;
                        v3 thisToOtherVector = firstOtherPoint - firstThisPoint;

                        int distCase = 1;
                        v3 contactVector = firstOtherPoint - currentBody->GetPointNearestToOther(currentFeature, firstOtherPoint);
                        f32 minDist = Length(contactVector);

                        f32 possibleDist = Length(currentBody->GetPointNearestToOther(currentFeature, secondOtherPoint) - secondOtherPoint);
                        if (possibleDist < minDist)
                        {
                            distCase = 2;
                            minDist = possibleDist;
                            contactVector = secondOtherPoint - currentBody->GetPointNearestToOther(currentFeature, secondOtherPoint);
                        }

                        possibleDist = Length(otherBody->GetPointNearestToOther(otherFeature, firstThisPoint) - firstThisPoint);
                        if (possibleDist < minDist)
                        {
                            distCase = 3;
                            minDist = possibleDist;
                            contactVector = otherBody->GetPointNearestToOther(otherFeature, firstThisPoint) - firstThisPoint;
                        }

                        possibleDist = Length(otherBody->GetPointNearestToOther(otherFeature, secondThisPoint) - secondThisPoint);
                        if (possibleDist < minDist)
                        {
                            distCase = 4;
                            minDist = possibleDist;
                            contactVector = otherBody->GetPointNearestToOther(otherFeature, secondThisPoint) - secondThisPoint;
                        }

                        // features are parallel case
                        // TODO: epsilon shouldn't be random
                        if (abs(thisFeatureVector.x * otherFeatureVector.y - thisFeatureVector.y * otherFeatureVector.x) < 0.01 &&
                            thisToOtherVector.x * thisFeatureVector.y - thisToOtherVector.y * thisFeatureVector.x < F32::Eps &&
                            otherFeatureVector.x * thisToOtherVector.y - otherFeatureVector.y * thisToOtherVector.x < F32::Eps)
                        {
                            distance = minDist;
                            contactVector /= distance;
                            relativeSpeed = Dot(contactVector.xy, currentBody->GetVelocity()) +
                                    Dot(-contactVector.xy, otherBody->GetVelocity());
                            return somethingUpdated;
                        }

                        somethingUpdated = true;
                        
                        switch (distCase)
                        {
                        case 1:
                            otherBody->UpdateToPreviousFeature(otherFeature);
                            break;
                        case 2:
                            otherBody->UpdateToNextFeature(otherFeature);
                            break;
                        case 3:
                            currentBody->UpdateToPreviousFeature(currentFeature);
                            break;
                        case 4:
                            currentBody->UpdateToNextFeature(currentFeature);
                            break;
                        }
                        break;
                    }
                }
            }
        }

    public:
        Contact(Body* a, Body* b) : 
            firstBody(a),
            secondBody(b), 
            firstCurrentFeature(a->GetRandomFeature()),
            secondCurrentFeature(b->GetRandomFeature()),
            TOI(0.0)
        {
        };

        f32 GetTOI() const
        {
            return TOI;
        }

        // Lin-Canny algorithm is used here
        void Update()
        {
            firstBody->AdvanceToTime(TOI);
            secondBody->AdvanceToTime(TOI);

            while (tryUpdateCurrentFeature(firstBody, firstCurrentFeature, secondBody, secondCurrentFeature) |
                   tryUpdateCurrentFeature(secondBody, secondCurrentFeature, firstBody, firstCurrentFeature)) {}

            if (distance < CONTACT_DISTANCE_EPSILON && relativeSpeed > 0.0)
            {
                v2 firstVelocity = firstBody->GetVelocity();
                firstBody->SetVelocity(secondBody->GetVelocity());
                secondBody->SetVelocity(firstVelocity);
                TOI = -1.0;
            }
            else
            {
                TOI += distance / relativeSpeed;
            }
        };

        inline bool operator<(const Contact& other) const
        {
            return this->TOI > other.TOI;
        }

        void Draw(RenderQueue* queue) const
        {
            v3 firstPoint = firstBody->GetFirstPointPosition(firstCurrentFeature);
            v3 secondPoint = !firstCurrentFeature.isPoint ? firstBody->GetSecondPointPosition(firstCurrentFeature) : firstPoint;
            DrawLine(queue, firstPoint, secondPoint, {1.0, 0.0, 0.0, 1.0});

            firstPoint = secondBody->GetFirstPointPosition(secondCurrentFeature);
            secondPoint = !secondCurrentFeature.isPoint ? secondBody->GetSecondPointPosition(secondCurrentFeature) : firstPoint;
            DrawLine(queue, firstPoint, secondPoint, {1.0, 0.0, 0.0, 1.0});
        }
    };
};
