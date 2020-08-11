#pragma once

#include "../containers/Array.h"
#include "../Math.h"
#include "../Render.h"


#include <iostream>

namespace Physics
{
    class ConvexBody;

    enum class BodyType
    {
        CONVEX = 0, SPHERE
    };

    struct BodyFeature
    {
    public:
        bool isPoint;

    private:
        friend class ConvexBody;
        Containers::Array<v3>::ConstIterator pointIterator;

        BodyFeature(Containers::Array<v3>::ConstIterator it, bool isP) :
             isPoint(isP),
             pointIterator(it)
        {}
    };

    class Body
    {
    protected:
        m3x3 transform;
        f32 lastTimeUpdated = 0.0f;
        v3 centerOfMass;
        v2 velocity = {0.0, 0.0}; // units / seconds

    public:
        virtual BodyFeature GetRandomFeature() const = 0;
        virtual bool TryUpdateToPreviousFeature(BodyFeature& feature, const v3& otherPoint) const = 0;
        virtual bool TryUpdateToNextFeature(BodyFeature& feature, const v3& otherPoint) const = 0;
        virtual void UpdateToNextFeature(BodyFeature& feature) const = 0;
        virtual void UpdateToPreviousFeature(BodyFeature& feature) const = 0;
        virtual v3 GetFirstPointPosition(const BodyFeature& feature) const = 0;
        virtual v3 GetSecondPointPosition(const BodyFeature& feature) const = 0;
        virtual v3 GetPointNearestToOther(const BodyFeature& feature, const v3& otherPoint) const = 0;
        virtual void Draw(RenderQueue* queue) const = 0;
        virtual inline BodyType Type() const = 0;

        v3 GetPointPosition(const v3& point) const
        {
            return transform * point;
        }

        void AdvanceToTime(f32 time)
        {
            f32 timeDelta = time - lastTimeUpdated;
            lastTimeUpdated = time;

            transform = Translate(velocity * timeDelta) * transform;
        }

        v2 GetVelocity() const
        {
            return velocity;
        }

        void SetVelocity(v2 newVelocity)
        {
            velocity = newVelocity;
        }

        v3 GetCenterOfMass() const
        {
            return transform * centerOfMass;
        }
    };

    // there is no checks of convexness inside, so coordinates of vertices should satisfy convexness when passed
    class ConvexBody : public Body
    {
    private:
        Containers::Array<v3> Vertices;

        inline Containers::Array<v3>::ConstIterator getPreviousIterator(Containers::Array<v3>::ConstIterator currentIterator) const
        {
            return currentIterator == Vertices.begin() ? (Vertices.end() - 1) : (currentIterator - 1);
        };

        inline Containers::Array<v3>::ConstIterator getNextIterator(Containers::Array<v3>::ConstIterator currentIterator) const
        {
            return currentIterator + 1 == Vertices.end() ? Vertices.begin() : (currentIterator + 1);
        };

    public:
        // points should be passed clockwise
        explicit ConvexBody(Containers::Array<v3> vertices) : Vertices(vertices)
        {
            transform = M3x3(1.0);

            centerOfMass = {0.0, 0.0, 0.0};

            for (const v3& vert : Vertices)
            {
                centerOfMass += vert;
            }
            centerOfMass /= (f32)Vertices.Size();
        }

        ConvexBody(Containers::Array<v3> vertices, m3x3 transf) : Vertices(vertices)
        {
            transform = transf;
        }

        void Draw(RenderQueue* queue) const
        {
            for (int i = 1; i < Vertices.Size(); ++i)
            {
                DrawLine(queue, transform * Vertices[i - 1], transform * Vertices[i], {0.0, 0.0, 0.0, 1.0});
            }
            DrawLine(queue, transform * Vertices[0], transform * Vertices.Back(), {0.0, 0.0, 0.0, 1.0});
        }

        v3 GetFirstPointPosition(const BodyFeature& feature) const
        {
            return transform * (*feature.pointIterator);
        }

        v3 GetSecondPointPosition(const BodyFeature& feature) const
        {
            return transform * (*getNextIterator(feature.pointIterator));
        }

        inline BodyFeature GetRandomFeature() const
        {
            return {Vertices.begin(), true};
        }

