#include "rmagine/util/assimp/helper.h"
#include <rmagine/math/assimp_conversions.h>

#include <vector>
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace rmagine
{

void get_nodes_with_meshes(const aiNode* node, std::vector<const aiNode*>& mesh_nodes)
{
    if(node->mNumMeshes > 0)
    {
        mesh_nodes.push_back(node);
    }

    if(node->mNumChildren > 0)
    {
        // is parent. check if it has meshes anyway
        for(size_t i=0; i<node->mNumChildren; i++)
        {
            get_nodes_with_meshes(node->mChildren[i], mesh_nodes);
        }
    }
}

void validate_triangle_mesh(
    const aiMesh* mesh,
    const std::string& backend)
{
    if(mesh == nullptr)
    {
        throw std::runtime_error(
            "[RMagine][" + backend + "] Null aiMesh");
    }

    if(!mesh->HasPositions())
    {
        throw std::runtime_error(
            "[RMagine][" + backend + "] Mesh has no vertices");
    }

    for(unsigned int i = 0; i < mesh->mNumVertices; ++i)
    {
        const aiVector3D& v = mesh->mVertices[i];

        if(!std::isfinite(v.x) ||
           !std::isfinite(v.y) ||
           !std::isfinite(v.z))
        {
            throw std::runtime_error(
                "[RMagine][" + backend +
                "] Non-finite vertex " + std::to_string(i));
        }
    }

    for(unsigned int i = 0; i < mesh->mNumFaces; ++i)
    {
        const aiFace& face = mesh->mFaces[i];

        if(face.mNumIndices != 3)
        {
            throw std::runtime_error(
                "[RMagine][" + backend + "] Face " +
                std::to_string(i) + " has " +
                std::to_string(face.mNumIndices) +
                " indices; expected 3");
        }

        for(unsigned int j = 0; j < 3; ++j)
        {
            if(face.mIndices[j] >= mesh->mNumVertices)
            {
                throw std::runtime_error(
                    "[RMagine][" + backend + "] Face " +
                    std::to_string(i) +
                    " contains out-of-range vertex index " +
                    std::to_string(face.mIndices[j]));
            }
        }
    }
}

std::vector<const aiNode*> get_nodes_with_meshes(const aiNode* node)
{
    std::vector<const aiNode*> ret;
    get_nodes_with_meshes(node, ret);
    return ret;
}

std::vector<std::string> path_names(const aiNode* node)
{
    std::vector<std::string> res;

    const aiNode* it = node;
    while(it)
    {
        res.push_back(std::string(it->mName.C_Str()));
        it = it->mParent;
    }

    std::reverse(res.begin(), res.end());
    return res;
}

Matrix4x4 global_transform(const aiNode* node)
{
    Matrix4x4 M;
    M.setIdentity();

    const aiNode* it = node;
    while(it != NULL)
    {
        M = convert(it->mTransformation) * M;
        it = it->mParent;
    }

    return M;
}

} // namespace rmagine