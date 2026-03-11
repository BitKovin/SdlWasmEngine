#include "ParticleEmitter.h"

#include "../MathHelper.hpp"

#include "../ShaderManager.h"

#include "../FrustrumCull.hpp"
#include "../BoundingSphere.hpp"

#include "../Renderer/Renderer.h"

#include "../BSP/Quake3Bsp.h"



mat4 GetWorldMatrix(const Particle& particle)
{
    return translate(particle.position) * MathHelper::GetRotationMatrix(particle.globalRotation) * scale(vec3(particle.Size));
}

void ParticleEmitter::SpawnParticles(int num)
{
    std::lock_guard<std::recursive_mutex> lock(particlesMutex);
    for (int i = 0; i < num; i++) {
        Particle particle = GetNewParticle();
        particle.deathTime *= i + 1;
        AddParticle(particle);
    }
}

void ParticleEmitter::Update(float deltaTime)
{
    if (destroyed)
        return;

    std::lock_guard<std::recursive_mutex> lock(particlesMutex);

    elapsedTime += deltaTime;
    if (elapsedTime > Duration)
        Emitting = false;

    // Spawn new particles at a fixed spawn rate.
    const float spawnInterval = (SpawnRate > 0.0f) ? (1.0f / SpawnRate) : 0.0f;

    if (SpawnRate > 0.0f && Emitting && spawnInterval > 0.0f)
    {
        while (elapsedTime >= spawnInterval)
        {
            Particles.push_back(GetNewParticle());
            elapsedTime -= spawnInterval;
        }
    }

    // Update lifetime.
    for (auto& p : Particles)
        p.lifeTime += deltaTime;

    // Remove expired particles.
    Particles.erase(
        std::remove_if(Particles.begin(), Particles.end(),
            [](const Particle& p) { return p.lifeTime >= p.deathTime; }),
        Particles.end());

    // Enforce MaxParticles (0 => unlimited). Remove oldest first.
    if (MaxParticles > 0 && Particles.size() > MaxParticles)
    {
        const size_t excess = Particles.size() - MaxParticles;
        // Oldest are at the front because we push_back() new ones.
        Particles.erase(
            Particles.begin(),
            Particles.begin() + static_cast<std::ptrdiff_t>(excess));
    }

    // Update each remaining particle.
    for (auto& p : Particles)
        p = UpdateParticle(p, deltaTime);

    // If we are no longer emitting and there are no particles left, mark as destroyed.
    if (!Emitting && Particles.empty())
        destroyed = true;
}


void ParticleEmitter::DrawForward(mat4x4 view, mat4x4 projection)
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
		forward_shader_program = ShaderManager::GetShaderProgram("instanced_bilboard_vertex", PixelShader);

	forward_shader_program->UseProgram();

    forward_shader_program->SetUniform("view", view);
    forward_shader_program->SetUniform("projection", projection);

    forward_shader_program->SetUniform("is_particle", isDecal == false);
    forward_shader_program->SetUniform("is_decal", isDecal);
    forward_shader_program->SetUniform("isViewmodel", false);

    Renderer::SetSurfaceShaderUniforms(forward_shader_program);

    forward_shader_program->SetTexture("u_texture", savedTexture);

    SetInstanceData(instances);

	bilboardVAO->Bind();
	
    int instanceCount = static_cast<int>(instances.size());
    glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(bilboardVAO->IndexCount), GL_UNSIGNED_INT, 0, instanceCount);

    forward_shader_program->SetUniform("is_particle", false);
    forward_shader_program->SetUniform("is_decal", false);

    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
    */
}

