/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2030 Samuel Huang - All rights reserved.

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

#include "graphics/graphiccore.h"
#include "graphics/rhi/rhishader.h"
#include "graphics/rhi/rhishaderreflection.h"
#include <filesystem>

#define ETH_CLASS_ID_SHADER "Graphics::Shader"

constexpr uint32_t ShaderVersion = 0;

Ether::Graphics::RhiShader::RhiShader(const RhiShaderDesc& desc)
    : Serializable(ShaderVersion, ETH_CLASS_ID_SHADER)
    , m_Type(desc.m_Type)
    , m_Reflection(nullptr)
    , m_IsCompiled(false)
    , m_FileName(desc.m_Filename)
    , m_FilePath(GraphicCore::GetGraphicConfig().GetShaderSourcePath() + "\\" + desc.m_Filename)
    , m_EntryPoint(desc.m_EntryPoint)
{
}

void Ether::Graphics::RhiShader::Serialize(OStream& ostream) const
{
    AssertGraphics(m_IsCompiled, "Shaders that have yet to be compiled cannot be serialized");
    Serializable::Serialize(ostream);

    ostream << m_Type;
    ostream << m_FileName;
    ostream << m_FilePath;
    ostream << m_EntryPoint;
    ostream << static_cast<uint32_t>(m_CompiledData.size());
    ostream.WriteBytes(m_CompiledData.data(), m_CompiledData.size());

    m_Reflection->Serialize(ostream);
}

void Ether::Graphics::RhiShader::Deserialize(IStream& istream)
{
    Serializable::Deserialize(istream);

    istream >> m_Type;
    istream >> m_FileName;
    istream >> m_FilePath;
    istream >> m_EntryPoint;

    uint32_t compiledSize;
    istream >> compiledSize;

    m_CompiledData.resize(compiledSize);
    if (compiledSize > 0)
        istream.ReadBytes(m_CompiledData.data(), compiledSize);

    m_Reflection = std::make_unique<RhiShaderReflection>();
    m_Reflection->Deserialize(istream);
    m_IsCompiled = true;
}

bool Ether::Graphics::RhiShader::TryLoadFromCache()
{
    IFileStream fstream(m_FilePath);
    if (!fstream.IsOpen() || fstream.GetFileSize() <= 0)
    {
        return false;
    }

    size_t fileSize = fstream.GetFileSize();
    uint8_t* data = (uint8_t*)malloc(fileSize);
    fstream.ReadBytes(data, fileSize);
    size_t hash = ComputeHash(data, fileSize);

    std::string cachedShaderFile = std::format("{}\\{:016x}.shcache", GraphicCore::GetGraphicConfig().GetCompiledShaderPath(), hash);
    IFileStream cachedShader(cachedShaderFile);

    if (!cachedShader.IsOpen() || cachedShader.GetFileSize() <= 0)
    {
        return false;
    }

    LogGraphicsInfo("Loaded shader %s from cache %s", m_FileName.c_str(), cachedShaderFile.c_str());

    Deserialize(cachedShader);
    return true;
}

void Ether::Graphics::RhiShader::SaveToCache()
{
    IFileStream fstream(m_FilePath);
    if (!fstream.IsOpen() || fstream.GetFileSize() <= 0)
    {
        LogGraphicsWarning("Failed to open shader file %s", m_FilePath);
        return;
    }

    size_t fileSize = fstream.GetFileSize();
    uint8_t* data = (uint8_t*)malloc(fileSize);
    fstream.ReadBytes(data, fileSize);
    size_t hash = ComputeHash(data, fileSize);

    std::string cacheDir = GraphicCore::GetGraphicConfig().GetCompiledShaderPath();
    std::filesystem::create_directories(cacheDir);

    std::string cachedShaderFile = std::format("{}\\{:016x}.shcache", cacheDir, hash);
    OFileStream cachedShader(cachedShaderFile);
    Serialize(cachedShader);
}

size_t Ether::Graphics::RhiShader::ComputeHash(uint8_t* data, size_t size) const
{
    std::hash<std::string_view> hasher;
    size_t hash = hasher(std::string_view(reinterpret_cast<const char*>(data), size));
    auto hashCombine = [](size_t& seed, size_t value) { seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2); };

    hashCombine(hash, std::hash<uint32_t>{}((uint32_t)m_Type));
    hashCombine(hash, std::hash<std::string>{}(m_FilePath));
    hashCombine(hash, std::hash<std::string>{}(m_EntryPoint));

    return hash;
}

