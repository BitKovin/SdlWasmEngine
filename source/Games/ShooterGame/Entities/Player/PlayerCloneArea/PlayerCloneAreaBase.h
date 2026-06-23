#pragma once

#include <Entities/Brushes/AreaBase.hpp>

class PlayerCloneAreaBase : public AreaBase
{
public:
	PlayerCloneAreaBase();

	mat4 playerTransformation = glm::identity<mat4>();

	virtual mat4 GetTransformation() { return playerTransformation; };

private:

};