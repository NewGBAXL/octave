#pragma once

#include <glm/glm.hpp>

struct GlobalUniformData
{
    glm::mat4 mViewProjMatrix;
    glm::mat4 mDirectionalLightVP;
    glm::vec4 mDirectionalLightDirection;
    glm::vec4 mDirectionalLightColor;
    glm::vec4 mAmbientLightColor;
    glm::vec4 mViewPosition;
    glm::vec4 mViewDirection;
    glm::vec2 mScreenDimensions;
    glm::vec2 mInterfaceResolution;
    glm::vec4 mPointLightPositions[MAX_POINTLIGHTS];
    glm::vec4 mPointLightColors[MAX_POINTLIGHTS];
    glm::vec4 mShadowColor;

    int32_t mNumPointLights;
    int32_t mVisualizationMode;
    int32_t mPadding0;
    int32_t mPadding1;

    glm::vec4 mFogColor;
    int mFogEnabled;
    int mFogDensityFunc;
    float mFogNear;
    float mFogFar;
};

struct GeometryData
{
    glm::mat4 mWVPMatrix;
    glm::mat4 mWorldMatrix;
    glm::mat4 mNormalMatrix;
    glm::mat4 mLightWVPMatrix;
    glm::vec4 mColor;

    uint32_t mHitCheckId;
    uint32_t mPadding0;
    uint32_t mPadding1;
    uint32_t mPadding2;
};

struct SkinnedGeometryData
{
    GeometryData mBase;

    glm::mat4 mBoneMatrices[MAX_BONES];

    uint32_t mNumBoneInfluences;
    uint32_t mPadding0;
    uint32_t mPadding1;
    uint32_t mPadding2;
};

struct QuadUniformData
{
    glm::mat4 mTransform;
    glm::vec4 mColor;
    glm::vec4 mTint;
};

struct TextUniformData
{
    glm::mat4 mTransform;
    glm::vec4 mColor;

    float mX;
    float mY;
    float mCutoff;
    float mOutlineSize;

    float mScale;
    float mSoftness;
    float mPadding1;
    float mPadding2;

    int32_t mDistanceField;
    int32_t mEffect;
};

struct PolyUniformData
{
    glm::mat4 mTransform;
    glm::vec4 mColor;

    float mX;
    float mY;
    float mPad0;
    float mPad1;
};

struct MaterialData
{
    glm::vec2 mUvOffset0;
    glm::vec2 mUvScale0;

    glm::vec2 mUvOffset1;
    glm::vec2 mUvScale1;

    glm::vec4 mColor;
    glm::vec4 mFresnelColor;

    uint32_t mShadingModel;
    uint32_t mBlendMode;
    uint32_t mToonSteps;
    float mFresnelPower;

    float mSpecular;
    float mOpacity;
    float mMaskCutoff;
    float mShininess;

    uint32_t mFresnelEnabled;
    uint32_t mVertexColorMode;
    uint32_t mApplyFog;
    float mPadding0;

    uint32_t mUvMaps[MATERIAL_MAX_TEXTURES];
    uint32_t mTevModes[MATERIAL_MAX_TEXTURES];
};

struct PathTraceVertex
{
    glm::vec3 mPosition;
    float mPad0;

    glm::vec2 mTexcoord0;
    glm::vec2 mTexcoord1;

    glm::vec3 mNormal;
    uint32_t mColor;
};

struct PathTraceTriangle
{
    PathTraceVertex mVertices[3];
};

struct PathTraceMesh
{
    glm::vec4 mBounds;

    uint32_t mStartTriangleIndex;
    uint32_t mNumTriangles;
    uint32_t mPad0;
    uint32_t mPad1;

    MaterialData mMaterial;
};

struct PathTraceLight
{
    glm::vec3 mPosition;
    float mRadius;

    glm::vec4 mColor;

    uint32_t mLightType;
    glm::vec3 mDirection;
};

enum class PathTraceLightType
{
    Point,
    Directional,

    Count
};

enum class DescriptorSetBinding
{
    Global = 0,

    Geometry = 1,
    PostProcess = 1,
    Quad = 1,
    Text = 1,
    Poly = 1,

    Material = 2
};
