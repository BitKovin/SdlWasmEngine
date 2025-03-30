#include "utils.hpp"

namespace utils
{
	bool fileio::read(const std::string& path, std::string& data)
	{
        std::ifstream file(path);
        if (!file.is_open())
            return false;

        std::stringstream buffer;
        buffer << file.rdbuf();
        data = buffer.str();
        file.close();

        return true;
	}

    glm::mat4 assimp::toGlmMat4(aiMatrix4x4 mat) {
        glm::mat4 m;
        for (int y = 0; y < 4; y++)
        {
            for (int x = 0; x < 4; x++)
            {
                m[x][y] = mat[y][x];
            }
        }
        return m;
    }

    glm::vec3 assimp::toGlmVec3(aiVector3D vec) {
        return glm::vec3(vec.x, vec.y, vec.z);
    }

    glm::quat assimp::toGlmQuat(aiQuaternion quat) {
        glm::quat q;
        q.x = quat.x;
        q.y = quat.y;
        q.z = quat.z;
        q.w = quat.w;

        return q;
    }
}