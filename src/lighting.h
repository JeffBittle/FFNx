/****************************************************************************/
//    Copyright (C) 2009 Aali132                                            //
//    Copyright (C) 2018 quantumpencil                                      //
//    Copyright (C) 2018 Maxime Bacoux                                      //
//    Copyright (C) 2020 myst6re                                            //
//    Copyright (C) 2020 Chris Rizzitello                                   //
//    Copyright (C) 2020 John Pritchard                                     //
//    Copyright (C) 2022 Julian Xhokaxhiu                                   //
//    Copyright (C) 2022 Cosmos                                             //
//                                                                          //
//    This file is part of FFNx                                             //
//                                                                          //
//    FFNx is free software: you can redistribute it and/or modify          //
//    it under the terms of the GNU General Public License as published by  //
//    the Free Software Foundation, either version 3 of the License         //
//                                                                          //
//    FFNx is distributed in the hope that it will be useful,               //
//    but WITHOUT ANY WARRANTY; without even the implied warranty of        //
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         //
//    GNU General Public License for more details.                          //
/****************************************************************************/

#pragma once

#include "matrix.h"
#include "common.h"
#include "globals.h"

#include <vector>

enum DebugOutput
{
    DEBUG_OUTPUT_DISABLED = 0,
    DEBUG_OUTPUT_COLOR,
    DEBUG_OUTPUT_NORMALMAP,
    DEBUG_OUTPUT_ROUGHNESS,
    DEBUG_OUTPUT_METALLIC,
    DEBUG_OUTPUT_AO,
    DEBUG_OUTPUT_SPECULAR,
    DEBUG_OUTPUT_IBL_SPECULAR,
    DEBUG_OUTPUT_IBL_DIFFUSE,
};

struct walkmeshEdge
{
    int v0;                      // edge vertex index 0
    int v1;                      // edge vertex index 1
    int ov;                      // opposite vertex index
    int prevEdge;                // previous vertex index
    int nextEdge;	             // next vertex index
    bool isBorder;               // border edge flag
    vector3<float> perpDir;      // perpendicular direction
};

struct LightingState
{
    float lightViewMatrix[16];
    float lightProjMatrix[16];
    float lightInvViewProjTexMatrix[16];
    float lightViewProjMatrix[16];
    float lightViewProjTexMatrix[16];

    float lightingSettings[4] = { 1.0, 1.0, 1.0, 0.0 };
    float lightDirData[4] = { 0.3, -1.0, -0.3, 0.0 };
    float lightData[4] = { 1.0, 1.0, 1.0, 4.0 };
    float ambientLightData[4] = { 1.0, 1.0, 1.0, 1.0 };
    float materialData[4] = { 0.7, 0.5, 0.0, 0.0 };
    float materialScaleData[4] = { 1.0, 1.0, 1.0, 1.0 };
    float shadowData[4] = { 0.001, 0.0, 0.0, 2048.0 };
    float fieldShadowData[4] = { 0.3, 200.0, 100.0, 0.0 };
    float iblData[4] = { 1.0, 0.0, 0.0, 0.0 };

    float lightingDebugData[4] = { 0.0, 0.0, 0.0, 0.0 };

    // Light Direction
    vector3<float> worldLightRot = { 60.0, 60.0, 0.0 };

    // Shadowmap face culling
    bool isShadowMapFaceCullingEnabled = false;

    // Battle shadowmap frustum parameters
    float shadowMapArea = 20000.0f;
    float shadowMapNearFarSize = 20000.0f;

    // Field shadowmap frustum parameters
    float fieldShadowMapArea = 3000.0f;
    float fieldShadowMapNearFarSize = 3000.0f;

    // Field Shadow walkmesh parameters
    float walkMeshExtrudeSize = 20.0;
    float walkMeshPosOffset = -11.0;
};

class Lighting
{
private:
    LightingState lightingState;

