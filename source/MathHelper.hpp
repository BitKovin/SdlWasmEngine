#pragma once

#include "glm.h"

#include <algorithm>
#include <vector>
#include <stdexcept>
#include <cmath>

class MathHelper
{
public:

	// Converts degrees to radians.
	inline static float ToRadians(float degrees) {
		return glm::radians(degrees);
	}

	// Converts radians to degrees.
	inline static float ToDegrees(float radians) {
		return glm::degrees(radians);
	}

	// Constructs a quaternion from a rotation vector (pitch, yaw, roll)
	// following the XNA convention: Y (yaw), X (pitch), Z (roll)
	inline static glm::quat GetRotationQuaternion(const glm::vec3& rotation) {
		return quat(rotation / 180.0f * pi<float>());
	}

	// Transforms the given vector by the quaternion
	inline static glm::vec3 TransformVector(const glm::vec3& v, const glm::quat& q) {
		return q * v;
	}

	// Returns the forward vector given the Euler rotation.
	inline static glm::vec3 GetForwardVector(const glm::vec3& rotation) {
		return TransformVector(glm::vec3(0.f, 0.f, 1.f), GetRotationQuaternion(rotation));
	}

	// Returns the right vector given the Euler rotation.
	inline static glm::vec3 GetRightVector(const glm::vec3& rotation) {
		return TransformVector(glm::vec3(-1.f, 0.f, 0.f), GetRotationQuaternion(rotation));
	}

	// Returns the up vector given the Euler rotation.
	inline static glm::vec3 GetUpVector(const glm::vec3& rotation) {
		return TransformVector(glm::vec3(0.f, 1.f, 0.f), GetRotationQuaternion(rotation));
	}

	// Returns a vector with only the X and Z components (Y set to 0).
	inline static glm::vec3 XZ(const glm::vec3& vector) {
		return glm::vec3(vector.x, 0.f, vector.z);
	}

	// Fast inverse square root using the “Quake III” method.
	inline static float InvSqrt(float x) {
		float xhalf = 0.5f * x;
		int i = *reinterpret_cast<int*>(&x);
		i = 0x5f3759df - (i >> 1);
		x = *reinterpret_cast<float*>(&i);
		x = x * (1.5f - xhalf * x * x);
		return x;
	}

	// Fast normalization of a vector using InvSqrt.
	inline static glm::vec3 FastNormalize(const glm::vec3& v) 
	{
		float lengthSq = v.x * v.x + v.y * v.y + v.z * v.z;
		float invLen = InvSqrt(lengthSq);
		return v * invLen;
	}

	// Normalizes the vector (using the fast normalization).
	inline static glm::vec3 Normalized(const glm::vec3& v) {
		return FastNormalize(v);
	}

	// Snap to grid using a uniform grid size (uses ceiling as in the C# version).
	inline static glm::vec3 SnapToGrid(const glm::vec3& position, float gridSize) {
		if (gridSize <= 0)
			throw std::invalid_argument("Grid size must be greater than zero.");
		float snappedX = std::ceil(position.x / gridSize) * gridSize;
		float snappedY = std::ceil(position.y / gridSize) * gridSize;
		float snappedZ = std::ceil(position.z / gridSize) * gridSize;
		return glm::vec3(snappedX, snappedY, snappedZ);
	}

	// Snap to grid using a separate grid size per axis (uses floor as in the C# version).
	inline static glm::vec3 SnapToGrid(const glm::vec3& position, const glm::vec3& gridSize) {
		float snappedX = std::floor(position.x / gridSize.x) * gridSize.x;
		float snappedY = std::floor(position.y / gridSize.y) * gridSize.y;
		float snappedZ = std::floor(position.z / gridSize.z) * gridSize.z;
		return glm::vec3(snappedX, snappedY, snappedZ);
	}

	// Clamps each component of the vector between min and max.
	inline static glm::vec3 Clamp(const glm::vec3& v, float minVal, float maxVal) {
		return glm::clamp(v, glm::vec3(minVal), glm::vec3(maxVal));
	}

	// Clamps each component of a vec4 between min and max.
	inline static glm::vec4 Clamp(const glm::vec4& v, float minVal, float maxVal) {
		return glm::clamp(v, glm::vec4(minVal), glm::vec4(maxVal));
	}

