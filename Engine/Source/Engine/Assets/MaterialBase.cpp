#include "Assets/MaterialBase.h"
#include "Assets/MaterialInstance.h"
#include "Assets/Texture.h"
#include "Renderer.h"
#include "AssetManager.h"
#include "Constants.h"
#include "Log.h"
#include "Engine.h"
#include "Utilities.h"

#include "Nodes/3D/DirectionalLight3d.h"
#include "Nodes/3D/PointLight3d.h"
#include "Nodes/3D/Camera3d.h"

#include "Graphics/Graphics.h"



#include <sstream>

const char* gBlendModeStrings[] =
{
    "Opaque",
    "Masked",
    "Transparent",
    "Additive"
};
static_assert(int32_t(BlendMode::Count) == 4, "Need to update string conversion table");

const char* gCullModeStrings[] =
{
    "None",
    "Back",
    "Front"
};
static_assert(int32_t(CullMode::Count) == 3, "Need to update string conversion table");

FORCE_LINK_DEF(MaterialBase);
DEFINE_ASSET(MaterialBase);

bool MaterialBase::HandlePropChange(Datum* datum, uint32_t index, const void* newValue)
{
    Property* prop = static_cast<Property*>(datum);

    OCT_ASSERT(prop != nullptr);
    MaterialBase* materialBase = static_cast<MaterialBase*>(prop->mOwner);
    bool success = false;

    materialBase->MarkDirty();
    materialBase->MarkStale();

    return success;
}

MaterialBase::MaterialBase()
{
    mType = MaterialBase::GetStaticType();

    MarkDirty();
}

MaterialBase::~MaterialBase()
{

}

void MaterialBase::LoadStream(Stream& stream, Platform platform)
{
    Material::LoadStream(stream, platform);

    stream.ReadString(mShader);
    mBlendMode = (BlendMode)stream.ReadUint32();
    mMaskCutoff = stream.ReadFloat();
    mSortPriority = stream.ReadInt32();
    mDisableDepthTest = stream.ReadBool();
    mApplyFog = stream.ReadBool();
    mCullMode = (CullMode)stream.ReadUint8();

    for (uint32_t i = 0; i < uint32_t(VertexType::Max); ++i)
    {
        uint32_t vertSize = stream.ReadUint32();

        if (vertSize > 0)
        {
            mVertexCode[i].resize(vertSize);
            stream.ReadBytes(mVertexCode[i].data(), vertSize);
        }
    }

    uint32_t fragSize = stream.ReadUint32();
    if (fragSize > 0)
    {
        mFragmentCode.resize(fragSize);
        stream.ReadBytes(mFragmentCode.data(), fragSize);
    }
}

void MaterialBase::SaveStream(Stream& stream, Platform platform)
{
    Material::SaveStream(stream, platform);

    stream.WriteString(mShader);
    stream.WriteUint32((uint32_t)mBlendMode);
    stream.WriteFloat(mMaskCutoff);
    stream.WriteInt32(mSortPriority);
    stream.WriteBool(mDisableDepthTest);
    stream.WriteBool(mApplyFog);
    stream.WriteUint8((uint8_t)mCullMode);

    for (uint32_t i = 0; i < uint32_t(VertexType::Max); ++i)
    {
        uint32_t vertSize = (uint32_t)mVertexCode[i].size();
        stream.WriteUint32(vertSize);
        if (mVertexCode[i].size() > 0)
        {
            stream.WriteBytes(mVertexCode[i].data(), vertSize);
        }
    }

    uint32_t fragSize = mFragmentCode.size();
    stream.WriteUint32(fragSize);
    if (fragSize > 0)
    {
        stream.WriteBytes(mFragmentCode.data(), fragSize);
    }
}

void MaterialBase::Create()
{
    Material::Create();

    Renderer* renderer = Renderer::Get();

    // Do we want to default all textures to White? Or just leave null.
    // If we do, this should probably be movedup to Material::Create().
#if 0
    for (uint32_t i = 0; i < mParameters.size(); ++i)
    {
        if (mParameters[i].mType == ShaderParameterType::Texture;
            mParameters[i].mTextureValue == nullptr)
        {
            mParameters[i].mTextureValue = renderer->mWhiteTexture;
        }
    }
#endif

    GFX_CreateMaterialResource(this);
    MarkDirty();
}

