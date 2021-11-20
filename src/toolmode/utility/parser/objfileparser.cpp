/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2021 Samuel Van Allen - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "objfileparser.h"
#include "toolmode/asset/intermediate/rawmesh.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "parser/tiny_obj_loader.h"

ETH_NAMESPACE_BEGIN

void ObjFileParser::Parse(const std::string& path, Asset* asset)
{
    MeshAsset* meshAsset = static_cast<MeshAsset*>(asset);
    RawMesh& mesh = meshAsset->GetRawMesh();

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> objmaterials;
    std::string err;

    bool success = tinyobj::LoadObj(&attrib, &shapes, &objmaterials, &err, 
        path.c_str(), //model to load
        path.c_str(), //directory to search for materials
        true); //enable triangulation

    if (!success)
    {
        LogToolmodeError("Failed to load mesh: %s", path.c_str());
        return;
    }

    for (size_t i = 0; i < attrib.vertices.size(); i += 3)
        mesh.m_Positions.emplace_back(attrib.vertices[i], attrib.vertices[i + 1], attrib.vertices[i + 2]);

    for (size_t i = 0; i < attrib.normals.size(); i += 3)
        mesh.m_Normals.emplace_back(attrib.normals[i], attrib.normals[i + 1], attrib.normals[i + 2]);

    for (size_t i = 0; i < attrib.texcoords.size(); i += 2)
        mesh.m_TexCoords.emplace_back(attrib.texcoords[i], attrib.texcoords[i + 1]);

    for (auto shape = shapes.begin(); shape < shapes.end(); ++shape)
        for (auto i = 0; i < shape->mesh.indices.size(); ++i)
            mesh.m_Indices.emplace_back(shape->mesh.indices[i].vertex_index);

    mesh.Format();
    meshAsset->UpdateBuffers();
}

ETH_NAMESPACE_END

