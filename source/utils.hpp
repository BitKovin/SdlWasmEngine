#ifndef UTILS_HPP
#define UTILS_HPP
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#include "glm.h"
#include "gl.h"

#include <assimp/scene.h>

namespace utils
{
	namespace fileio
	{
		bool read(const std::string& path, std::string& data);
	}

	namespace assimp
	{
		glm::mat4 toGlmMat4(aiMatrix4x4 mat);
		glm::vec3 toGlmVec3(aiVector3D vec);
		glm::quat toGlmQuat(aiQuaternion quat);
	}
}

#endif //-UTILS_HPP
