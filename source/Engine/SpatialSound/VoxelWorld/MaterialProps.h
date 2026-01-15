#pragma once

#include <map>

struct MaterialProps {
    std::map<uint8_t, float> absorption;
    std::map<uint8_t, float> reflectivity;

    float GetAbsorption(uint8_t mat) const {
        auto it = absorption.find(mat);
        return (it != absorption.end()) ? it->second : 0.1f; // Default absorption
    }

    float GetReflectivity(uint8_t mat) const {
        auto it = reflectivity.find(mat);
        return (it != reflectivity.end()) ? it->second : 0.8f; // Default reflectivity
    }
};