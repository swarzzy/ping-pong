#pragma once

#include <vector>

#include "../Math.h"

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
        std::vector<v3>::const_iterator pointIterator;

        BodyFeature(std::vector<v3>::const_iterator it, bool isP)
        {
             isPoint = isP;
             pointIterator = it;
        }
    };

    class Body
    {
    protected:
        m3x3 transform;

    public:
        virtual BodyFeature GetRandomFeature() const = 0;
        virtual bool TryUpdateToPreviousFeature(BodyFeature& feature, const v3& otherPoint) const = 0;
        virtual bool TryUpdateToNextFeature(BodyFeature& feature, const v3& otherPoint) const = 0;
        virtual void UpdateToNextFeature(BodyFeature& feature) const = 0;
        virtual void UpdateToPreviousFeature(BodyFeature& feature) const = 0;
        virtual v3 GetFirstPointPosition(const BodyFeature& feature) const = 0;
        virtual v3 GetSecondPointPosition(const BodyFeature& feature) const = 0;
        virtual v3 GetPointNearestToOther(const BodyFeature& feature, const v3& otherPoint) const = 0;
        virtual inline BodyType Type() const = 0;

        v3 GetPointPosition(const v3& point) const
        {
            return transform * point;
        }
    };

    // there is no checks of convexness inside, so coordinates of vertices should satisfy convexness when passed
    class ConvexBody : public Body
    {
    private:
        std::vector<v3> Vertices;

        inline std::vector<v3>::const_iterator getPreviousIterator(std::vector<v3>::const_iterator currentIterator) const
        {
            return currentIterator == Vertices.begin() ? (Vertices.end() - 1) : (currentIterator - 1);
        };

        inline std::vector<v3>::const_iterator getNextIterator(std::vector<v3>::const_iterator currentIterator) const
        {
            return currentIterator + 1 == Vertices.end() ? Vertices.begin() : (currentIterator + 1);
        };

    public:
        // points should be passed clockwise
        ConvexBody(std::vector<v3> vertices, m3x3 transf) : Vertices(vertices)
        {
            transform = transf;
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
            std::vector<v3>::const_iterator& currentIterator = feature.pointIterator;
            v3 currentPoint = GetPointPosition(*currentIterator);
            v3 vectorToOtherPoint = otherPoint - currentPoint;

            if (feature.isPoint)
            {
                std::vector<v3>::const_iterator previousIterator = getPreviousIterator(currentIterator);
                const v3 previousPoint = GetPointPosition(*previousIterator);
                const v3 vectorToPreviousPoint = previousPoint - currentPoint;
                const v3 voronoiEdgeVector = {vectorToPreviousPoint.y, -vectorToPreviousPoint.x, 0.0}; // rotate 90 degree clockwise
                
                if (vectorToOtherPoint.x * voronoiEdgeVector.y - vectorToOtherPoint.y * voronoiEdgeVector.x > 0.0)
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

                if (vectorToOtherPoint.x * voronoiEdgeVector.y - vectorToOtherPoint.y * voronoiEdgeVector.x > 0.0 &&
                    vectorToNextPoint.x * vectorToOtherPoint.y - vectorToNextPoint.y * vectorToOtherPoint.x > 0.0) // count feature only as a 
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
            std::vector<v3>::const_iterator& currentIterator = feature.pointIterator;
            const v3 currentPoint = GetPointPosition(*currentIterator);

            std::vector<v3>::const_iterator nextIterator = getNextIterator(currentIterator);
            const v3 nextPoint = GetPointPosition(*nextIterator);

            if (feature.isPoint)
            {
                const v3 vectorToOtherPoint = otherPoint - currentPoint;
                const v3 vectorToNextPoint = nextPoint - currentPoint;
                const v3 voronoiEdgeVector = {-vectorToNextPoint.y, vectorToNextPoint.x, 0.0};

                if (voronoiEdgeVector.x * vectorToOtherPoint.y - voronoiEdgeVector.y * vectorToOtherPoint.x > 0.0)
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

                if (voronoiEdgeVector.x * vectorToOtherPoint.y - voronoiEdgeVector.y * vectorToOtherPoint.x > 0.0)
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