void MaterialBase::Destroy()
{
    Material::Destroy();

    GFX_DestroyMaterialResource(this);
}

void MaterialBase::Import(const std::string& path, ImportOptions* options)
{
    Material::Import(path, options);
}

void MaterialBase::GatherProperties(std::vector<Property>& outProps)
{
    Material::GatherProperties(outProps);

    outProps.push_back(Property(DatumType::String, "Shader", this, &mShader, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Integer, "Blend Mode", this, &mBlendMode, 1, HandlePropChange, 0, int32_t(BlendMode::Count), gBlendModeStrings));
    outProps.push_back(Property(DatumType::Byte, "Cull Mode", this, &mCullMode, 1, HandlePropChange, 0, int32_t(CullMode::Count), gCullModeStrings));
    outProps.push_back(Property(DatumType::Float, "Mask Cutoff", this, &mMaskCutoff, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Integer, "Sort Priority", this, &mSortPriority, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Bool, "Disable Depth Test", this, &mDisableDepthTest, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Bool, "Apply Fog", this, &mApplyFog, 1, HandlePropChange));
}

glm::vec4 MaterialBase::GetTypeColor()
{
    return glm::vec4(0.2f, 1.0f, 0.25f, 1.0f);
}

const char* MaterialBase::GetTypeName()
{
    return "MaterialBase";
}

bool MaterialBase::IsBase() const
{
    return true;
}

