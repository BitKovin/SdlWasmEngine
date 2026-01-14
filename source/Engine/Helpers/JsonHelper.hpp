#pragma once
#include "../json.hpp"
#include "../glm.h"

#include "../Delay.hpp"
#include "../utility/hashed_string.hpp"

namespace nlohmann {


    template <>
    struct adl_serializer<hashed_string> {
        static void to_json(json& j, const hashed_string& hs) {
            j = hs.str();
        }

        static void from_json(const json& j, hashed_string& hs) {
            hs = hashed_string(j.get<std::string>());
        }
    };

    // Specialization for Delay
    template<>
    struct adl_serializer<Delay> {
        static void to_json(json& j, const Delay& d) {
            j = json{
                {"remainTime", d.GetRemainTime()},
                { "totalDuration", d.totalDuration },
                { "active", d.Active }
            };
        }

        static void from_json(const json& j, Delay& d) {
            if (j.contains("remainTime") && j["remainTime"].is_number()) {
                d.AddDelay(j["remainTime"].get<double>());
                d.totalDuration = j["totalDuration"].get<double>();
                d.Active = j["active"].get<bool>();
            }
        }
    };

    // vec2
    template<> struct adl_serializer<glm::vec2> {
        static void to_json(json& j, const glm::vec2& v) {
            j = { {"x", v.x}, {"y", v.y} };
        }
        static void from_json(const json& j, glm::vec2& v) {
            v.x = j.at("x").get<float>();
            v.y = j.at("y").get<float>();
        }
    };

    // vec3
    template<> struct adl_serializer<glm::vec3> {
        static void to_json(json& j, const glm::vec3& v) {
            j = { {"x", v.x}, {"y", v.y}, {"z", v.z} };
        }
        static void from_json(const json& j, glm::vec3& v) {
            v.x = j.at("x").get<float>();
            v.y = j.at("y").get<float>();
            v.z = j.at("z").get<float>();
        }
    };

    // vec4
    template<> struct adl_serializer<glm::vec4> {
        static void to_json(json& j, const glm::vec4& v) {
            j = { {"x", v.x}, {"y", v.y}, {"z", v.z}, {"w", v.w} };
        }
        static void from_json(const json& j, glm::vec4& v) {
            v.x = j.at("x").get<float>();
            v.y = j.at("y").get<float>();
            v.z = j.at("z").get<float>();
            v.w = j.at("w").get<float>();
        }
    };

    // ivec2
    template<> struct adl_serializer<glm::ivec2> {
        static void to_json(json& j, const glm::ivec2& v) {
            j = { {"x", v.x}, {"y", v.y} };
        }
        static void from_json(const json& j, glm::ivec2& v) {
            v.x = j.at("x").get<int>();
            v.y = j.at("y").get<int>();
        }
    };

    // ivec3
    template<> struct adl_serializer<glm::ivec3> {
        static void to_json(json& j, const glm::ivec3& v) {
            j = { {"x", v.x}, {"y", v.y}, {"z", v.z} };
        }
        static void from_json(const json& j, glm::ivec3& v) {
            v.x = j.at("x").get<int>();
            v.y = j.at("y").get<int>();
            v.z = j.at("z").get<int>();
        }
    };

    // ivec4
    template<> struct adl_serializer<glm::ivec4> {
        static void to_json(json& j, const glm::ivec4& v) {
            j = { {"x", v.x}, {"y", v.y}, {"z", v.z}, {"w", v.w} };
        }
        static void from_json(const json& j, glm::ivec4& v) {
            v.x = j.at("x").get<int>();
            v.y = j.at("y").get<int>();
            v.z = j.at("z").get<int>();
            v.w = j.at("w").get<int>();
        }
    };

    // uvec2
    template<> struct adl_serializer<glm::uvec2> {
        static void to_json(json& j, const glm::uvec2& v) {
            j = { {"x", v.x}, {"y", v.y} };
        }
        static void from_json(const json& j, glm::uvec2& v) {
            v.x = j.at("x").get<unsigned int>();
            v.y = j.at("y").get<unsigned int>();
        }
    };

    // uvec3
    template<> struct adl_serializer<glm::uvec3> {
        static void to_json(json& j, const glm::uvec3& v) {
            j = { {"x", v.x}, {"y", v.y}, {"z", v.z} };
        }
        static void from_json(const json& j, glm::uvec3& v) {
            v.x = j.at("x").get<unsigned int>();
            v.y = j.at("y").get<unsigned int>();
            v.z = j.at("z").get<unsigned int>();
        }
    };

    // uvec4
    template<> struct adl_serializer<glm::uvec4> {
        static void to_json(json& j, const glm::uvec4& v) {
            j = { {"x", v.x}, {"y", v.y}, {"z", v.z}, {"w", v.w} };
        }
        static void from_json(const json& j, glm::uvec4& v) {
            v.x = j.at("x").get<unsigned int>();
            v.y = j.at("y").get<unsigned int>();
            v.z = j.at("z").get<unsigned int>();
            v.w = j.at("w").get<unsigned int>();
        }
    };

    // bvec3
    template<> struct adl_serializer<glm::bvec3> {
        static void to_json(json& j, const glm::bvec3& v) {
            j = { {"x", v.x}, {"y", v.y}, {"z", v.z} };
        }
        static void from_json(const json& j, glm::bvec3& v) {
            v.x = j.at("x").get<bool>();
            v.y = j.at("y").get<bool>();
            v.z = j.at("z").get<bool>();
        }
    };

    template<> struct adl_serializer<glm::quat> {
        static void to_json(json& j, const glm::quat& q) {
            j = {
                { "x", q.x },
                { "y", q.y },
                { "z", q.z },
                { "w", q.w }
            };
        }

        static void from_json(const json& j, glm::quat& q) {
            q.x = j.at("x").get<float>();
            q.y = j.at("y").get<float>();
            q.z = j.at("z").get<float>();
            q.w = j.at("w").get<float>();
        }
    };


    // mat4
    template<> struct adl_serializer<glm::mat4> {
        static void to_json(json& j, const glm::mat4& m) {
            j = {
                { "col0", { m[0][0], m[0][1], m[0][2], m[0][3] } },
                { "col1", { m[1][0], m[1][1], m[1][2], m[1][3] } },
                { "col2", { m[2][0], m[2][1], m[2][2], m[2][3] } },
                { "col3", { m[3][0], m[3][1], m[3][2], m[3][3] } }
            };
        }
        static void from_json(const json& j, glm::mat4& m) {
            for (int col = 0; col < 4; ++col)
                for (int row = 0; row < 4; ++row)
                    m[col][row] = j.at("col" + std::to_string(col)).at(row).get<float>();
        }
    };


} // namespace nlohmann