	// Linearly interpolates between two floats.
	inline static float Lerp(float a, float b, float t) {
		return glm::mix(a, b, t);
	}

	// Linearly interpolates between two vectors componentwise.
	inline static glm::vec3 Lerp(const glm::vec3& a, const glm::vec3& b, const glm::vec3& t) {
		return glm::vec3(
			glm::mix(a.x, b.x, t.x),
			glm::mix(a.y, b.y, t.y),
			glm::mix(a.z, b.z, t.z)
		);
	}

	// Saturate clamps a float between 0 and 1.
	inline static float Saturate(float a) {
		return glm::clamp(a, 0.f, 1.f);
	}

	// Check if any element in a 4x4 matrix is NaN.
	inline static bool HasNan(const glm::mat4& matrix) {
		for (int i = 0; i < 4; ++i)
		{
			glm::vec4 row = matrix[i]; // GLM stores as columns; here we assume each index returns a column.
			if (std::isnan(row.x) || std::isnan(row.y) || std::isnan(row.z) || std::isnan(row.w))
				return true;
		}
		return false;
	}

	// Rotates a vector around an axis by a given angle (in degrees).
	inline static glm::vec3 RotateVector(const glm::vec3& vector, const glm::vec3& axis, float angleDegrees) {
		float angleRadians = ToRadians(angleDegrees);
		glm::mat3 rotationMatrix = glm::mat3(glm::rotate(angleRadians, axis));
		return rotationMatrix * vector;
	}


	// Returns Euler angles (in degrees) in the order (pitch, yaw, roll) from a quaternion.
	inline static glm::vec3 ToYawPitchRoll(const glm::quat& q) 
	{

		return glm::vec3(
			glm::degrees(glm::pitch(q)),
			glm::degrees(glm::yaw(q)),
			glm::degrees(glm::roll(q)));
	}



	// Calculates the look-at rotation (in degrees) for 3D positions.
	inline static glm::vec3 FindLookAtRotation(const glm::vec3& source, const glm::vec3& target) {
		glm::vec3 direction = target - source;
		float yaw = ToDegrees(std::atan2(direction.x, direction.z));
		float pitch = ToDegrees(-std::asin(direction.y / glm::length(direction)));
		return glm::vec3(pitch, yaw, 0.f);
	}

	// Calculates the look-at rotation (in radians) for 2D positions.
	inline static float FindLookAtRotation(const glm::vec2& origin, const glm::vec2& target) {
		glm::vec2 direction = target - origin;
		return std::atan2(direction.y, direction.x);
	}

	// Returns a rotation matrix from a rotation vector (in degrees).
	inline static glm::mat4 GetRotationMatrix(const glm::vec3& rotation) {
		return glm::mat4_cast(GetRotationQuaternion(rotation));
	}

	// Returns the world-to-local transformation matrix.
	inline static glm::mat4 WorldToLocal(const glm::mat4& worldTransform, const glm::mat4& relativeToWorldTransform) {
		return worldTransform * glm::inverse(relativeToWorldTransform);
	}

	// Calculates the minimum distance from a center point to a set of points.
	inline static float CalculateMinDistance(const glm::vec3& center, const std::vector<glm::vec3>& points) {
		if (points.empty())
			throw std::invalid_argument("Points list cannot be empty.");
		float minDist = std::numeric_limits<float>::max();
		for (const auto& p : points)
			minDist = std::min(minDist, glm::distance(center, p));
		return minDist;
	}

	// Calculates the maximum distance from a center point to a set of points.
	inline static float CalculateMaxDistance(const glm::vec3& center, const std::vector<glm::vec3>& points) {
		if (points.empty())
			throw std::invalid_argument("Points list cannot be empty.");
		float maxDist = 0.f;
		for (const auto& p : points)
			maxDist = std::max(maxDist, glm::distance(center, p));
		return maxDist;
	}

	// A simple transform struct.
	struct Transform {
		glm::vec3 Position = glm::vec3(0.f);
		glm::vec3 Rotation = glm::vec3(0.f); // In degrees
		glm::quat RotationQuaternion = glm::quat(1.f, 0.f, 0.f, 0.f);
		glm::vec3 Scale = glm::vec3(1.f);

		Transform() = default;