void MaterialBase::Compile()
{
#if EDITOR
    if (mShader == "")
        return;

    // Compile MaterialBase:
    // (X) Find engine/user shader directories
    std::string engineShaderDir = "Engine/Shaders/GLSL/";
    std::string projectShaderDir = GetEngineState()->mProjectDirectory + "Shaders/";

    // (X) Get user shader code from .glsl file.
    std::string rawUserCode;

    {
        // Check for engine shader first
        std::string shaderPath = engineShaderDir + "mat/" + mShader + ".glsl";
        if (SYS_DoesFileExist(shaderPath.c_str(), false))
        {
            Stream shaderStream;
            shaderStream.ReadFile(shaderPath.c_str(), false);
            shaderStream.GetData();

            rawUserCode.assign(shaderStream.GetData(), shaderStream.GetSize());
        }
        else
        {
            shaderPath = projectShaderDir + mShader + ".glsl";
            if (SYS_DoesFileExist(shaderPath.c_str(), false))
            {
                Stream shaderStream;
                shaderStream.ReadFile(shaderPath.c_str(), false);
                shaderStream.GetData();

                rawUserCode.assign(shaderStream.GetData(), shaderStream.GetSize());
            }
        }
    }

    if (rawUserCode == "")
        return;

    // (X) Get template vert + frag code
    std::string templateVertPath = engineShaderDir + "src/MatTemplateVert.glsl";
    std::string templateFragPath = engineShaderDir + "src/MatTemplateFrag.glsl";

    //std::string headerCode;
    std::string fragCode;
    std::string vertCode;

    {
        Stream stream;
        stream.ReadFile(templateVertPath.c_str(), false);
        vertCode.assign(stream.GetData(), stream.GetSize());
    }

    {
        Stream stream;
        stream.ReadFile(templateFragPath.c_str(), false);
        fragCode.assign(stream.GetData(), stream.GetSize());
    }

    OCT_ASSERT(vertCode.size() > 0);
    OCT_ASSERT(fragCode.size() > 0);

    // (X) Extract shader parameters from user code
    std::vector<ShaderParameter> userParams;
    std::string userCode;
    {
        std::istringstream iss(rawUserCode);

        for (std::string line; std::getline(iss, line);)
        {
            size_t scalarIdx = line.find("MAT_SCALAR(");
            size_t vectorIdx = line.find("MAT_VECTOR(");
            size_t textureIdx = line.find("MAT_TEXTURE(");

            ShaderParameterType paramType = ShaderParameterType::Count;

            if (scalarIdx != std::string::npos)
            {
                paramType = ShaderParameterType::Scalar;
                line = line.substr(strlen("MAT_SCALAR("));
            }
            else if (vectorIdx != std::string::npos)
            {
                paramType = ShaderParameterType::Vector;
                line = line.substr(strlen("MAT_VECTOR("));
            }
            else if (textureIdx != std::string::npos)
            {
                paramType = ShaderParameterType::Texture;
                line = line.substr(strlen("MAT_TEXTURE("));
            }

            if (paramType != ShaderParameterType::Count)
            {
                size_t endParen = line.find(")");
                if (endParen != std::string::npos)
                {
                    std::string paramName = line.substr(0, endParen - 1);

                    // Remove spaces
                    RemoveSpacesFromString(paramName);

                    ShaderParameter param;
                    param.mName = paramName;
                    param.mType = paramType;
                    userParams.push_back(param);
                }
            }
            else
            {
                // Filter out the parameters from the raw code.
                userCode += line;
            }
        }
    }

    // (X) Generate shader parameter string while determining param Offset values
    uint32_t numScalarParams = 0;
    uint32_t numVectorParams = 0;
    uint32_t numTextureParams = 0;
    uint32_t uniformByteOffset = 0;

    std::string userParamsString = "layout(set = 2, binding = 0) uniform MaterialUniformBuffer\n{\n";

    // Add vector parameters first (for alignment reasons).
    for (uint32_t i = 0; i < userParams.size(); ++i)
    {
        ShaderParameter& param = userParams[i];
        if (param.mType == ShaderParameterType::Vector)
        {
            param.mOffset = uniformByteOffset;
            uniformByteOffset += 4;
            userParamsString += ("    vec4 " + param.mName + ";\n");
            numVectorParams++;
        }
    }

    // Then add scalars
    for (uint32_t i = 0; i < userParams.size(); ++i)
    {
        ShaderParameter& param = userParams[i];
        if (param.mType == ShaderParameterType::Scalar)
        {
            param.mOffset = uniformByteOffset;
            uniformByteOffset += 1;
            userParamsString += ("    float " + param.mName + ";\n");
            numScalarParams++;
        }
    }

    userParamsString += "}\n\n";

    // Lastly, gather textures
    for (uint32_t i = 0; i < userParams.size(); ++i)
    {
        ShaderParameter& param = userParams[i];
        if (param.mType == ShaderParameterType::Texture)
        {
            // Unlike uniform parameters, texture "mOffset" means descriptor binding location, not a byte offset or anything.
            param.mOffset = numTextureParams + 1;
            userParamsString += ("layout(set = 2, binding = " + std::to_string(param.mOffset) + ") uniform sampler2D " + param.mName + ";\n");
            numTextureParams++;
        }
    }
    
    // (X) Insert user code + parameter string into the template code
    {
        std::string paramStub = "---CUSTOM-PARAMS---";
        vertCode.replace(vertCode.find(paramStub), paramStub.length(), userParamsString);
        fragCode.replace(fragCode.find(paramStub), paramStub.length(), userParamsString);

        std::string codeStub = "---CUSTOM-CODE---";
        vertCode.replace(vertCode.find(codeStub), codeStub.length(), userCode);
        fragCode.replace(fragCode.find(codeStub), codeStub.length(), userCode);
    }

    // (X) Compile with shaderc / glslc to get the spirv (save to members)
    {
        Stream vertStream;
        vertStream.WriteString(vertCode);
        vertStream.WriteFile("Engine/Saves/TempMaterialVert.vert");

        Stream fragStream;
        fragStream.WriteString(fragCode);
        fragStream.WriteFile("Engine/Saves/TempMaterialFrag.frag");
    }

    // Remove old files first. (This could cause problems if file is opened and locked.
    // Just another reason this code should calling a library and not running shell commands.
    SYS_RemoveFile("Engine/Saves/TempMaterialVert.bin");
    SYS_RemoveFile("Engine/Saves/TempMaterialFrag.bin");

#if PLATFORM_WINDOWS
    std::string compStr = "cd Engine/Saves && \"%VULKAN_SDK%/Bin/glslc.exe TempMaterialVert.vert -Os -o TempMaterialVert.bin\" -I \"../Shaders/GLSL/src\"";
    SYS_Exec(compStr.c_str());
    compStr = "cd Engine/Saves && \"%VULKAN_SDK%/Bin/glslc.exe TempMaterialFrag.frag -Os -o TempMaterialFrag.bin\" -I \"../Shaders/GLSL/src\"";
    SYS_Exec(compStr.c_str());
    //SYS_Exec("cd Engine/Saves && \"./compile.bat\"");
#else
    std::string compStr = "cd Engine/Saves && \"%VULKAN_SDK%/Bin/glslc TempMaterialVert.vert -Os -o TempMaterialVert.bin\" -I \"../Shaders/GLSL/src\"";
    SYS_Exec(compStr.c_str());
    compStr = "cd Engine/Saves && \"%VULKAN_SDK%/Bin/glslc TempMaterialFrag.frag -Os -o TempMaterialFrag.bin\" -I \"../Shaders/GLSL/src\"";
    SYS_Exec(compStr.c_str());
    //SYS_Exec("cd Engine/Saves && \"./compile.sh\"");
#endif

    // Check if code compiled
    bool compiledSuccessful = false;

    {
        Stream vertBin;
        vertBin.ReadFile("Engine/Saves/TempMaterialVert.bin", false);

        Stream fragBin;
        fragBin.ReadFile("Engine/Saves/TempMaterialFrag.bin", false);

        if (vertBin.GetSize() > 0 && fragBin.GetSize() > 0)
        {
            compiledSuccessful = true;

            // ... Wait we need to compile 6 different vertex shaders...
        }
    }


    // (X) Fillout parameters using SpirvReflect (???? not needed?)
    // (X) Call GFX_BuildMaterial() to create pipelines/descriptor layout.
    // (X) Assuming compilation was successful, copy over old parameter values (that match new params) and param counts.
    // (X) Set the material's parameters to the new parameter list.

    // Relink any loaded material instances that use this base.
    std::unordered_map<std::string, AssetStub*>& assetMap = AssetManager::Get()->GetAssetMap();

    for (auto& pair : assetMap)
    {
        if (pair.second->mAsset && 
            pair.second->mType == MaterialInstance::GetStaticType())
        {
            MaterialInstance* inst = pair.second->mAsset->As<MaterialInstance>();
            if (inst && inst->GetBaseMaterial() == this)
            {
                inst->LinkToBase();
            }
        }
    }
#endif
}

