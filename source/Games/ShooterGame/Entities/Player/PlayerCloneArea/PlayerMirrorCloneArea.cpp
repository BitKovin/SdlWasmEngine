#include "PlayerCloneAreaBase.h"

class PlayerMirrorCloneArea : public PlayerCloneAreaBase
{
public:
	
	std::string target = "";

	void FromData(EntityData data) override
	{

		Entity::FromData(data);

		target = data.GetPropertyString("target");

	}

	void Start() override
	{

		PlayerCloneAreaBase::Start();

		auto transformPoint = Level::Current->FindEntityWithName(target);

		assert(transformPoint);

		playerTransformation = GetMirrorMatrix(transformPoint->Position, MathHelper::GetForwardVector(transformPoint->Rotation));

	}

private:

	glm::mat4 GetMirrorMatrix(const glm::vec3& point, const glm::vec3& normal)
	{
		const glm::vec3 n = glm::normalize(normal);

		// Householder reflection: I - 2 * n * n^T, reflects any vector across
		// the plane through the origin with normal n.
		glm::mat3 reflect3 = glm::mat3(1.0f) - 2.0f * glm::outerProduct(n, n);

		glm::mat4 reflect4 = glm::mat4(reflect3);

		// The plane doesn't pass through the origin, so translate to the plane,
		// reflect, then translate back: T(point) * R * T(-point).
		glm::vec3 translatedPart = point - reflect3 * point;
		reflect4[3] = glm::vec4(translatedPart, 1.0f);

		return reflect4;
	}

};

REGISTER_ENTITY(PlayerMirrorCloneArea, "area_mirror")