#pragma once

#include "glm.h"

#include <algorithm>
#include <vector>
#include <stdexcept>
#include <cmath>


class MathHelper
{
public:


	template<typename T>
	static T Interp(const T& current, const T& target, float deltaTime, float speed)
	{
		float t = 1.0f - expf(-speed * deltaTime); // smooth exponential approach
		return mix(current, target, t);
	}

	template<typename T, glm::qualifier Q>
	inline static glm::vec<3, T, Q> ClampLength(
		const glm::vec<3, T, Q>& v,
		T minLen,
		T maxLen)
	{
		T lenSq = glm::length2(v);

		if (lenSq == T(0))
			return v;

		T minSq = minLen * minLen;
		T maxSq = maxLen * maxLen;

		if (lenSq < minSq)
			return v * (minLen / glm::sqrt(lenSq));

		if (lenSq > maxSq)
			return v * (maxLen / glm::sqrt(lenSq));

		return v;
	}

	// Converts degrees to radians.
	inline static float ToRadians(float degrees) {
		return glm::radians(degrees);
	}

	// Converts radians to degrees.
	inline static float ToDegrees(float radians) {
		return glm::degrees(radians);
	}

	inline static float MapRange(
		float value,
		float inMin, float inMax,
		float outMin, float outMax,
		bool clamp = true)
	{
		if (inMin == inMax) // avoid divide by zero
			return outMin;

		float t = (value - inMin) / (inMax - inMin);

		if (clamp)
		{
			if (t < 0.0f) t = 0.0f;
			if (t > 1.0f) t = 1.0f;
		}

		return outMin + t * (outMax - outMin);
	}

	// Constructs a quaternion from a rotation vector (pitch, yaw, roll)
	// following the XNA convention: Y (yaw), X (pitch), Z (roll)
	inline static glm::quat GetRotationQuaternion(const glm::vec3& rotationDeg)
	{
		glm::vec3 r = glm::radians(rotationDeg);
		// note: yawPitchRoll takes (yaw, pitch, roll)
		return glm::yawPitchRoll(r.y, r.x, r.z);
	}

	// Transforms the given vector by the quaternion
	inline static glm::vec3 TransformVector(const glm::vec3& v, const glm::quat& q) {
		return q * v;
	}

