#include "RibbonEmitter.h"
#include "../MathHelper.hpp"

#include "../Renderer/Renderer.h"

RibbonEmitter::RibbonEmitter()

{
    DepthSorting = false;
}

RibbonEmitter::~RibbonEmitter()
{

}

void RibbonEmitter::RenderRibbon(const std::vector<Particle>& inParticles)
{
    primitiveCount = 0;

    if (inParticles.size() < 1 || destroyed)
        return;
    /*
    VertexArrayObject::Unbind();

    // Initialize with empty buffers since data is dynamic
    std::vector<VertexData> emptyVerts;
    std::vector<GLuint> emptyIdxs;

    if (vb == nullptr)
    {
        // Create VBO with GL_STREAM_DRAW for dynamic updates
        vb = new VertexBuffer(emptyVerts, VertexData::Declaration(), GL_STREAM_DRAW);
    }
    if (ib == nullptr)
    {
        // Create IBO with GL_STREAM_DRAW for dynamic updates
        ib = new IndexBuffer(emptyIdxs, GL_STREAM_DRAW);
    }

    if (vao == nullptr)
    {
        // Create VAO to manage VBO and IBO state
        vao = new VertexArrayObject(*vb, *ib);
    }



    // 1. Copy and possibly append particles
    std::vector<Particle> particles = inParticles;
    if (true) {
        Particle np = GetNewParticle();
        np.position = Position + MathHelper::GetForwardVector(particles.back().globalRotation) * 0.001f;
        particles.push_back(np);

    }

    if (SimpleRibbon) 
    {
        std::vector<Particle> cParticles = particles;
        particles.clear();
        particles.push_back(cParticles[0]);
        particles.push_back(cParticles[cParticles.size() - 1]);
    }
    int n = static_cast<int>(particles.size());
    int vCount = n * 2;          // Two vertices per particle (ribbon width)
    int idxCount = (n - 1) * 6;  // Six indices per segment (two triangles)

    // 2. Resize and fill CPU-side arrays
    verts.resize(vCount);
    idxs.resize(idxCount);
    GenerateIndices(idxs, n);

    glm::vec3 camPos = Camera::position;
    for (int i = 0; i < n; ++i) {
        const Particle& p = particles[i];
        glm::vec3 P = p.position;

        // Calculate direction and perpendicular vector for ribbon orientation
        glm::vec3 dir = (i < n - 1)
            ? glm::normalize(P - particles[i + 1].position)
            : glm::normalize(particles[i - 1].position - P);
        glm::vec3 camFwd = glm::normalize(P - camPos);
        glm::vec3 perp = glm::normalize(glm::cross(dir, camFwd));

        float half = p.Size * 0.5f;
        int b = i * 2;

        vec3 light = GetLightForParticle(p);

        // Vertex on one side of the ribbon
        verts[b + 0].Position = P + perp * half;
		verts[b + 0].TextureCoordinate = glm::vec2(static_cast<float>(i) / (n - 1), 0.0f);

		verts[b + 0].Color = p.Color * vec4(1, 1, 1, p.Transparency) * vec4(light,1.0);

        // Vertex on the other side
        verts[b + 1].Position = P - perp * half;
        verts[b + 1].TextureCoordinate = glm::vec2(static_cast<float>(i) / (n - 1), 1.0f) ;
        verts[b + 1].Color = p.Color * vec4(1, 1, 1, p.Transparency) * vec4(light, 1.0);
    }

    primitiveCount = idxCount / 3;

    // 3. Update VBO and IBO with new data
    vb->UpdateData(verts);  // Assuming VertexBuffer has an UpdateData method
    ib->UpdateData(idxs);   // Assuming IndexBuffer has an UpdateData method (see note below)

    // 4. Render using VAO
    vao->Bind();
    glDrawElements(GL_TRIANGLES, idxs.size(), GL_UNSIGNED_INT, 0);
    vao->Unbind();
    vb->Unbind();
    ib->Unbind();
    VertexArrayObject::Unbind();
    */
}

void RibbonEmitter::FinalizeFrameData()
{
    std::lock_guard<std::recursive_mutex> lock(particlesMutex);
    finalizedParticles = Particles;
}

void RibbonEmitter::DrawForward(mat4x4 view, mat4x4 projection)
{
    if (savedTextureName != texture)
    {
        savedTexture = AssetRegistry::GetTextureFromFile(texture);
        savedTextureName = texture;
    }
    /*
    glDepthMask(GL_FALSE);

    glDisable(GL_CULL_FACE);

    ShaderProgram* forward_shader_program = nullptr;

    if (forward_shader_program == nullptr)
        forward_shader_program = ShaderManager::GetShaderProgram("default_vertex", PixelShader);

    forward_shader_program->UseProgram();

    forward_shader_program->SetUniform("view", view);
    forward_shader_program->SetUniform("projection", projection);

    forward_shader_program->SetUniform("is_particle", true);
    forward_shader_program->SetUniform("is_decal", false);
    forward_shader_program->SetUniform("world", glm::identity<mat4>());
    forward_shader_program->SetUniform("isViewmodel", false);

    Renderer::SetSurfaceShaderUniforms(forward_shader_program);

    forward_shader_program->SetTexture("u_texture", savedTexture);

    RenderRibbon(finalizedParticles);

    forward_shader_program->SetUniform("is_particle", false);
    forward_shader_program->SetUniform("is_decal", false);

    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
    */
}


void RibbonEmitter::GenerateIndices(std::vector<int>& dst, int n) {

}
