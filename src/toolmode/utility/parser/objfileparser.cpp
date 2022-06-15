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

#include "objfileparser.h"
#include "toolmode/utility/pathutils.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "parser/mesh/tiny_obj_loader.h"

ETH_NAMESPACE_BEGIN

void ObjFileParser::Parse(const std::string& path)
{
    m_RawMeshGroup = std::make_unique<MeshGroup>();

    tinyobj::ObjReaderConfig reader_config;
    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(path, reader_config)) {
        if (!reader.Error().empty())
			LogToolmodeError("Failed to load mesh: %s", path.c_str());
            LogToolmodeError(reader.Error().c_str());
        return;
    }

    if (!reader.Warning().empty())
        LogToolmodeWarning(reader.Warning().c_str());

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    auto& materials = reader.GetMaterials();
    std::string err;

	m_RawMeshGroup->m_MaterialTable.resize(materials.size());
	for (int i = 0; i < materials.size(); ++i)
	{
        const auto rawMat = materials[i];
        m_RawMeshGroup->m_MaterialTable[i] = std::make_shared<Material>();
        m_RawMeshGroup->m_MaterialTable[i]->SetRoughness(rawMat.roughness);
        m_RawMeshGroup->m_MaterialTable[i]->SetMetalness(rawMat.metallic);
        m_RawMeshGroup->m_MaterialTable[i]->SetBaseColor({ rawMat.diffuse[0], rawMat.diffuse[1], rawMat.diffuse[2], 1.0f });
        m_RawMeshGroup->m_MaterialTable[i]->SetSpecularColor({ rawMat.specular[0], rawMat.specular[1], rawMat.specular[2], 1.0f });
		m_RawMeshGroup->m_MaterialTable[i]->m_AlbedoTexturePath = rawMat.diffuse_texname;
		m_RawMeshGroup->m_MaterialTable[i]->m_SpecularTexturePath = rawMat.specular_texname;
		m_RawMeshGroup->m_MaterialTable[i]->m_RoughnessTexturePath = rawMat.roughness_texname;
		m_RawMeshGroup->m_MaterialTable[i]->m_MetalnessTexturePath = rawMat.metallic_texname;
	}

    // Loop over shapes (each shape is a "mesh" that should have different materials)
    // LIMITATION FOR NOW: Each shape can only have 1 material. In order to support multiple
    // material per shape, we cannot render the entire shape in one single draw call.
    // We would have to further split each shape (internally) by material and submit draw calls
    // per material in the renderer. (TODO)

    for (size_t s = 0; s < shapes.size(); s++) {

        std::shared_ptr<RawMesh> rawMesh = std::make_shared<RawMesh>();

        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

            // LIMITATION FOR NOW: polygons must be triangles. In order to support n-gon
            // rendering, we cannot draw simply with a triangle list. Most engines simplify
            // the rendering process by triangulating imported meshes to fix this, and if we 
            // want to support that we should probably do it somewhere here (TODO)
            AssertToolmode(fv == 3, "Only triangulated meshes are supported! Could bypass assert for undefined behaviour");

            // Loop over vertices in the face.
            for (size_t v = 0; v < fv; v++) {
                // access to vertex
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

                // TODO: Resize the array in the beginning for better performance
                // z-axis is flipped (pos/norm) because .obj specs specifies that vertices be in RH coordinate system
                // but Ether uses a LH coordinate system by convention.
                rawMesh->m_Positions.emplace_back(vx, vy, -vz);
 
                // Check if `normal_index` is zero or positive. negative = no normal data
                if (idx.normal_index >= 0) {
                    tinyobj::real_t nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
                    tinyobj::real_t ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
                    tinyobj::real_t nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
                    rawMesh->m_Normals.emplace_back(nx, ny, -nz);
                }

                // Check if `texcoord_index` is zero or positive. negative = no texcoord data
                if (idx.texcoord_index >= 0) {
                    tinyobj::real_t tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
                    tinyobj::real_t ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
                    rawMesh->m_TexCoords.emplace_back(tx, ty);
                }
            }

            // per-face material
            if (materials.size() > shapes[s].mesh.material_ids[f])
				rawMesh->m_Material = m_RawMeshGroup->m_MaterialTable[shapes[s].mesh.material_ids[f]];

            index_offset += fv;
        }

        //// Assign indices
        //for (auto i = 0; i < shapes[s].mesh.indices.size(); ++i)
        //{
        //    rawMesh->m_PositionIndices.emplace_back(shapes[s].mesh.indices[i].vertex_index);

        //    if (shapes[s].mesh.indices[i].normal_index != -1)
        //        rawMesh->m_NormalIndices.emplace_back(shapes[s].mesh.indices[i].normal_index);
        //    else
        //        rawMesh->m_NormalIndices.emplace_back(0);

        //    if (shapes[s].mesh.indices[i].texcoord_index != -1)
        //        rawMesh->m_TexCoordIndices.emplace_back(shapes[s].mesh.indices[i].texcoord_index);
        //    else
        //        rawMesh->m_TexCoordIndices.emplace_back(0);
        //}

        m_RawMeshGroup->m_SubMeshes.push_back(rawMesh);
    }
}

std::shared_ptr<Asset> ObjFileParser::GetRawAsset() const
{
    return std::dynamic_pointer_cast<Asset>(m_RawMeshGroup);
}

ETH_NAMESPACE_END