    // Config
    toml::parse_result config;

private:
    void loadConfig();
    void initParamsFromConfig();
    void updateLightMatrices(struct boundingbox* sceneAabb);

    void ff7_load_ibl();

    void ff7_get_field_view_matrix(struct matrix* outViewMatrix);
    void ff7_create_walkmesh(std::vector<struct nvertex>& vertices, std::vector<WORD>& indices, std::vector<struct walkmeshEdge>& edges);

    void createFieldWalkmesh(std::vector<struct nvertex>& vertices, std::vector<WORD>& indices, float extrudeSize);
    void extractWalkmeshBorderData(std::vector<struct nvertex>& vertices, std::vector<struct walkmeshEdge>& edges);
    void createWalkmeshBorderExtrusionData(std::vector<struct nvertex>& vertices, std::vector<struct walkmeshEdge>& edges);
    void createWalkmeshBorder(std::vector<struct nvertex>& vertices, std::vector<WORD>& indices, std::vector<struct walkmeshEdge>& edges, float extrudeSize);
    struct boundingbox calcFieldSceneAabb(struct boundingbox* sceneAbb, struct matrix* viewMatrix);

public:
    void init();

    void draw(struct game_obj* game_object);
    void drawFieldShadow();

    const LightingState& getLightingState();

    // Lighting
    void setPbrTextureEnabled(bool isEnabled);
    bool isPbrTextureEnabled();
    void setEnvironmentLightingEnabled(bool isEnabled);
    bool isEnvironmentLightingEnabled();
    void setWorldLightDir(float dirX, float dirY, float dirZ);
    vector3<float> getWorldLightDir();
    void setLightIntensity(float intensity);
    float getLightIntensity();
    void setLightColor(float r, float g, float b);
    vector3<float> getLightColor();
    void setAmbientIntensity(float intensity);
    float getAmbientIntensity();
    void setAmbientLightColor(float r, float g, float b);
    vector3<float> getAmbientLightColor();
    void setIblMipCount(int mipCount);
    bool isDisabledLightingTexture(const std::string& textureName);

    // Material
    void setRoughness(float roughness);
    float getRoughness();
    void setMetallic(float metallic);
    float getMetallic();
    void setSpecular(float metallic);
    float getSpecular();
    void setRoughnessScale(float scale);
    float getRoughnessScale();
    void setMetallicScale(float scale);
    float getMetallicScale();
    void setSpecularScale(float scale);
    float getSpecularScale();

    // Shadow (common)
    void setShadowFaceCullingEnabled(bool isEnabled);
    bool isShadowFaceCullingEnabled();
    void setShadowMapResolution(int size);
    int getShadowMapResolution();
    void setShadowConstantBias(float bias);
    float getShadowConstantBias();

    // Battle Shadow
    void setShadowMapArea(float area);
    float getShadowMapArea();
    void setShadowMapNearFarSize(float size);
    float getShadowMapNearFarSize();

    // Field Shadow
    void setFieldShadowMapArea(float area);
    float getFieldShadowMapArea();
    void setFieldShadowMapNearFarSize(float size);
    float getFieldShadowMapNearFarSize();
    void setFieldShadowOcclusion(float value);
    float getFieldShadowOcclusion();
    void setFieldShadowFadeStartDistance(float value);
    float getFieldShadowFadeStartDistance();
    void setFieldShadowFadeRange(float value);
    float getFieldShadowFadeRange();
    void setWalkmeshExtrudeSize(float size);
    float getWalkmeshExtrudeSize();
    void setWalkmeshPosOffset(float size);
    float getWalkmeshPosOffset();

    // Lighting Debug
    void setHide2dEnabled(bool isEnabled);
    bool isHide2dEnabled();
    void setShowWalkmeshEnabled(bool isEnabled);
    bool isShowWalkmeshEnabled();
    void setDebugOutput(DebugOutput output);
    DebugOutput GetDebugOutput();
};

extern Lighting lighting;
