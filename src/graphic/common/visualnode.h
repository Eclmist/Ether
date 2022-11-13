/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2023 Samuel Huang - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

ETH_NAMESPACE_BEGIN

struct VisualNodeDesc
{
    uint32_t m_NodeId;

    const void* m_VertexBuffer;
    uint32_t m_NumVertices;

    const ethMatrix4x4* m_ModelMatrix;
    const Material* m_Material;

    ETH_TOOLONLY(ethVector4u m_PickerColor);
};

struct VisualNodeInstanceParams
{
    struct MaterialParams
    {
        ethVector4 m_BaseColor;
        ethVector4 m_SpecularColor;
        float m_Roughness;
        float m_Metalness;
        uint32_t m_AlbedoTextureIndex;
        uint32_t m_SpecularTextureIndex;
    };

    MaterialParams m_MaterialParams;

    ethMatrix4x4 m_ModelMatrix;
    ethMatrix4x4 m_NormalMatrix;

    float m_Padding[16];

    #ifdef ETH_TOOLMODE
        ethVector4u m_PickerColor;
    #else
        float m_Padding2[4];
    #endif

}; static_assert((sizeof(VisualNodeInstanceParams) % 256) == 0, "Constant Buffer size must be 256-byte aligned");

class VisualNode
{
public:
    VisualNode(const VisualNodeDesc desc);
    ~VisualNode();

    inline RhiVertexBufferViewDesc GetVertexBufferView() const { return m_VertexBufferView; }
    inline uint32_t GetNumVertices() const { return m_NodeDesc.m_NumVertices; }
    inline const ethMatrix4x4 GetModelMatrix() const { return *m_NodeDesc.m_ModelMatrix; }
    inline const Material* GetMaterial() const { return m_NodeDesc.m_Material; }
    inline RhiResourceHandle GetInstanceParams() const { return m_InstanceParams; }
    inline VisualNodeInstanceParams* GetMappedParams() const { return m_MappedParams; }

    ETH_TOOLONLY(inline const ethVector4u GetPickerColor() const { return m_NodeDesc.m_PickerColor; })

private:
    void UploadVertexBuffer(const void* data, uint32_t numVertices);
    void InitVertexBufferView(size_t bufferSize, size_t stride);
    void InitInstanceParams();

private:
    RhiResourceHandle m_VertexBuffer;
    RhiResourceHandle m_IndexBuffer;
    RhiResourceHandle m_InstanceParams;

    RhiVertexBufferViewDesc m_VertexBufferView;
    RhiIndexBufferViewDesc m_IndexBufferView;

    VisualNodeDesc m_NodeDesc;
    VisualNodeInstanceParams* m_MappedParams;
};

ETH_NAMESPACE_END

