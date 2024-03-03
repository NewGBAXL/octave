#include "BlurPass.h"
#include "Graphics/Vulkan/VulkanUtils.h"
#include "Graphics/Vulkan/VulkanContext.h"

void BlurPass::Create()
{
    mName = "Blur";

    VulkanContext* context = GetVulkanContext();
    uint32_t width = context->GetSceneWidth();
    uint32_t height = context->GetSceneHeight();

    ImageDesc image;
    image.mFormat = context->GetSceneColorFormat();
    image.mWidth = width;
    image.mHeight = height;
    image.mLayers = 1;
    image.mMipLevels = 1;
    image.mUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

    SamplerDesc sampler;
    sampler.mAddressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

    mXBlurImage = new Image(image, sampler, "Half Blur Image");
    mXBlurImage->Transition(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void BlurPass::Destroy()
{
    GetDestroyQueue()->Destroy(mXBlurImage);
}

void BlurPass::Render(Image* input, Image* output)
{
    VulkanContext* context = GetVulkanContext();
    VkCommandBuffer cb = GetCommandBuffer();

    // Populate gaussian weights
    mUniforms.mNumSamples = glm::clamp<int32_t>(mUniforms.mNumSamples, 1, BLUR_MAX_SAMPLES);
    const float E = 2.71828182846f;
    float stdDev = mUniforms.mSigmaRatio * mUniforms.mBlurSize;
    const float stdDev2 = stdDev * stdDev;
    for (int32_t i = 0; i < mUniforms.mNumSamples; ++i)
    {
        float offset = (i / (mUniforms.mNumSamples - 1.0f) - 0.5f) * mUniforms.mBlurSize;
        float gauss = (1.0f / sqrtf(2.0f * PI * stdDev2)) * powf(E, -((offset * offset) / (2 * stdDev2)));
        mUniforms.mGaussianWeights[i].x = gauss;
    }

    // Pass 1 - Horizontal Blur
    {
        mUniforms.mHorizontal = true;

        // Set render target
        RenderPassSetup rpSetup;
        rpSetup.mColorImages[0] = mXBlurImage;
        rpSetup.mLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        rpSetup.mStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
        context->BeginVkRenderPass(rpSetup, true);

        // Bind vertex/index buffers for full screen quad.
        context->BindFullscreenVertexBuffer(cb);

        // Set vertex + fragment shader
        context->SetVertexShader("ScreenRect.vert");
        context->SetFragmentShader("Blur.frag");

        // Commit graphics pipeline
        context->CommitPipeline();

        // Bind descriptor set
        UniformBlock block = WriteUniformBlock(&mUniforms, sizeof(BlurUniforms));

        DescriptorSet::Begin("Blur X DS")
            .WriteUniformBuffer(0, block)
            .WriteImage(1, input)
            .Build()
            .Bind(cb, 1);

        // Draw 
        vkCmdDraw(cb, 4, 1, 0, 0);

        // End render pass
        context->EndVkRenderPass();
    }

    // Pass 2 - Vertical Blur
    {
        mUniforms.mHorizontal = 0;

        // Set render target
        RenderPassSetup rpSetup;
        rpSetup.mColorImages[0] = output;
        rpSetup.mLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        rpSetup.mStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
        context->BeginVkRenderPass(rpSetup, true);

        // Bind vertex/index buffers for full screen quad.
        context->BindFullscreenVertexBuffer(cb);

        // Set vertex + fragment shader
        context->SetVertexShader("ScreenRect.vert");
        context->SetFragmentShader("Blur.frag");

        // Commit graphics pipeline
        context->CommitPipeline();

        // Bind descriptor set
        UniformBlock block = WriteUniformBlock(&mUniforms, sizeof(BlurUniforms));

        DescriptorSet::Begin("Blur Y DS")
            .WriteUniformBuffer(0, block)
            .WriteImage(1, mXBlurImage)
            .Build()
            .Bind(cb, 1);

        // Draw 
        vkCmdDraw(cb, 4, 1, 0, 0);

        // End render pass
        context->EndVkRenderPass();
    }
}

void BlurPass::GatherProperties(std::vector<Property>& props)
{
    PostProcessPass::GatherProperties(props);
    props.push_back(Property(DatumType::Integer, "Blur Samples", nullptr, &mUniforms.mNumSamples));
    props.push_back(Property(DatumType::Float, "Blur Size", nullptr, &mUniforms.mBlurSize));
    props.push_back(Property(DatumType::Float, "Sigma Ratio", nullptr, &mUniforms.mSigmaRatio));
    props.push_back(Property(DatumType::Bool, "Blur Box", nullptr, &mUniforms.mBoxBlur)); // Stored as integer, but it is simply a flag, so not zero = true

}
