#pragma once

#include "bifrost.h"
#include <vector>

namespace bifrost
{
    struct Hitbox
    {
        glm::vec2 origin;
        std::vector<glm::vec2> offsets;
    };

    struct CollisionResult
    {
        bool hit;
        glm::vec2 penetration;
    };

    struct LineIntersectionResult
    {
        bool hit;
        glm::vec2 point;
        glm::vec2 normal;
    };

    Hitbox GenRectHitbox(glm::vec2 origin, glm::vec2 size);
    bool CheckCollision(Hitbox a, glm::vec2 pos_a, float angle_a, Hitbox b, glm::vec2 pos_b, float angle_b);
    CollisionResult GetCollision(Hitbox a, glm::vec2 pos_a, float angle_a, Hitbox b, glm::vec2 pos_b, float angle_b);
    bool ContainsPoint(Hitbox h, glm::vec2 pos, float angle, glm::vec2 point);
    bool CheckLineIntersection(Hitbox h, glm::vec2 pos, float angle, glm::vec2 line_start, glm::vec2 line_end);
    LineIntersectionResult GetLineIntersection(Hitbox h, glm::vec2 pos, float angle, glm::vec2 line_start, glm::vec2 line_end);

    void DrawHitbox(Camera2d camera, Hitbox hitbox, glm::vec2 pos, float angle, glm::vec3 color);
    void DrawHitbox(Camera2d camera, Hitbox hitbox, glm::vec2 pos, float angle, glm::vec4 color);
}
