/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2022 Samuel Huang - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "asset.h"
#include "graphic/rhi/rhicommon.h"

#ifdef ETH_TOOLMODE
#include "toolmode/asset/intermediate/texture.h"
#endif

ETH_NAMESPACE_BEGIN

class ETH_ENGINE_DLL CompiledTexture : public Asset
{
public:
    CompiledTexture() = default;
    ~CompiledTexture();

    void Serialize(OStream& ostream) override;
    void Deserialize(IStream& istream) override;

public:
    inline RhiResourceHandle& GetResource() { return m_Resource; }
    inline RhiResourceHandle GetResource() const { return m_Resource; }
    inline RhiShaderResourceViewHandle& GetView() { return m_View; }
    inline RhiShaderResourceViewHandle GetView() const { return m_View; }

    inline uint32_t GetWidth() const { return m_Width; }
    inline uint32_t GetHeight() const { return m_Height; }
    inline uint32_t GetDepth() const { return m_Depth; }
    inline RhiFormat GetFormat() const { return m_Format; }

    inline void* GetData() const { return m_Data; }

    size_t GetSizeInBytes() const;
    size_t GetBytesPerPixel() const;

public:
    ETH_TOOLONLY(void SetRawTexture(std::shared_ptr<Texture> rawTexture));

private:
    friend class GraphicCommon;

    ETH_TOOLONLY(std::shared_ptr<Texture> m_RawTexture);
    uint32_t m_Width;
    uint32_t m_Height;
    uint32_t m_Depth;
    RhiFormat m_Format;
    unsigned char* m_Data;

    RhiResourceHandle m_Resource = RhiResourceHandle(nullptr, L"CompiledTextureResource");
    RhiShaderResourceViewHandle m_View = RhiShaderResourceViewHandle(nullptr, L"CompiledTextureView");;
};

ETH_NAMESPACE_END

