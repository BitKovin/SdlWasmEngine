// BVH.h
#pragma once
#include <vector>
#include <cstdint>
#include <limits>
#include <tuple>

#include "../../glm.h"

#include <bvh/v2/bvh.h>
#include <bvh/v2/vec.h>
#include <bvh/v2/ray.h>
#include <bvh/v2/node.h>
#include <bvh/v2/default_builder.h>
#include <bvh/v2/thread_pool.h>
#include <bvh/v2/executor.h>
#include <bvh/v2/stack.h>
#include <bvh/v2/tri.h>

struct Ray {
    glm::vec3 origin;
    glm::vec3 dir;
    glm::vec3 invDir;
    float tmin, tmax;

    Ray(const glm::vec3& o, const glm::vec3& d,
        float tmin = 0.0f, float tmax = 1e30f)
        : origin(o), dir(d), invDir(1.0f / d), tmin(tmin), tmax(tmax) {
    }
};

struct RayHit {
    float distance = 1e30f;
    glm::vec3 pos{};
    glm::vec3 normal{};
    bool hit = false;
    uint8_t material = 1; // always 1
};

class BVH {
public:
    BVH(const std::vector<glm::vec3>& vertices,
        const std::vector<uint32_t>& indices) {
        using Scalar = float;
        using Vec3 = bvh::v2::Vec<Scalar, 3>;
        using Tri = bvh::v2::Tri<Scalar, 3>;
        using BBox = bvh::v2::BBox<Scalar, 3>;
        using Node = bvh::v2::Node<Scalar, 3>;
        using PrecomputedTri = bvh::v2::PrecomputedTri<Scalar>;

        tris_.reserve(indices.size() / 3);
        for (size_t i = 0; i < indices.size() / 3; ++i) {
            size_t idx0 = indices[i * 3 + 0];
            size_t idx1 = indices[i * 3 + 1];
            size_t idx2 = indices[i * 3 + 2];
            Vec3 p0(vertices[idx0].x, vertices[idx0].y, vertices[idx0].z);
            Vec3 p1(vertices[idx1].x, vertices[idx1].y, vertices[idx1].z);
            Vec3 p2(vertices[idx2].x, vertices[idx2].y, vertices[idx2].z);
            tris_.emplace_back(p0, p1, p2);
        }

        bvh::v2::ThreadPool thread_pool;
        bvh::v2::ParallelExecutor executor(thread_pool);

        std::vector<BBox> bboxes(tris_.size());
        std::vector<Vec3> centers(tris_.size());
        executor.for_each(0, tris_.size(), [&](size_t begin, size_t end) {
            for (size_t i = begin; i < end; ++i) {
                bboxes[i] = tris_[i].get_bbox();
                centers[i] = tris_[i].get_center();
            }
            });

        typename bvh::v2::DefaultBuilder<Node>::Config config;
        config.quality = bvh::v2::DefaultBuilder<Node>::Quality::High;
        bvh_ = bvh::v2::DefaultBuilder<Node>::build(bboxes, centers, config);

        static constexpr bool should_permute = true;
        precomputed_tris_.resize(tris_.size());
        executor.for_each(0, tris_.size(), [&](size_t begin, size_t end) {
            for (size_t i = begin; i < end; ++i) {
                size_t j = should_permute ? bvh_.prim_ids[i] : i;
                precomputed_tris_[i] = PrecomputedTri(tris_[j]);
            }
            });
    }

    RayHit Intersect(const Ray& ray) const {
        using Scalar = float;
        using Vec3 = bvh::v2::Vec<Scalar, 3>;
        using BvhRay = bvh::v2::Ray<Scalar, 3>;
        using PrecomputedTri = bvh::v2::PrecomputedTri<Scalar>;

        BvhRay bray(Vec3(ray.origin.x, ray.origin.y, ray.origin.z),
            Vec3(ray.dir.x, ray.dir.y, ray.dir.z),
            ray.tmin, ray.tmax);

        static constexpr size_t stack_size = 64;
        bvh::v2::SmallStack<BvhType::Index, stack_size> stack;

        RayHit result;
        size_t prim_id = std::numeric_limits<size_t>::max();
        Scalar u, v;

        static constexpr bool should_permute = true;
        static constexpr bool IsAnyHit = false;
        static constexpr bool IsRobust = false;

        bvh_.template intersect<IsAnyHit, IsRobust>(bray, bvh_.get_root().index, stack,
            [&](size_t begin, size_t end) {
                bool found = false;
                for (size_t i = begin; i < end; ++i) {
                    size_t j = should_permute ? i : bvh_.prim_ids[i];
                    if (auto hit = precomputed_tris_[j].intersect(bray)) {
                        prim_id = i;
                        auto [t, hu, hv] = *hit;
                        bray.tmax = t;
                        u = hu;
                        v = hv;
                        found = true;
                    }
                }
                return found;
            });

        if (prim_id != std::numeric_limits<size_t>::max()) {
            result.hit = true;
            result.distance = bray.tmax;
            result.pos = ray.origin + result.distance * ray.dir;
            size_t original_tri_idx = bvh_.prim_ids[prim_id];
            const auto& tri = tris_[original_tri_idx];
            Vec3 e1 = tri.p1 - tri.p0;
            Vec3 e2 = tri.p2 - tri.p0;
            Vec3 n = bvh::v2::cross(e1, e2);
            result.normal = glm::normalize(glm::vec3(n[0], n[1], n[2]));
            result.material = 1;
        }
        else {
            result.hit = false;
            result.distance = 1e30f;
        }
        return result;
    }

    bool IntersectAny(const Ray& ray, float maxDist) const {
        using Scalar = float;
        using Vec3 = bvh::v2::Vec<Scalar, 3>;
        using BvhRay = bvh::v2::Ray<Scalar, 3>;
        using PrecomputedTri = bvh::v2::PrecomputedTri<Scalar>;

        BvhRay bray(Vec3(ray.origin.x, ray.origin.y, ray.origin.z),
            Vec3(ray.dir.x, ray.dir.y, ray.dir.z),
            ray.tmin, maxDist);

        static constexpr size_t stack_size = 64;
        bvh::v2::SmallStack<BvhType::Index, stack_size> stack;

        bool hit = false;

        static constexpr bool should_permute = true;
        static constexpr bool IsAnyHit = true;
        static constexpr bool IsRobust = false;

        bvh_.template intersect<IsAnyHit, IsRobust>(bray, bvh_.get_root().index, stack,
            [&](size_t begin, size_t end) {
                for (size_t i = begin; i < end; ++i) {
                    size_t j = should_permute ? i : bvh_.prim_ids[i];
                    if (precomputed_tris_[j].intersect(bray)) {
                        hit = true;
                        return true;
                    }
                }
                return false;
            });

        return hit;
    }

private:
    using Scalar = float;
    using Tri = bvh::v2::Tri<Scalar, 3>;
    using PrecomputedTri = bvh::v2::PrecomputedTri<Scalar>;
    using Node = bvh::v2::Node<Scalar, 3>;
    using BvhType = bvh::v2::Bvh<Node>;

    std::vector<Tri> tris_;
    std::vector<PrecomputedTri> precomputed_tris_;
    BvhType bvh_;
};