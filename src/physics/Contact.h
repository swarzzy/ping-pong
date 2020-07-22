#pragma once

#include "../Math.h"
#include "Body.h"

#include <iostream>

namespace Physics
{
    class Contact
    {
    private:
        Contact();

        Body* firstBody;
        Body* secondBody;

        BodyFeature firstCurrentFeature;
        BodyFeature secondCurrentFeature;

        bool tryUpdateCurrentFeature(const Body* const currentBody, BodyFeature& currentFeature,
                                  const Body* const otherBody, BodyFeature& otherFeature)
        {
            bool somethingUpdated = false;
            while (true)
            {
                if (otherFeature.isPoint)
                {
                    const v3 currentPoint = otherBody->GetFirstPointPosition(otherFeature);
                    while (currentBody->TryUpdateToNextFeature(currentFeature, currentPoint)){ somethingUpdated = true; };
                    while (currentBody->TryUpdateToPreviousFeature(currentFeature, currentPoint)){ somethingUpdated = true; };
                    return somethingUpdated;
                }

                while (true)
                {
                    if (currentFeature.isPoint)
                    {
                        const v3 nearestPoint =
                            otherBody->GetPointNearestToOther(otherFeature, currentBody->GetFirstPointPosition(currentFeature));

                        if (currentBody->TryUpdateToNextFeature(currentFeature, nearestPoint) ||
                            currentBody->TryUpdateToPreviousFeature(currentFeature, nearestPoint))
                        {
                            somethingUpdated = true;
                            continue;
                        }
                        else
                        {
                            return somethingUpdated;
                        }
                    }
                    else
                    {
                        somethingUpdated = true;

                        const v3 firstOtherPoint = otherBody->GetFirstPointPosition(otherFeature);
                        const v3 secondOtherPoint = otherBody->GetSecondPointPosition(otherFeature);
                        const v3 firstThisPoint = currentBody->GetFirstPointPosition(currentFeature);
                        const v3 secondThisPoint = currentBody->GetSecondPointPosition(currentFeature);

                        int distCase = 1;
                        f32 minDist = Dist(currentBody->GetPointNearestToOther(currentFeature, firstOtherPoint), firstOtherPoint);

                        f32 possibleDist = Dist(currentBody->GetPointNearestToOther(currentFeature, secondOtherPoint), secondOtherPoint);
                        if (possibleDist < minDist)
                        {
                            distCase = 2;
                            minDist = possibleDist;
                        }

                        possibleDist = Dist(otherBody->GetPointNearestToOther(otherFeature, firstThisPoint), firstThisPoint);
                        if (possibleDist < minDist)
                        {
                            distCase = 3;
                            minDist = possibleDist;
                        }

                        possibleDist = Dist(otherBody->GetPointNearestToOther(otherFeature, secondThisPoint), secondThisPoint);
                        if (possibleDist < minDist)
                        {
                            distCase = 4;
                            minDist = possibleDist;
                        }
                        
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
            secondCurrentFeature(b->GetRandomFeature())
        {
        };

        // Lin-Canny algorithm is used here
        void UpdateCurrentFeatures()
        {
            while (tryUpdateCurrentFeature(firstBody, firstCurrentFeature, secondBody, secondCurrentFeature) |
                   tryUpdateCurrentFeature(secondBody, secondCurrentFeature, firstBody, firstCurrentFeature)) {}
        };


        void printDebug(const char* name)
        {
            v3 one = firstBody->GetFirstPointPosition(firstCurrentFeature);
            std::cout << name << ": firstBody feature coordinates are (" << one.x << "; " << one.y << ")";
            if (!firstCurrentFeature.isPoint)
            {
                one = firstBody->GetSecondPointPosition(firstCurrentFeature);
                std::cout << " and (" << one.x << "; " << one.y << ")";
            }
            std::cout << std::endl;

            one = secondBody->GetFirstPointPosition(secondCurrentFeature);
            std::cout << '\t' << "secondBody feature coordinates are (" << one.x << "; " << one.y << ")";
            if (!secondCurrentFeature.isPoint)
            {
                one = secondBody->GetSecondPointPosition(secondCurrentFeature);
                std::cout << " and (" << one.x << "; " << one.y << ")";
            }
            std::cout << std::endl;
        }
    };
};
