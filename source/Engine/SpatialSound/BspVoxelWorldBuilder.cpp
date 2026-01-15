#include "BspVoxelWorldBuilder.h"
#include "../Level.hpp"
#include "Helpers.h"

#include "../Physics.h"
#include "../Helpers/StringHelper.h"

static std::string SelectLeadingSurface(const std::vector<std::string>& surfaces)
{
	if (surfaces.empty())
		return {};

	std::unordered_map<std::string, int> counts;
	for (const auto& s : surfaces)
		counts[s]++;

	const std::string* best = nullptr;
	int bestCount = 0;

	for (const auto& [name, count] : counts)
	{
		if (count > bestCount)
		{
			best = &name;
			bestCount = count;
		}
	}

	return best ? *best : std::string{};
}

static const glm::vec3 SAMPLE_DIRS[] =
{
    {  0,  1,  0 }, // up
    {  0, -1,  0 }, // down
    {  1,  0,  0 }, // +X
    { -1,  0,  0 }, // -X
    {  0,  0,  1 }, // +Z
    {  0,  0, -1 }, // -Z
};

uint8_t BspVoxelWorldBuilder::GetValue(const glm::vec3& worldPos) const
{

    vec3 wPos = VoxelToWorldPos(worldPos);

    int density =
        (Level::Current->BspData.FindClusterAtPosition(wPos) >= 0)
        ? 0
        : 1;

    if (density == 0)
        return density;

    std::vector<std::string> hitSurfaces;
    hitSurfaces.reserve(std::size(SAMPLE_DIRS));

    const float rayOffset = 1.0f;

    for (const glm::vec3& dir : SAMPLE_DIRS)
    {
        glm::vec3 start = wPos + dir * rayOffset;
        glm::vec3 end = wPos;

        auto hit = Physics::LineTrace(
            start,
            end,
            BodyType::WorldOpaque | BodyType::WorldSkybox
        );

        if (!hit.hasHit)
            continue;

        if (!hit.surfaceName.empty())
            hitSurfaces.push_back(hit.surfaceName);
    }

    std::string leadingSurface = SelectLeadingSurface(hitSurfaces);

    // --- Surface classification (future-proof) ---
    if (!leadingSurface.empty())
    {
        if (StringHelper::Contains(leadingSurface, "/skies/"))
        {
            density = 2; // sky
        }
        else
        {
            density = 1;
        }

    }
    else
    {
        density = 0;
    }

    return density;
}

