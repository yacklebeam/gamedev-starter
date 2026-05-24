#include "bifrost_collision.h"
#include <cmath>
#include <limits>

namespace
{

std::vector<glm::vec2> GetWorldVertices(const bifrost::Hitbox& h, glm::vec2 pos, float angle)
{
    std::vector<glm::vec2> verts;
    verts.reserve(h.offsets.size());
    float c = std::cos(angle);
    float s = std::sin(angle);
    for (const auto& offset : h.offsets)
    {
        glm::vec2 local = h.origin + offset;
        verts.push_back(pos + glm::vec2(local.x * c - local.y * s,
                                        local.x * s + local.y * c));
    }
    return verts;
}

std::vector<glm::vec2> GetAxes(const std::vector<glm::vec2>& verts)
{
    std::vector<glm::vec2> axes;
    axes.reserve(verts.size());
    for (size_t i = 0; i < verts.size(); i++)
    {
        glm::vec2 edge = verts[(i + 1) % verts.size()] - verts[i];
        axes.push_back(glm::normalize(glm::vec2(-edge.y, edge.x)));
    }
    return axes;
}

void Project(const std::vector<glm::vec2>& verts, glm::vec2 axis, float& out_min, float& out_max)
{
    out_min = out_max = glm::dot(verts[0], axis);
    for (size_t i = 1; i < verts.size(); i++)
    {
        float p = glm::dot(verts[i], axis);
        if (p < out_min) out_min = p;
        if (p > out_max) out_max = p;
    }
}

// Returns t in [0,1] along segment p1->p2 where it intersects q1->q2, or -1 if no intersection.
float SegmentIntersectT(glm::vec2 p1, glm::vec2 p2, glm::vec2 q1, glm::vec2 q2)
{
    glm::vec2 r = p2 - p1;
    glm::vec2 s = q2 - q1;
    float denom = r.x * s.y - r.y * s.x;
    if (std::abs(denom) < 1e-10f)
        return -1.0f;
    glm::vec2 diff = q1 - p1;
    float t = (diff.x * s.y - diff.y * s.x) / denom;
    float u = (diff.x * r.y - diff.y * r.x) / denom;
    if (t >= 0.0f && t <= 1.0f && u >= 0.0f && u <= 1.0f)
        return t;
    return -1.0f;
}

} // anonymous namespace

namespace bifrost
{

Hitbox GenRectHitbox(glm::vec2 origin, glm::vec2 size)
{
    glm::vec2 half = size * 0.5f;
    return Hitbox{origin, {
        {-half.x, -half.y},
        { half.x, -half.y},
        { half.x,  half.y},
        {-half.x,  half.y}
    }};
}

CollisionResult GetCollision(Hitbox a, glm::vec2 pos_a, float angle_a, Hitbox b, glm::vec2 pos_b, float angle_b)
{
    auto verts_a = GetWorldVertices(a, pos_a, angle_a);
    auto verts_b = GetWorldVertices(b, pos_b, angle_b);

    auto axes_a = GetAxes(verts_a);
    auto axes_b = GetAxes(verts_b);

    float min_overlap = std::numeric_limits<float>::max();
    glm::vec2 mtv{};

    auto test_axes = [&](const std::vector<glm::vec2>& axes) -> bool
    {
        for (const auto& axis : axes)
        {
            float min_a, max_a, min_b, max_b;
            Project(verts_a, axis, min_a, max_a);
            Project(verts_b, axis, min_b, max_b);

            if (max_a <= min_b || max_b <= min_a)
                return false;

            float overlap = std::min(max_a, max_b) - std::max(min_a, min_b);
            if (overlap < min_overlap)
            {
                min_overlap = overlap;
                mtv = axis;
            }
        }
        return true;
    };

    if (!test_axes(axes_a) || !test_axes(axes_b))
        return {false, {}};

    // Ensure penetration vector points from b toward a (pushes a out of b)
    if (glm::dot(pos_a - pos_b, mtv) < 0.0f)
        mtv = -mtv;

    return {true, mtv * min_overlap};
}

bool CheckCollision(Hitbox a, glm::vec2 pos_a, float angle_a, Hitbox b, glm::vec2 pos_b, float angle_b)
{
    return GetCollision(a, pos_a, angle_a, b, pos_b, angle_b).hit;
}

bool ContainsPoint(Hitbox h, glm::vec2 pos, float angle, glm::vec2 point)
{
    auto verts = GetWorldVertices(h, pos, angle);
    auto axes = GetAxes(verts);

    for (const auto& axis : axes)
    {
        float min_h, max_h;
        Project(verts, axis, min_h, max_h);
        float p = glm::dot(point, axis);
        if (p < min_h || p > max_h)
            return false;
    }
    return true;
}

LineIntersectionResult GetLineIntersection(Hitbox h, glm::vec2 pos, float angle, glm::vec2 line_start, glm::vec2 line_end)
{
    auto verts = GetWorldVertices(h, pos, angle);

    float best_t = std::numeric_limits<float>::max();
    glm::vec2 best_normal{};

    for (size_t i = 0; i < verts.size(); i++)
    {
        glm::vec2 a = verts[i];
        glm::vec2 b = verts[(i + 1) % verts.size()];
        float t = SegmentIntersectT(line_start, line_end, a, b);
        if (t >= 0.0f && t < best_t)
        {
            best_t = t;
            glm::vec2 edge = b - a;
            best_normal = glm::normalize(glm::vec2(-edge.y, edge.x));
        }
    }

    if (best_t <= 1.0f)
        return {true, line_start + best_t * (line_end - line_start), best_normal};

    return {false, {}, {}};
}

bool CheckLineIntersection(Hitbox h, glm::vec2 pos, float angle, glm::vec2 line_start, glm::vec2 line_end)
{
    return GetLineIntersection(h, pos, angle, line_start, line_end).hit;
}

void DrawHitbox(Camera2d camera, Hitbox hitbox, glm::vec2 pos, float angle, glm::vec3 color)
{
    return DrawHitbox(camera, hitbox, pos, angle, glm::vec4(color, 1.0f));
}

void DrawHitbox(Camera2d camera, Hitbox hitbox, glm::vec2 pos, float angle, glm::vec4 color)
{
    auto verts = GetWorldVertices(hitbox, pos, angle);
    for (size_t i = 0; i < verts.size(); i++)
        DrawLine(camera, verts[i], verts[(i + 1) % verts.size()], 1.0f, color);
}

} // namespace bifrost
