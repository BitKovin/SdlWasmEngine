#include "transformPoint.h"

void TransformPoint::FromData(EntityData data)
{

	Entity::FromData(data);

	vec3 angles = data.GetPropertyVectorRotation("angles");

	Rotation = EntityData::ConvertRotation(angles, true);

	DebugDraw::Line(Position, Position + MathHelper::GetForwardVector(Rotation), 50,0.1f);

}
