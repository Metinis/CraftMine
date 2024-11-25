#ifndef FRUSTUM_H
#define FRUSTUM_H
#include "dual_quaternion.hpp"
#include "vec3.hpp"
#include <array>
#include "Input/Camera.h"


class Camera;

class Frustum {
public:
    struct Plane
    {
        glm::vec3 normal = { 0.f, 1.f, 0.f }; // unit vector
        float     distance = 0.f;        // Distance with origin

        Plane() = default;

        Plane(const glm::vec3& p1, const glm::vec3& norm)
            : normal(glm::normalize(norm)),
            distance(glm::dot(normal, p1))
        {}

        float getSignedDistanceToPlane(const glm::vec3& point) const
        {
            return glm::dot(normal, point) - distance;
        }  };
    struct FrustumPlanes
    {
        Plane topFace;
        Plane bottomFace;

        Plane rightFace;
        Plane leftFace;

        Plane farFace;
        Plane nearFace;
    };

    static bool isChunkInFrustum(const glm::vec3& minCorner, const glm::vec3& maxCorner, const FrustumPlanes& frumstumPlanes) ;
    static FrustumPlanes createFrustumFromCamera(const Camera& cam, float aspect, float fovY,
                                                                float zNear, float zFar);
    static bool isPointInFrustum(const glm::vec3& point, const FrustumPlanes& frustum);
};



#endif //FRUSTUM_H