		// Linearly interpolates between two transforms.
		static Transform Lerp(const Transform& a, const Transform& b, float factor) {
			Transform result;
			result.Position = glm::mix(a.Position, b.Position, factor);
			result.RotationQuaternion = glm::slerp(a.RotationQuaternion, b.RotationQuaternion, factor);
			result.Rotation = ToYawPitchRoll(result.RotationQuaternion);
			result.Scale = glm::mix(a.Scale, b.Scale, factor);
			return result;
		}

		// Operator overloads (note: these are defined componentwise; you may need to adjust for your needs)
		Transform operator+(const Transform& other) const {
			Transform result;
			result.Position = Position + other.Position;
			result.Rotation = Rotation + other.Rotation;
			result.RotationQuaternion = RotationQuaternion * other.RotationQuaternion;
			result.Scale = Scale + other.Scale;
			return result;
		}

		Transform operator-(const Transform& other) const {
			Transform result;
			result.Position = Position - other.Position;
			result.Rotation = Rotation - other.Rotation;
			result.RotationQuaternion = RotationQuaternion * glm::inverse(other.RotationQuaternion);
			result.Scale = Scale - other.Scale;
			return result;
		}

		Transform operator*(const Transform& other) const {
			Transform result;
			result.Position = Position * other.Position;
			result.Rotation = Rotation * other.Rotation;
			result.RotationQuaternion = RotationQuaternion * glm::inverse(other.RotationQuaternion);
			result.Scale = Scale * other.Scale;
			return result;
		}

		Transform operator-() const {
			Transform result;
			result.Position = -Position;
			result.Rotation = -Rotation;
			result.RotationQuaternion = glm::quat(-RotationQuaternion.w, -RotationQuaternion.x, -RotationQuaternion.y, -RotationQuaternion.z);
			result.Scale = -Scale;
			return result;
		}

		// Converts the transform to a matrix.
		// Note: In GLM the typical order is T * R * S. Adjust if needed.
		inline glm::mat4 ToMatrix() const {
			// It is common to use translation * rotation * scale.
			glm::mat4 T = glm::translate(Position);
			glm::mat4 R = glm::mat4_cast(RotationQuaternion);
			glm::mat4 S = glm::scale(Scale);
			return T * R * S;
		}
	};

	// Normalizes an angle to the range [-180, 180].
	inline static float NormalizeAngle(float angle) {
		while (angle < -180.f) angle += 360.f;
		while (angle > 180.f) angle -= 360.f;
		return angle;
	}

	// Offsets an angle if it is exactly a multiple of 90.
	inline static float NonZeroAngle(float angle) {
		if (std::fmod(angle, 90.f) == 0.f)
			angle += 0.0001f;
		return angle;
	}

	// Normalizes each component of a vec3 angle.
	inline static glm::vec3 NormalizeAngles(const glm::vec3& angles) {
		return glm::vec3(NormalizeAngle(angles.x),
			NormalizeAngle(angles.y),
			NormalizeAngle(angles.z));
	}

	// Adjusts each component of a vec3 angle so none are exactly multiples of 90.
	inline static glm::vec3 NonZeroAngles(const glm::vec3& angles) {
		return glm::vec3(NonZeroAngle(angles.x),
			NonZeroAngle(angles.y),
			NonZeroAngle(angles.z));
	}

	// Returns the specified row of a 4x4 matrix (GLM is column-major).
	inline static glm::vec4 GetRow(const glm::mat4& matrix, int row) {
		// GLM provides a row() function in <glm/gtx/matrix_query.hpp> if available.
		// Otherwise, manually extract the row:
		return glm::vec4(matrix[0][row], matrix[1][row], matrix[2][row], matrix[3][row]);
	}

	// Decomposes a transformation matrix into translation, rotation (in degrees) and scale.
	inline static Transform DecomposeMatrix(const glm::mat4& matrix) {
		Transform transform;
		glm::vec3 scale;
		glm::quat rotation;
		glm::vec3 translation;
		glm::vec3 skew;
		glm::vec4 perspective;

		// glm::decompose returns true if successful.
		if (!glm::decompose(matrix, scale, rotation, translation, skew, perspective))
			return transform; // return default transform on failure

		transform.Position = translation;
		transform.Scale = scale;
		transform.RotationQuaternion = rotation;
		transform.Rotation = ToYawPitchRoll(rotation);
		transform.Rotation = NormalizeAngles(transform.Rotation);
		return transform;
	}


};