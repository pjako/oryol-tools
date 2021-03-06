//------------------------------------------------------------------------------
//  Config.cc
//------------------------------------------------------------------------------
#include "Config.h"
#include "ExportUtil/Log.h"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "assimp/config.h"
#include <iostream>

namespace OryolTools {

struct NameAndValue {
    const char* name;
    unsigned int val;
};

static NameAndValue aiProcessFlags[] = {
    { "CalcTangentSpace", aiProcess_CalcTangentSpace },
    { "JoinIdenticalVertices", aiProcess_JoinIdenticalVertices },
    { "MakeLeftHanded", aiProcess_MakeLeftHanded },
    { "Triangulate", aiProcess_Triangulate },
    { "RemoveComponent", aiProcess_RemoveComponent },
    { "GenNormals", aiProcess_GenNormals },
    { "GenSmoothNormals", aiProcess_GenSmoothNormals },
    { "SplitLargeMeshes", aiProcess_SplitLargeMeshes },
    { "PreTransformVertices", aiProcess_PreTransformVertices },
    { "LimitBoneWeights", aiProcess_LimitBoneWeights },
    { "ValidateDataStructure", aiProcess_ValidateDataStructure },
    { "ImproveCacheLocality", aiProcess_ImproveCacheLocality },
    { "RemoveRedundantMaterials", aiProcess_RemoveRedundantMaterials },
    { "FixInfacingNormals", aiProcess_FixInfacingNormals },
    { "SortByPType", aiProcess_SortByPType },
    { "FindDegenerates", aiProcess_FindDegenerates },
    { "FindInvalidData", aiProcess_FindInvalidData },
    { "GenUVCoords", aiProcess_GenUVCoords },
    { "TransformUVCoords", aiProcess_TransformUVCoords },
    { "FindInstances", aiProcess_FindInstances },
    { "OptimizeMeshes", aiProcess_OptimizeMeshes },
    { "OptimizeGraph", aiProcess_OptimizeGraph },
    { "FlipUVs", aiProcess_FlipUVs },
    { "FlipWindingOrder", aiProcess_FlipWindingOrder },
    { "SplitByBoneCount", aiProcess_SplitByBoneCount },
    { "Debone", aiProcess_Debone }
};
static NameAndValue componentFlags[] = {
    { "NORMALS", aiComponent_NORMALS },
    { "TANGENTS_AND_BITANGENTS", aiComponent_TANGENTS_AND_BITANGENTS },
    { "COLORS", aiComponent_COLORS },
    { "COLORS0", aiComponent_COLORSn(0) },
    { "COLORS1", aiComponent_COLORSn(1) },
    { "COLORS2", aiComponent_COLORSn(2) },
    { "COLORS3", aiComponent_COLORSn(3) },
    { "TEXCOORDS", aiComponent_TEXCOORDS },
    { "TEXCOORDS0", aiComponent_TEXCOORDSn(0) },
    { "TEXCOORDS1", aiComponent_TEXCOORDSn(1) },
    { "TEXCOORDS2", aiComponent_TEXCOORDSn(2) },
    { "TEXCOORDS3", aiComponent_TEXCOORDSn(3) },
    { "BONEWEIGHTS", aiComponent_BONEWEIGHTS },
    { "ANIMATIONS", aiComponent_ANIMATIONS },
    { "TEXTURES", aiComponent_TEXTURES },
    { "LIGHTS", aiComponent_LIGHTS },
    { "CAMERAS", aiComponent_CAMERAS },
    { "MESHES", aiComponent_MESHES },
    { "MATERIALS", aiComponent_MATERIALS }
};

static NameAndValue primTypeFlags[] = {
    { "POINT", aiPrimitiveType_POINT },
    { "LINE", aiPrimitiveType_LINE },
    { "TRIANGLE", aiPrimitiveType_TRIANGLE },
    { "POLYGON", aiPrimitiveType_POLYGON }
};

//------------------------------------------------------------------------------
bool
Config::Load(const std::string& path) {
    try {
        this->config = cpptoml::parse_file(path);
    }
    catch (const cpptoml::parse_exception& e) {
        Log::Warn("Config::Load(): %s\n", e.what());
        return false;
    }
    return true;
}

//------------------------------------------------------------------------------
unsigned int
Config::GetAiProcessFlags() const {
    unsigned int result = 0;
    if (this->config.contains("aiProcess")) {
        auto flags = this->config.get("aiProcess");
        for (const auto& item : flags->as_array()->get()) {
            const std::string name = item->as<std::string>()->get();
            for (const auto& flagItem : aiProcessFlags) {
                if (name == flagItem.name) {
                    result |= flagItem.val;
                }
            }
        }
    }
    else {
        Log::Warn("no aiProcess entry in config!\n");
    }
    return result;
}

//------------------------------------------------------------------------------
unsigned int
Config::GetAiProcessRemoveComponentsFlags() const {
    unsigned int result = 0;
    if (this->config.contains("PP_RVC_FLAGS")) {
        auto flags = this->config.get("PP_RVC_FLAGS");
        for (const auto& item : flags->as_array()->get()) {
            const std::string name = item->as<std::string>()->get();
            for (const auto& rvcItem : componentFlags) {
                if (name == rvcItem.name) {
                    result |= rvcItem.val;
                }
            }
        }
    }
    else {
        Log::Warn("no PP_RVC_FLAGS in config!\n");
    }
    return result;
}

//------------------------------------------------------------------------------
unsigned int
Config::GetAiProcessSortByPTypeRemoveFlags() const {
    unsigned int result = 0;
    if (this->config.contains("PP_SBP_REMOVE")) {
        auto flags = this->config.get("PP_SBP_REMOVE");
        for (const auto& item : flags->as_array()->get()) {
            const std::string name = item->as<std::string>()->get();
            for (const auto& primTypeItem : primTypeFlags) {
                if (name == primTypeItem.name) {
                    result |= primTypeItem.val;
                }
            }
        }
    }
    else {
        Log::Warn("no PP_SBP_REMOVE in config!\n");
    }
    return result;
}

//------------------------------------------------------------------------------
int
Config::GetIndexSize() const {
    if (this->config.contains("IndexSize")) {
        return this->config.get("IndexSize")->as<int64_t>()->get();
    }
    else {
        Log::Warn("no IndexSize in config, using 2 (for 16-bit indices)!\n");
        return 2;
    }
}

//------------------------------------------------------------------------------
VertexLayout
Config::GetLayout() const {
    if (this->config.contains("Layout")) {
        VertexLayout result;
        auto layout = this->config.get("Layout")->as_table();
        for (int attrIndex = 0; attrIndex < VertexAttr::Num; attrIndex++) {
            const std::string& attrName = VertexAttr::ToString((VertexAttr::Code)attrIndex);
            if (layout->contains(attrName)) {
                auto attr = layout->get(attrName)->as_table();
                if (attr->contains("format")) {
                    auto format = attr->get("format");
                    result.Components[attrIndex].Format = VertexFormat::FromString(format->as<std::string>()->get());
                }
                if (attr->contains("scale")) {
                    auto scale = attr->get("scale");
                    float scaleValue = 1.0f;
                    if (scale->as<double>()) {
                        scaleValue = scale->as<double>()->get();
                    }
                    else if (scale->as<int64_t>()) {
                        scaleValue = scale->as<int64_t>()->get();
                    }
                    result.Components[attrIndex].Scale = scaleValue;
                }
                if (attr->contains("bias")) {
                    auto bias = attr->get("bias");
                    float biasValue = 0.0f;
                    if (bias->as<double>()) {
                        biasValue = bias->as<double>()->get();
                    }
                    else if (bias->as<int64_t>()) {
                        biasValue = bias->as<int64_t>()->get();
                    }
                    result.Components[attrIndex].Bias = biasValue;
                }
            }
        }
        return result;
    }
    else {
        Log::Fatal("no VertexLayout in config!");
        return VertexLayout();
    }
}

} // namesapce OryolTools