	inline static glm::vec3 TransformVector(const glm::vec3& v, const glm::mat4& m) {
		return m * vec4(v,1);
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

	inline static glm::vec3 ProjectVelocityOnNormal(const glm::vec3& velocity,
		const glm::vec3& normal)
	{
		// Safety: ensure normal is normalized
		float lenSq = glm::length2(normal);
		if (lenSq < 1e-6f)
			return velocity;

		glm::vec3 n = normal * glm::inversesqrt(lenSq);

		// Remove normal component
		return velocity - n * glm::dot(velocity, n);
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

	inline static glm::vec3 RotateAroundPoint(
		const glm::vec3& point,
		const glm::vec3& pivot,
		const glm::vec3& euler)
	{
		glm::mat4 rot = MathHelper::GetRotationMatrix(euler);

		// pure rotation → w = 0
		glm::vec4 local = glm::vec4(point - pivot, 1.0f);

		glm::vec3 rotated = glm::vec3(rot * local);

		return pivot + rotated;
	}


	// startPosition: where we start measuring from
	// path: sequence of points we follow (absolute positions)
	// pathLength: desired total travel distance from startPosition along path
	inline static std::vector<glm::vec3> GetPathWithLength(const glm::vec3& startPosition,
		const std::vector<glm::vec3>& path,
		float pathLength)
	{

		constexpr float EPSILON = 1e-6f;

		std::vector<glm::vec3> result;
		result.reserve(path.size() + 1);

		// sanity
		if (pathLength <= 0.0f) {
			result.push_back(startPosition);
			return result;
		}

		glm::vec3 curr = startPosition;
		result.push_back(curr);

		float remaining = pathLength;

		for (size_t i = 0; i < path.size(); ++i) {
			const glm::vec3& next = path[i];
			glm::vec3 seg = next - curr;
			float segLen = glm::length(seg);

			// skip degenerate (zero-length) segments
			if (segLen <= EPSILON) {
				// still push the point so returned path follows original vertices
				// (you can omit this if you don't want duplicates)
				curr = next;
				result.push_back(curr);
				continue;
			}

			if (segLen <= remaining + EPSILON) {
				// we can take whole segment and continue
				result.push_back(next);
				remaining -= segLen;
				curr = next;

				if (remaining <= EPSILON) {
					// we've reached requested length exactly (or nearly)
					break;
				}
			}
			else {
				// we must stop inside this segment — interpolate the final point
				float t = remaining / segLen; // in (0,1)
				glm::vec3 finalPoint = curr + seg * t;
				result.push_back(finalPoint);
				remaining = 0.0f;
				break;
			}
		}

		// If remaining > 0 here, the provided path is shorter than pathLength;
		// we return the whole available path (total length < requested length).
		return result;
	}

	// Rotates a vector around an axis by a given angle (in degrees).
	inline static glm::vec3 RotateVector(const glm::vec3& vector, const glm::vec3& axis, float angleDegrees) {
		float angleRadians = ToRadians(angleDegrees);
		glm::mat3 rotationMatrix = glm::mat3(glm::rotate(angleRadians, axis));
		return rotationMatrix * vector;
	}

	// Rotates a vector around an axis by a given angle (in degrees).
	inline static glm::vec3 RotateVector(const glm::vec3& vector, const glm::vec3& angle) {
		glm::mat3 rotationMatrix = GetRotationMatrix(angle);
		return rotationMatrix * vector;
	}


	// Returns Euler angles (in degrees) in the order (pitch, yaw, roll) from a quaternion.
	inline static glm::vec3 ToYawPitchRoll(const glm::quat& q)
	{
		// Convert the quaternion to a rotation matrix in order to call the
		// next function.  (Presumably this entire procedure could be
		// optimized by inlining and simplifying the arithmetic.)
		glm::mat4 m = mat4_cast(q);

		// Now get the YPR angles.  The `euler_angles.hpp` header contains
		// many similar functions, but this one is the inverse of its
		// `yawPitchRoll` function.
		float x, y, z;
		glm::extractEulerAngleYXZ(m, y, x, z);

		// Package them as a vector for interface similarity with
		// `glm::eulerAngles` (but the order is different!).
		return glm::degrees(glm::vec3{ x,y,z });
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

		// Converts the transform to a matrix.
// Note: In GLM the typical order is T * R * S. Adjust if needed.
		inline glm::mat4 ToMatrixEuler() const {
			// It is common to use translation * rotation * scale.
			glm::mat4 T = glm::translate(Position);
			glm::mat4 R = MathHelper::GetRotationMatrix(Rotation);
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

	inline static vec3 CalculateBulletRotation(
		const vec3& shooterPos,
		const vec3& shooterVel,
		const vec3& targetPos,
		const vec3& targetVel,
		float bulletSpeed)
	{
		// Relative motion
		vec3 relativePos = targetPos - shooterPos;
		vec3 relativeVel = targetVel - shooterVel;

		float a = bulletSpeed * bulletSpeed - dot(relativeVel, relativeVel);
		float b = dot(relativePos, relativeVel);
		float c = dot(relativePos, relativePos);

		float discriminant = b * b + a * c;

		vec3 predictedTargetPosition = targetPos;

		// Check if intercept is possible
		if (discriminant >= 0.0f && std::abs(a) > 1e-6f)
		{
			float t = (b + std::sqrt(discriminant)) / a;
			if (t > 0.0f)
			{
				predictedTargetPosition = targetPos + targetVel * t;
			}
		}

		// Compute direction and convert to rotation
		vec3 direction = normalize(predictedTargetPosition - shooterPos);
		return MathHelper::FindLookAtRotation(vec3(), direction);
	}

	// Generates a world matrix for a spherical billboard with optional rotation (in radians)
	inline static glm::mat4 CreateBillboardMatrix(
		const glm::vec3& billboardPos,
		const glm::vec3& camPos,         // included for typical API symmetry (unused here)
		const glm::vec3& camForward,
		const glm::vec3& camRight,
		const glm::vec3& camUp,
		const glm::vec3& scale = glm::vec3(1.0f),
		float rotation = 0.0f // rotation (in radians) applied around the view direction
	) {
		// For a 2D billboard (sprite), we want its face to be perpendicular to the view direction.
		// So we define the billboard’s forward as opposite to the camera's forward.
		glm::vec3 billboardForward = -camForward;

		// Start with the camera’s right and up vectors, which will form the local X and Y axes.
		glm::vec3 billboardRight = camRight;
		glm::vec3 billboardUp = camUp;

		// Apply an extra rotation about the view (forward) axis.
		if (rotation != 0.0f) {
			billboardRight = glm::rotate(camRight, rotation, camForward);
			billboardUp = glm::rotate(camUp, rotation, camForward);
		}

		// Build the rotation matrix from the three axes.
		// The resulting billboard will have:
		//   - its local X axis pointing along billboardRight,
		//   - its local Y axis pointing along billboardUp,
		//   - and its local Z axis (the "face" normal) along billboardForward.
		glm::mat4 rotationMat(1.0f);
		rotationMat[0] = glm::vec4(billboardRight * scale.x, 0.0f);   // X axis (right)
		rotationMat[1] = glm::vec4(billboardUp * scale.y, 0.0f);        // Y axis (up)
		rotationMat[2] = glm::vec4(billboardForward * scale.z, 0.0f);   // Z axis (facing, note the minus sign)

		// Create a translation matrix from the billboard's position.
		glm::mat4 translationMat = glm::translate(glm::mat4(1.0f), billboardPos);

		// The final model matrix is translation * rotation.
		return translationMat * rotationMat;
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
		if (!glm::detail::decomposeMod(matrix, scale, rotation, translation, skew, perspective))
			return transform; // return default transform on failure

		transform.Position = translation;
		transform.Scale = scale;
		transform.RotationQuaternion = rotation;
		transform.Rotation = ToYawPitchRoll(rotation);
		//transform.Rotation = NormalizeAngles(transform.Rotation);
		return transform;
	}


	inline static uint32_t HashVector(const std::vector<uint8_t>& data)
	{
		uint32_t hash = 2166136261u; // FNV offset basis
		for (uint8_t byte : data)
		{
			hash ^= byte;
			hash *= 16777619u; // FNV prime
		}
		return hash;
	}

	inline static constexpr double constexpr_cos(double x) {
		// Normalize x to [-PI, PI] for better accuracy if needed
		double term = 1.0;
		double res = 1.0;
		for (int i = 1; i <= 10; ++i) {
			term *= -x * x / (2 * i * (2 * i - 1));
			res += term;
		}
		return res;
	}

};