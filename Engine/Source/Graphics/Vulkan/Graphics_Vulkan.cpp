#if API_VULKAN

#include "Graphics/Graphics.h"
#include "Graphics/Vulkan/VulkanContext.h"
#include "Graphics/Vulkan/VulkanUtils.h"

#include "Maths.h"

extern VulkanContext* gVulkanContext;

void GFX_Initialize()
{
    // On Android, it's possible that GFX_Initialize() was already called.
    if (gVulkanContext == nullptr)
    {
        CreateVulkanContext();
        gVulkanContext->Initialize();
    }
}

void GFX_Shutdown()
{
    gVulkanContext->Destroy();
    DestroyVulkanContext();
}

void GFX_BeginFrame()
{
    gVulkanContext->BeginFrame();
}

void GFX_EndFrame()
{
    gVulkanContext->EndFrame();
}

void GFX_BeginScreen(uint32_t screenIndex)
{

}

void GFX_BeginView(uint32_t viewIndex)
{

}

bool GFX_ShouldCullLights()
{
    return true;
}

void GFX_BeginRenderPass(RenderPassId renderPassId)
{
    gVulkanContext->BeginRenderPass(renderPassId);
}

void GFX_EndRenderPass()
{
    gVulkanContext->EndRenderPass();
}

void GFX_BindPipeline(PipelineId pipelineId, VertexType vertexType)
{
    gVulkanContext->BindPipeline(pipelineId, vertexType);
}

void GFX_SetViewport(int32_t x, int32_t y, int32_t width, int32_t height)
{
    gVulkanContext->SetViewport(x, y, width, height);
}

void GFX_SetScissor(int32_t x, int32_t y, int32_t width, int32_t height)
{
    gVulkanContext->SetScissor(x, y, width, height);
}

glm::mat4 GFX_MakePerspectiveMatrix(float fovyDegrees, float aspectRatio, float zNear, float zFar)
{
    VkSurfaceTransformFlagBitsKHR preTransformFlag = gVulkanContext->GetPreTransformFlag();

    glm::mat4 preRotateMat = glm::mat4(1.0f);
    glm::vec3 rotationAxis = glm::vec3(0.0f, 0.0f, 1.0f);

    if (preTransformFlag & VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR) {
        preRotateMat = glm::rotate(preRotateMat, glm::radians(90.0f), rotationAxis);
    }

    else if (preTransformFlag & VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR) {
        preRotateMat = glm::rotate(preRotateMat, glm::radians(270.0f), rotationAxis);
    }

    else if (preTransformFlag & VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR) {
        preRotateMat = glm::rotate(preRotateMat, glm::radians(180.0f), rotationAxis);
    }

    glm::mat4 perspMat = glm::perspectiveFov(glm::radians(fovyDegrees), aspectRatio, 1.0f, zNear, zFar);
    perspMat = preRotateMat * perspMat;

    return perspMat;
}

glm::mat4 GFX_MakeOrthographicMatrix(float left, float right, float bottom, float top, float zNear, float zFar)
{
    VkSurfaceTransformFlagBitsKHR preTransformFlag = gVulkanContext->GetPreTransformFlag();

    glm::mat4 preRotateMat = glm::mat4(1.0f);
    glm::vec3 rotationAxis = glm::vec3(0.0f, 0.0f, 1.0f);

    if (preTransformFlag & VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR) {
        preRotateMat = glm::rotate(preRotateMat, glm::radians(90.0f), rotationAxis);
    }

    else if (preTransformFlag & VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR) {
        preRotateMat = glm::rotate(preRotateMat, glm::radians(270.0f), rotationAxis);
    }

    else if (preTransformFlag & VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR) {
        preRotateMat = glm::rotate(preRotateMat, glm::radians(180.0f), rotationAxis);
    }

    glm::mat4 orthoMat = glm::ortho(left, right, bottom, top, zNear, zFar);
    orthoMat = preRotateMat * orthoMat;

    return orthoMat;
}

void GFX_SetFog(const FogSettings& fogSettings)
{

}

void GFX_Draw(void* data)
{
    //gVulkanContext->Draw();
}

void GFX_DrawLines(const std::vector<Line>& lines)
{
    gVulkanContext->DrawLines(lines);
}