        // test feature Voronoi region against other point
        bool TryUpdateToPreviousFeature(BodyFeature& feature, const v3& otherPoint) const
        {
            Containers::Array<v3>::ConstIterator& currentIterator = feature.pointIterator;
            v3 currentPoint = GetPointPosition(*currentIterator);
            v3 vectorToOtherPoint = otherPoint - currentPoint;

            if (feature.isPoint)
            {
                Containers::Array<v3>::ConstIterator previousIterator = getPreviousIterator(currentIterator);
                const v3 previousPoint = GetPointPosition(*previousIterator);
                const v3 vectorToPreviousPoint = previousPoint - currentPoint;
                const v3 voronoiEdgeVector = {vectorToPreviousPoint.y, -vectorToPreviousPoint.x, 0.0}; // rotate 90 degree clockwise
                
                if (vectorToOtherPoint.x * voronoiEdgeVector.y - vectorToOtherPoint.y * voronoiEdgeVector.x > -F32::Eps)
                {
                    return false;
                }
                else
                {
                    feature.isPoint = false;
                    currentIterator = previousIterator;
                    return true;
                }
            }
            else
            {
                const v3 nextPoint = GetPointPosition(*getNextIterator(currentIterator));
                const v3 vectorToNextPoint = nextPoint - currentPoint;
                const v3 voronoiEdgeVector = {-vectorToNextPoint.y, vectorToNextPoint.x, 0.0}; // rotate 90 degrees counter clockwise

                if (vectorToOtherPoint.x * voronoiEdgeVector.y - vectorToOtherPoint.y * voronoiEdgeVector.x > -F32::Eps &&
                    vectorToNextPoint.x * vectorToOtherPoint.y - vectorToNextPoint.y * vectorToOtherPoint.x > -F32::Eps) // count feature only as a 
                {                                                                                                  // previous edge of voronoi
                    return false;                                                                                  // region
                }
                else
                {
                    feature.isPoint = true;
                    return true;
                }
            }
        }

        // Almost copy-paste from TryUpdatePreviousFeature except we don't test other point against edge itself.
        // That's why we should check next feature before previous.
        bool TryUpdateToNextFeature(BodyFeature& feature, const v3& otherPoint) const
        {
            Containers::Array<v3>::ConstIterator& currentIterator = feature.pointIterator;
            const v3 currentPoint = GetPointPosition(*currentIterator);

            Containers::Array<v3>::ConstIterator nextIterator = getNextIterator(currentIterator);
            const v3 nextPoint = GetPointPosition(*nextIterator);

            if (feature.isPoint)
            {
                const v3 vectorToOtherPoint = otherPoint - currentPoint;
                const v3 vectorToNextPoint = nextPoint - currentPoint;
                const v3 voronoiEdgeVector = {-vectorToNextPoint.y, vectorToNextPoint.x, 0.0};

                if (voronoiEdgeVector.x * vectorToOtherPoint.y - voronoiEdgeVector.y * vectorToOtherPoint.x > -F32::Eps)
                {
                    return false;
                }
                else
                {
                    feature.isPoint = false;
                    return true;
                }
            }
            else
            {
                const v3 vectorToOtherPoint = otherPoint - nextPoint;
                const v3 vectorFromNextPoint = currentPoint - nextPoint;
                const v3 voronoiEdgeVector = {vectorFromNextPoint.y, -vectorFromNextPoint.x, 0.0};

                if (voronoiEdgeVector.x * vectorToOtherPoint.y - voronoiEdgeVector.y * vectorToOtherPoint.x > -F32::Eps)
                {
                    return false;
                }
                else
                {
                    feature.isPoint = true;
                    currentIterator = nextIterator;
                    return true;
                }
            }
        }

        void UpdateToNextFeature(BodyFeature& feature) const
        {
            if (feature.isPoint)
            {
                feature.isPoint = false;
            }
            else
            {
                feature.isPoint = true;
                feature.pointIterator = getNextIterator(feature.pointIterator);
            }
        }

        void UpdateToPreviousFeature(BodyFeature& feature) const
        {
            if (feature.isPoint)
            {
                feature.isPoint = false;
                feature.pointIterator = getPreviousIterator(feature.pointIterator);
            }
            else
            {
                feature.isPoint = true;
            }
        }

        v3 GetPointNearestToOther(const BodyFeature& feature, const v3& otherPoint) const
        {
            const v3 firstPoint = GetFirstPointPosition(feature);
            const v3 secondPoint = GetSecondPointPosition(feature);
            const v3 featureVector = secondPoint - firstPoint;
            const v3 hypotVector = otherPoint - firstPoint;

            //we don't really need this one
            const v3 orthoVector = {featureVector.y, -featureVector.x, 0.0};

            // now let's solve "hypotVector = x * featureVector + y * orthoVector"
            const f32 determinant = featureVector.x * orthoVector.y - featureVector.y * orthoVector.x; // == 0 <=> featureVector == 0
            const f32 x = ((hypotVector.x * orthoVector.y) - (hypotVector.y * orthoVector.x)) / determinant;

            if (abs(x) < 1.0)
            {
                return firstPoint + x * featureVector;
            }
            else if (x > 0.0)
            {
                return secondPoint;
            }
            else
            {
                return firstPoint;
            }
        }

        inline BodyType Type() const
        {
            return BodyType::CONVEX;
        }
    };

    
};
