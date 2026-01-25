#include "Frustum.h"

#include "World.h"

bool Frustum::isChunkInFrustum(const glm::vec3& minCorner, const glm::vec3& maxCorner, const FrustumPlanes& frumstumPlanes) {
    const std::array<glm::vec3, 8> corners = {
        glm::vec3(minCorner.x, minCorner.y, minCorner.z), // Bottom-left-near
        glm::vec3(maxCorner.x, minCorner.y, minCorner.z), // Bottom-right-near
        glm::vec3(minCorner.x, maxCorner.y, minCorner.z), // Top-left-near
        glm::vec3(maxCorner.x, maxCorner.y, minCorner.z), // Top-right-near
        glm::vec3(minCorner.x, minCorner.y, maxCorner.z), // Bottom-left-far
        glm::vec3(maxCorner.x, minCorner.y, maxCorner.z), // Bottom-right-far
        glm::vec3(minCorner.x, maxCorner.y, maxCorner.z), // Top-left-far
        glm::vec3(maxCorner.x, maxCorner.y, maxCorner.z)  // Top-right-far
    };

    // Check corners first
    for (const auto& corner : corners) {
        if (isPointInFrustum(corner, frumstumPlanes)) {
            return true; // Chunk is visible if any corner is inside
        }
    }


    for (int x = static_cast<int>(minCorner.x); x <= static_cast<int>(maxCorner.x); x += 4) { // Adjust step size for accuracy
        for (int y = static_cast<int>(minCorner.y); y <= static_cast<int>(maxCorner.y); y += 4) {
            for (int z = static_cast<int>(minCorner.z); z <= static_cast<int>(maxCorner.z); z += 4) {
                glm::vec3 point(x, y, z);
                if (isPointInFrustum(point, frumstumPlanes)) {
                    return true; // At least one point inside the chunk is visible
                }
            }
        }
    }

    return false; // No points are visible
}
bool Frustum::isPointInFrustum(const glm::vec3& point, const FrustumPlanes& frustum) {

    if (frustum.topFace.getSignedDistanceToPlane(point) > 0 &&
        frustum.bottomFace.getSignedDistanceToPlane(point) > 0 &&
        frustum.rightFace.getSignedDistanceToPlane(point) > 0 &&
        frustum.leftFace.getSignedDistanceToPlane(point) > 0 &&
        frustum.farFace.getSignedDistanceToPlane(point) > 0 &&
        frustum.nearFace.getSignedDistanceToPlane(point) > 0) {
        return true;
    }

    return false;}
Frustum::FrustumPlanes Frustum::createFrustumFromCamera(const Camera& cam, float aspect, float fovY,
                                                                float zNear, float zFar)
{
    FrustumPlanes     frustum;
    const float halfVSide = zFar * tanf(fovY * .5f);
    const float halfHSide = halfVSide * aspect;
    const glm::vec3 frontMultFar = zFar * cam.Front;

    frustum.nearFace = { cam.position + zNear * cam.Front, cam.Front };
    frustum.farFace = { cam.position + frontMultFar, -cam.Front };
    frustum.rightFace = { cam.position,
                            glm::cross(frontMultFar - cam.Right * halfHSide, cam.Up) };
    frustum.leftFace = { cam.position,
                            glm::cross(cam.Up,frontMultFar + cam.Right * halfHSide) };
    frustum.topFace = { cam.position,
                            glm::cross(cam.Right, frontMultFar - cam.Up * halfVSide) };
    frustum.bottomFace = { cam.position,
                            glm::cross(frontMultFar + cam.Up * halfVSide, cam.Right) };

    return frustum;
}