void ParticleEmitter::FinalizeFrameData()
{
    // 1) Snapshot particles quickly under a short lock (one copy) and release the lock.
    {
        std::lock_guard<std::recursive_mutex> lock(particlesMutex);
        finalizedParticles = Particles; // single copy while locked
    }

    // 2) Read camera once
    const vec3 cameraPosition = Camera::finalizedPosition;
    const vec3 cameraRotation = Camera::finalizedRotation;
    const vec3 cameraForward = MathHelper::GetForwardVector(cameraRotation);
    const vec3 cameraRight = MathHelper::GetRightVector(cameraRotation);
    const vec3 cameraUp = MathHelper::GetUpVector(cameraRotation);

    const int cameraC = Level::Current->BspData.FindClusterAtPosition(cameraPosition);

    // 3) Prepare containers
    instances.clear();


    if (DepthSorting)
    {
        std::vector<std::pair<float, InstanceData>> visible;
        visible.reserve(finalizedParticles.size());

        for (const auto& particle : finalizedParticles)
        {
            // Frustum - cheap reject
            if (!Camera::frustum.IsSphereVisible(particle.position, particle.Size))
                continue;

            // PVS / cluster visibility
            int targetC = Level::Current->BspData.FindClusterAtPosition(particle.position);
            if (!Level::Current->BspData.IsClusterVisible(cameraC, targetC))
                continue;

            // Create world/billboard matrix only for visible particles
            mat4x4 world;
            if (particle.UseWorldRotation)
                world = GetWorldMatrix(particle);
            else
                world = MathHelper::CreateBillboardMatrix(
                    particle.position,
                    cameraPosition,
                    cameraForward,
                    cameraRight,
                    cameraUp,
                    vec3(particle.Size),
                    particle.rotation
                );

            InstanceData data;
            data.ModelMatrix = world;
            data.Color = particle.Color * vec4(GetLightForParticle(particle), 1.0f);
            data.Color.a *= particle.Transparency;

            // compute depth only for visible particles
            float depth = glm::dot(cameraForward, particle.position - cameraPosition);
            visible.emplace_back(depth, std::move(data));
        }

        if (!visible.empty())
        {
            // sort visible set by depth (farthest first)
            std::sort(visible.begin(), visible.end(),
                [](const auto& a, const auto& b) { return a.first > b.first; });

            instances.reserve(visible.size());
            for (auto& v : visible)
                instances.push_back(std::move(v.second));
        }
    }
    else
    {
        // No depth sorting: build instances in one pass (no sorting overhead)
        instances.reserve(finalizedParticles.size());
        for (const auto& particle : finalizedParticles)
        {
            if (!Camera::frustum.IsSphereVisible(particle.position, particle.Size))
                continue;

            int targetC = Level::Current->BspData.FindClusterAtPosition(particle.position);
            if (!Level::Current->BspData.IsClusterVisible(cameraC, targetC))
                continue;

            mat4x4 world;
            if (particle.UseWorldRotation)
                world = GetWorldMatrix(particle);
            else
                world = MathHelper::CreateBillboardMatrix(
                    particle.position,
                    cameraPosition,
                    cameraForward,
                    cameraRight,
                    cameraUp,
                    vec3(particle.Size),
                    particle.rotation
                );

            InstanceData data;
            data.ModelMatrix = world;
            data.Color = particle.Color * vec4(GetLightForParticle(particle), 1.0f);
            data.Color.a *= particle.Transparency;
            instances.push_back(std::move(data));
        }
    }
}


void ParticleEmitter::InitBilboardVaoIfNeeded()
{
    /*
    if (bilboardVAO) return;

    // Define billboard vertices (a quad centered at the origin with size 1x1 meter)
// Using VertexData to leverage your declarations. Other fields are set to default.
    std::vector<VertexData> vertices(4);

    // Bottom-left
    vertices[0].Position = glm::vec3(-0.5f, -0.5f, 0.0f);
    vertices[0].TextureCoordinate = glm::vec2(0.0f, 1.0f);

    // Bottom-right
    vertices[1].Position = glm::vec3(0.5f, -0.5f, 0.0f);
    vertices[1].TextureCoordinate = glm::vec2(1.0f, 1.0f);

    // Top-right
    vertices[2].Position = glm::vec3(0.5f, 0.5f, 0.0f);
    vertices[2].TextureCoordinate = glm::vec2(1.0f, 0.0f);

    // Top-left
    vertices[3].Position = glm::vec3(-0.5f, 0.5f, 0.0f);
    vertices[3].TextureCoordinate = glm::vec2(0.0f, 0.0f);

    // Define indices for two triangles (quad)
    std::vector<GLuint> indices = {
        0, 1, 2,  // First triangle
        2, 3, 0   // Second triangle
    };

    std::vector<InstanceData> instanceData{ }; //empty by default

    // Create the vertex buffer for the billboard vertices
    VertexBuffer* vb = new VertexBuffer(vertices, VertexData::Declaration(), GL_STATIC_DRAW);

    // Create the index buffer for the quad
    IndexBuffer* ib = new IndexBuffer(indices, GL_STATIC_DRAW);

    // Create the instance buffer from the provided instance data
    VertexBuffer* instanceBuffer = nullptr;
    
    instanceBuffer = new VertexBuffer(instanceData, InstanceData::Declaration(), GL_STATIC_DRAW);
    

    // Create the VAO using the vertex buffer, index buffer, and instance buffer (if any)
    bilboardVAO = new VertexArrayObject(*vb, *ib, instanceBuffer);

    VertexArrayObject::Unbind();
    IndexBuffer::Unbind();
    VertexBuffer::Unbind();
    */
}

vec3 ParticleEmitter::GetLightForParticle(const Particle& particle)
{
    if (particle.UseWorldRotation == false)
    {
        auto light = Level::Current->BspData.GetLightvolColorPoint(particle.position * MAP_SCALE);
        return (light.ambientColor + light.directColor) * 2.0f;
    }

    vec3 normal = MathHelper::GetForwardVector(particle.globalRotation);

    auto light = Level::Current->BspData.GetLightvolColorPoint((particle.position + normal) * MAP_SCALE) * 4.0f;

    float dirFactor = 1.0;// glm::clamp(dot(normal, light.direction), 0.0f, 1.0f);

    return light.ambientColor + light.directColor * dirFactor;
}

void ParticleEmitter::SetInstanceData(std::vector<InstanceData>& instanceData)
{
    /*
    if (bilboardVAO == nullptr)
    {
        Logger::Log("null bilboard vao");
        return;
    }

    if (bilboardVAO->instanceBuffer == nullptr)
    {
        Logger::Log("null bilboard vao instanceBuffer");
        return;
    }

    bilboardVAO->instanceBuffer->UpdateData(instanceData);
    */

}
