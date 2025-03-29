#pragma once

#include "glm.h"
#include "gl.h"

#include <vector>

#include "EObject.hpp"

// Helper to calculate offsets
#define OFFSET_OF(type, member) ((void*)offsetof(type, member))

class VertexDeclaration {
public:
    struct Element {
        GLuint index;
        GLint componentCount;
        GLenum type;
        GLboolean normalized;
        GLsizei stride;
        const void* offset;
        GLuint divisor; // For instancing
    };

    VertexDeclaration(std::initializer_list<Element> elements)
        : m_elements(elements) {
    }

    const std::vector<Element>& GetElements() const { return m_elements; }

private:
    std::vector<Element> m_elements;
};

class VertexBuffer {
public:
    template<typename T>
    VertexBuffer(const std::vector<T>& vertices, const VertexDeclaration& declaration, GLenum usage = GL_STATIC_DRAW)
        : m_declaration(declaration), m_vertexCount(vertices.size()) {
        glGenBuffers(1, &m_id);
        Bind();
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(T), vertices.data(), usage);
    }

    ~VertexBuffer() { glDeleteBuffers(1, &m_id); }

    void Bind() const { glBindBuffer(GL_ARRAY_BUFFER, m_id); }
    static void Unbind() { glBindBuffer(GL_ARRAY_BUFFER, 0); }

    const VertexDeclaration& GetDeclaration() const { return m_declaration; }
    size_t GetVertexCount() const { return m_vertexCount; }

private:
    GLuint m_id;
    VertexDeclaration m_declaration;
    size_t m_vertexCount;
};

class IndexBuffer {
public:
    IndexBuffer(const std::vector<GLuint>& indices, GLenum usage = GL_STATIC_DRAW)
        : m_indexCount(indices.size()) {
        glGenBuffers(1, &m_id);
        Bind();
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), usage);
    }

    ~IndexBuffer() { glDeleteBuffers(1, &m_id); }

    void Bind() const { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id); }
    static void Unbind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }

    size_t GetIndexCount() const { return m_indexCount; }

private:
    GLuint m_id;
    size_t m_indexCount;
};

class VertexArrayObject {
public:

    int IndexCount = 0;

    VertexArrayObject(const VertexBuffer& vb, const IndexBuffer& ib) {
        glGenVertexArrays(1, &m_id);
        glBindVertexArray(m_id);

        IndexCount = ib.GetIndexCount();

        vb.Bind();
        ib.Bind();


        const auto& elements = vb.GetDeclaration().GetElements();
        for (const auto& element : elements) {
            glEnableVertexAttribArray(element.index);
            glVertexAttribPointer(
                element.index,
                element.componentCount,
                element.type,
                element.normalized,
                element.stride,
                element.offset
            );
            if (element.divisor > 0) {
                glVertexAttribDivisor(element.index, element.divisor);
            }
        }

        glBindVertexArray(0);
        VertexBuffer::Unbind();
        IndexBuffer::Unbind();
    }

    ~VertexArrayObject() { glDeleteVertexArrays(1, &m_id); }

    void Bind() const { glBindVertexArray(m_id); }
    static void Unbind() { glBindVertexArray(0); }

private:
    GLuint m_id;
};

// Example usage with your VertexData structure
struct VertexData {
    glm::vec3 Position = vec3();
    glm::vec3 Normal = vec3();
    glm::vec2 TextureCoordinate = vec2();
    glm::vec3 Tangent = vec3();
    glm::vec3 BiTangent = vec3();
    glm::vec4 BlendIndices = vec4();
    glm::vec4 BlendWeights = vec4();
    glm::vec3 SmoothNormal = vec3();
    glm::vec4 Color = vec4();

    static VertexDeclaration Declaration() {
        return VertexDeclaration({
            {0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), OFFSET_OF(VertexData, Position), 0},
            {1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), OFFSET_OF(VertexData, Normal), 0},
            {2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), OFFSET_OF(VertexData, TextureCoordinate), 0},
            {3, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), OFFSET_OF(VertexData, Tangent), 0},
            {4, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), OFFSET_OF(VertexData, BiTangent), 0},
            {5, 4, GL_FLOAT, GL_FALSE, sizeof(VertexData), OFFSET_OF(VertexData, BlendIndices), 0},
            {6, 4, GL_FLOAT, GL_FALSE, sizeof(VertexData), OFFSET_OF(VertexData, BlendWeights), 0},
            {7, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), OFFSET_OF(VertexData, SmoothNormal), 0},
            {8, 4, GL_FLOAT, GL_FALSE, sizeof(VertexData), OFFSET_OF(VertexData, Color), 0}
            });
    }
};