void GFX_DrawFullscreen()
{
    gVulkanContext->DrawFullscreen();
}

void GFX_ResizeWindow()
{
    if (gVulkanContext != nullptr)
    {
        gVulkanContext->RecreateSwapchain();
    }
}

TransformComponent* GFX_ProcessHitCheck(World* world, int32_t x, int32_t y)
{
#if EDITOR
    return gVulkanContext->ProcessHitCheck(world, x, y);
#else
    return nullptr;
#endif
}

uint32_t GFX_GetNumViews()
{
    return 1;
}

void GFX_SetFrameRate(int32_t frameRate)
{

}

void GFX_PathTrace()
{
    if (gVulkanContext->IsRayTracingSupported())
    {
        gVulkanContext->GetRayTracer()->PathTraceWorld();
    }
}

void GFX_BeginLightBake()
{
    if (gVulkanContext->IsRayTracingSupported())
    {
        gVulkanContext->GetRayTracer()->BeginLightBake();
    }
    else
    {
        LogError("GPU cannot bake light because maxPerStageDescriptorSampledImages < %d", PATH_TRACE_MAX_TEXTURES);
    }
}

void GFX_UpdateLightBake()
{
    if (gVulkanContext->IsRayTracingSupported())
    {
        gVulkanContext->GetRayTracer()->UpdateLightBake();
    }
}

void GFX_EndLightBake()
{
    if (gVulkanContext->IsRayTracingSupported())
    {
        gVulkanContext->GetRayTracer()->EndLightBake();
    }
}

bool GFX_IsLightBakeInProgress()
{
    bool ret = false;

    if (gVulkanContext->IsRayTracingSupported())
    {
        ret = gVulkanContext->GetRayTracer()->IsLightBakeInProgress();
    }

    return ret;
}

float GFX_GetLightBakeProgress()
{
    float ret = 0.0f;

    if (gVulkanContext->IsRayTracingSupported())
    {
        ret = gVulkanContext->GetRayTracer()->GetLightBakeProgress();
    }
    
    return ret;
}

void GFX_CreateTextureResource(Texture* texture, std::vector<uint8_t>& data)
{
    CreateTextureResource(texture, data.data());
}

void GFX_DestroyTextureResource(Texture* texture)
{
    DestroyTextureResource(texture);
}

void GFX_CreateMaterialResource(Material* material)
{
    CreateMaterialResource(material);
}

void GFX_DestroyMaterialResource(Material* material)
{
    DestroyMaterialResource(material);
}

void GFX_CreateStaticMeshResource(StaticMesh* staticMesh, bool hasColor, uint32_t numVertices, void* vertices, uint32_t numIndices, IndexType* indices)
{
    CreateStaticMeshResource(staticMesh, hasColor, numVertices, vertices, numIndices, indices);
}

void GFX_DestroyStaticMeshResource(StaticMesh* staticMesh)
{
    DestroyStaticMeshResource(staticMesh);
}

void GFX_CreateSkeletalMeshResource(SkeletalMesh* skeletalMesh, uint32_t numVertices, VertexSkinned* vertices, uint32_t numIndices, uint32_t* indices)
{
    CreateSkeletalMeshResource(skeletalMesh, numVertices, vertices, numIndices, indices);
}

void GFX_DestroySkeletalMeshResource(SkeletalMesh* skeletalMesh)
{
    DestroySkeletalMeshResource(skeletalMesh);
}

void GFX_CreateStaticMeshCompResource(StaticMeshComponent* staticMeshComp)
{
    CreateStaticMeshCompResource(staticMeshComp);
}

void GFX_DestroyStaticMeshCompResource(StaticMeshComponent* staticMeshComp)
{
    DestroyStaticMeshCompResource(staticMeshComp);
}

void GFX_UpdateStaticMeshCompResourceColors(StaticMeshComponent* staticMeshComp)
{
    UpdateStaticMeshCompResourceColors(staticMeshComp);
}

void GFX_DrawStaticMeshComp(StaticMeshComponent* staticMeshComp, StaticMesh* meshOverride)
{
    DrawStaticMeshComp(staticMeshComp, meshOverride);
}

void GFX_CreateSkeletalMeshCompResource(SkeletalMeshComponent* skeletalMeshComp)
{
    CreateSkeletalMeshCompResource(skeletalMeshComp);
}