void MaterialBase::MarkStale()
{
#if EDITOR
    mCompilationStale = true;
#endif
}

BlendMode MaterialBase::GetBlendMode() const
{
    return mBlendMode;
}

void MaterialBase::SetBlendMode(BlendMode blendMode)
{
    if (mBlendMode != blendMode)
    {
        mBlendMode = blendMode;
        MarkStale();
    }
}

float MaterialBase::GetMaskCutoff() const
{
    return mMaskCutoff;
}

void MaterialBase::SetMaskCutoff(float cutoff)
{
    if (mMaskCutoff != cutoff)
    {
        mMaskCutoff = cutoff;
        MarkStale();
    }
}

int32_t MaterialBase::GetSortPriority() const
{
    return mSortPriority;
}

void MaterialBase::SetSortPriority(int32_t priority)
{
    if (mSortPriority != priority)
    {
        mSortPriority = priority;
        MarkStale();
    }
}

bool MaterialBase::IsDepthTestDisabled() const
{
    return mDisableDepthTest;
}

void MaterialBase::SetDepthTestDisabled(bool depthTestDis)
{
    if (mDisableDepthTest != depthTestDis)
    {
        mDisableDepthTest = depthTestDis;
        MarkStale();
    }
}

bool MaterialBase::ShouldApplyFog() const
{
    return mApplyFog;
}

void MaterialBase::SetApplyFog(bool applyFog)
{
    if (mApplyFog != applyFog)
    {
        mApplyFog = applyFog;
        MarkStale();
    }
}

CullMode MaterialBase::GetCullMode() const
{
    return mCullMode;
}

void MaterialBase::SetCullMode(CullMode cullMode)
{
    if (mCullMode != cullMode)
    {
        mCullMode = cullMode;
        MarkStale();
    }
}