void GFX_DestroySkeletalMeshCompResource(SkeletalMeshComponent* skeletalMeshComp)
{
    DestroySkeletalMeshCompResource(skeletalMeshComp);
}

void GFX_ReallocateSkeletalMeshCompVertexBuffer(SkeletalMeshComponent* skeletalMeshComp, uint32_t numVertices)
{
    ReallocateSkeletalMeshCompVertexBuffer(skeletalMeshComp, numVertices);
}

void GFX_UpdateSkeletalMeshCompVertexBuffer(SkeletalMeshComponent* skeletalMeshComp, const std::vector<Vertex>& skinnedVertices)
{
    UpdateSkeletalMeshCompVertexBuffer(skeletalMeshComp, skinnedVertices);
}

void GFX_DrawSkeletalMeshComp(SkeletalMeshComponent* skeletalMeshComp)
{
    DrawSkeletalMeshComp(skeletalMeshComp);
}

bool GFX_IsCpuSkinningRequired(SkeletalMeshComponent* skeletalMeshComp)
{
    return IsCpuSkinningRequired(skeletalMeshComp);
}

void GFX_DrawShadowMeshComp(ShadowMeshComponent* shadowMeshComp)
{
    DrawShadowMeshComp(shadowMeshComp);
}

void GFX_CreateTextMeshCompResource(TextMeshComponent* textMeshComp)
{
    CreateTextMeshCompResource(textMeshComp);
}

void GFX_DestroyTextMeshCompResource(TextMeshComponent* textMeshComp)
{
    DestroyTextMeshCompResource(textMeshComp);
}

void GFX_UpdateTextMeshCompVertexBuffer(TextMeshComponent* textMeshComp, const std::vector<Vertex>& vertices)
{
    UpdateTextMeshCompVertexBuffer(textMeshComp, vertices);
}

void GFX_DrawTextMeshComp(TextMeshComponent* textMeshComp)
{
    DrawTextMeshComp(textMeshComp);
}

void GFX_CreateParticleCompResource(ParticleComponent* particleComp)
{
    CreateParticleCompResource(particleComp);
}

void GFX_DestroyParticleCompResource(ParticleComponent* particleComp)
{
    DestroyParticleCompResource(particleComp);
}

void GFX_UpdateParticleCompVertexBuffer(ParticleComponent* particleComp, const std::vector<VertexParticle>& vertices)
{
    UpdateParticleCompVertexBuffer(particleComp, vertices);
}

void GFX_DrawParticleComp(ParticleComponent* particleComp)
{
    DrawParticleComp(particleComp);
}

void GFX_CreateQuadResource(Quad* quad)
{
    CreateQuadResource(quad);
}

void GFX_DestroyQuadResource(Quad* quad)
{
    DestroyQuadResource(quad);
}

void GFX_UpdateQuadResource(Quad* quad)
{
    UpdateQuadResource(quad);
}

void GFX_DrawQuad(Quad* quad)
{
    DrawQuad(quad);
}

void GFX_CreateTextResource(Text* text)
{
    CreateTextResource(text);
}

void GFX_DestroyTextResource(Text* text)
{
    DestroyTextResource(text);
}

void GFX_UpdateTextResourceUniformData(Text* text)
{
    UpdateTextResourceUniformData(text);
}

void GFX_UpdateTextResourceVertexData(Text* text)
{
    UpdateTextResourceVertexData(text);
}

void GFX_DrawText(Text* text)
{
    DrawTextWidget(text);
}

void GFX_CreatePolyResource(Poly* poly)
{
    CreatePolyResource(poly);
}

void GFX_DestroyPolyResource(Poly* poly)
{
    DestroyPolyResource(poly);
}

void GFX_UpdatePolyResourceUniformData(Poly* poly)
{
    UpdatePolyResourceUniformData(poly);
}

void GFX_UpdatePolyResourceVertexData(Poly* poly)
{
    UpdatePolyResourceVertexData(poly);
}

void GFX_DrawPoly(Poly* poly)
{
    DrawPoly(poly);
}

void GFX_DrawStaticMesh(StaticMesh* mesh, Material* material, const glm::mat4& transform, glm::vec4 color)
{
    DrawStaticMesh(mesh, material, transform, color);
}

#endif