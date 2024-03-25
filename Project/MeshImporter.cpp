#include "MeshImporter.h"

#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <assimp/Importer.hpp>
#include <set>

#include "AnimationImporter.h"
#include "AssetManager.h"
#include "MeshImporterUtilities.h"
#include "RendererApi.h"

namespace base_engine {
static constexpr uint32_t kMeshImportFlags =
    aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_SortByPType |
    aiProcess_GenNormals | aiProcess_GenUVCoords | aiProcess_OptimizeMeshes |
    aiProcess_JoinIdenticalVertices | aiProcess_LimitBoneWeights |
    aiProcess_ValidateDataStructure | aiProcess_GlobalScale;

MeshImporter::MeshImporter(std::filesystem::path path)
    : path_(std::move(path)) {}

Ref<MeshSource> MeshImporter::ImportToMeshSource() {
  auto mesh_source = Ref<MeshSource>::Create();
  Assimp::Importer importer;

  const aiScene* scene = importer.ReadFile(path_.string(), kMeshImportFlags);
  if (!scene) {
    BE_CORE_ERROR_TAG("Mesh", "Failed to load mesh file: {0}", path_.string());
    return nullptr;
  }

  mesh_source->SetSkeleton(AnimationImporter::ImportSkeleton(scene));

  // アニメーションのロードはこの段階では行わない
  mesh_source->m_Animations.resize(scene->mNumAnimations);
  mesh_source->m_AnimationNames.reserve(scene->mNumAnimations);
  for (uint32_t i = 0; i < scene->mNumAnimations; ++i) {
    mesh_source->m_AnimationNames.emplace_back(
        scene->mAnimations[i]->mName.C_Str());
  }

  if (scene->HasMeshes()) {
    uint32_t vertex_count = 0;
    uint32_t index_count = 0;

    mesh_source->m_BoundingBox.Min = {FLT_MAX, FLT_MAX, FLT_MAX};
    mesh_source->m_BoundingBox.Max = {-FLT_MAX, -FLT_MAX, -FLT_MAX};

    mesh_source->m_Submeshes.reserve(scene->mNumMeshes);
    for (unsigned m = 0; m < scene->mNumMeshes; m++) {
      aiMesh* mesh = scene->mMeshes[m];

      auto& [BaseVertex, BaseIndex, MaterialIndex, IndexCount, VertexCount,
             Transform, LocalTransform, BoundingBox, NodeName, MeshName,
             IsRigged] = mesh_source->m_Submeshes.emplace_back();
      BaseVertex = vertex_count;
      BaseIndex = index_count;
      MaterialIndex = mesh->mMaterialIndex;
      VertexCount = mesh->mNumVertices;
      IndexCount = mesh->mNumFaces * 3;
      MeshName = mesh->mName.C_Str();

      vertex_count += mesh->mNumVertices;
      index_count += IndexCount;

      BE_CORE_ASSERT(mesh->HasPositions(), "Meshes require positions.");
      BE_CORE_ASSERT(mesh->HasNormals(), "Meshes require normals.");

      auto& aabb = BoundingBox;
      constexpr float float_max = (std::numeric_limits<float>::max)();
      constexpr float float_min = (std::numeric_limits<float>::min)();
      aabb.Min = {float_max, float_max, float_max};
      aabb.Max = {float_min, float_min, float_min};
      for (size_t i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        vertex.Position = {mesh->mVertices[i].x, mesh->mVertices[i].y,
                           mesh->mVertices[i].z};
        vertex.Normal = {mesh->mNormals[i].x, mesh->mNormals[i].y,
                         mesh->mNormals[i].z};
        aabb.Min.x = glm::min(vertex.Position.x, aabb.Min.x);
        aabb.Min.y = glm::min(vertex.Position.y, aabb.Min.y);
        aabb.Min.z = glm::min(vertex.Position.z, aabb.Min.z);
        aabb.Max.x = glm::max(vertex.Position.x, aabb.Max.x);
        aabb.Max.y = glm::max(vertex.Position.y, aabb.Max.y);
        aabb.Max.z = glm::max(vertex.Position.z, aabb.Max.z);

        if (mesh->HasTangentsAndBitangents()) {
          vertex.Tangent = {mesh->mTangents[i].x, mesh->mTangents[i].y,
                            mesh->mTangents[i].z};
          vertex.Binormal = {mesh->mBitangents[i].x, mesh->mBitangents[i].y,
                             mesh->mBitangents[i].z};
        }

        if (mesh->HasTextureCoords(0))
          vertex.Texcoord = {mesh->mTextureCoords[0][i].x,
                             mesh->mTextureCoords[0][i].y};

        mesh_source->m_Vertices.push_back(vertex);
      }

      // Indices
      for (size_t i = 0; i < mesh->mNumFaces; i++) {
        BE_CORE_ASSERT(mesh->mFaces[i].mNumIndices == 3,
                       "Must have 3 indices.");
        Index index = {mesh->mFaces[i].mIndices[0], mesh->mFaces[i].mIndices[1],
                       mesh->mFaces[i].mIndices[2]};
        mesh_source->m_Indices.push_back(index);

        mesh_source->m_TriangleCache[m].emplace_back(
            mesh_source->m_Vertices[index.V1 + BaseVertex],
            mesh_source->m_Vertices[index.V2 + BaseVertex],
            mesh_source->m_Vertices[index.V3 + BaseVertex]);
      }
    }

    mesh_source->m_Nodes.emplace_back();
    TraverseNodes(mesh_source, scene->mRootNode, 0);

    for (const auto& [BaseVertex, BaseIndex, MaterialIndex, IndexCount,
                      VertexCount, Transform, LocalTransform, BoundingBox,
                      NodeName, MeshName, IsRigged] :
         mesh_source->m_Submeshes) {
      AABB transformed_submesh_aabb = BoundingBox;
      auto min =
          glm::vec3(Transform * glm::vec4(transformed_submesh_aabb.Min, 1.0f));
      auto max =
          glm::vec3(Transform * glm::vec4(transformed_submesh_aabb.Max, 1.0f));

      mesh_source->m_BoundingBox.Min.x =
          glm::min(mesh_source->m_BoundingBox.Min.x, min.x);
      mesh_source->m_BoundingBox.Min.y =
          glm::min(mesh_source->m_BoundingBox.Min.y, min.y);
      mesh_source->m_BoundingBox.Min.z =
          glm::min(mesh_source->m_BoundingBox.Min.z, min.z);
      mesh_source->m_BoundingBox.Max.x =
          glm::max(mesh_source->m_BoundingBox.Max.x, max.x);
      mesh_source->m_BoundingBox.Max.y =
          glm::max(mesh_source->m_BoundingBox.Max.y, max.y);
      mesh_source->m_BoundingBox.Max.z =
          glm::max(mesh_source->m_BoundingBox.Max.z, max.z);
    }
  }

  // Bones
  if (mesh_source->HasSkeleton()) {
    mesh_source->m_BoneInfluences.resize(mesh_source->m_Vertices.size());
    for (uint32_t m = 0; m < scene->mNumMeshes; m++) {
      aiMesh* mesh = scene->mMeshes[m];
      auto& [BaseVertex, BaseIndex, MaterialIndex, IndexCount, VertexCount,
             Transform, LocalTransform, BoundingBox, NodeName, MeshName,
             IsRigged] = mesh_source->m_Submeshes[m];

      if (mesh->mNumBones > 0) {
        IsRigged = true;
        for (uint32_t i = 0; i < mesh->mNumBones; i++) {
          aiBone* bone = mesh->mBones[i];
          bool has_non_zero_weight = false;
          for (size_t j = 0; j < bone->mNumWeights; j++) {
            if (bone->mWeights[j].mWeight > 0.000001f) {
              has_non_zero_weight = true;
            }
          }
          if (!has_non_zero_weight) continue;

          // Find bone in skeleton
          uint32_t boneIndex =
              mesh_source->m_Skeleton->GetBoneIndex(bone->mName.C_Str());
          if (boneIndex == Skeleton::kNullIndex) {
            BE_CORE_ERROR_TAG("Animation",
                              "Could not find mesh bone '{}' in skeleton!",
                              bone->mName.C_Str());
          }

          uint32_t bone_info_index = ~0;
          for (size_t j = 0; j < mesh_source->m_BoneInfo.size(); ++j) {
            if ((mesh_source->m_BoneInfo[j].BoneIndex == boneIndex) &&
                (mesh_source->m_BoneInfo[j].SubMeshIndex == m)) {
              bone_info_index = j;
              break;
            }
          }
          if (bone_info_index == ~0) {
            bone_info_index = mesh_source->m_BoneInfo.size();
            mesh_source->m_BoneInfo.emplace_back(
                glm::inverse(Transform),
                utilities::Mat4FromAIMatrix4x4(bone->mOffsetMatrix), m,
                boneIndex);
          }

          for (size_t j = 0; j < bone->mNumWeights; j++) {
            int vertex_id = BaseVertex + bone->mWeights[j].mVertexId;
            float weight = bone->mWeights[j].mWeight;
            mesh_source->m_BoneInfluences[vertex_id].AddBoneData(
                bone_info_index, weight);
          }
        }
      }
    }

    for (auto& bone_influence : mesh_source->m_BoneInfluences) {
      bone_influence.NormalizeWeights();
    }
  }

  // Materials
  Ref<RendererTexture2D> white_texture = Renderer::GetWhiteTexture();
  if (scene->HasMaterials()) {
    mesh_source->m_Materials.resize(scene->mNumMaterials);

    for (uint32_t i = 0; i < scene->mNumMaterials; i++) {
      auto ai_material = scene->mMaterials[i];
      auto ai_material_name = ai_material->GetName();
      Ref<Material> mi =
          Material::Create(Renderer::GetShaderLibrary()->Get("HazelPBR_Static"),
                           ai_material_name.data);
      mesh_source->m_Materials[i] = mi;

      aiString ai_tex_path;

      glm::vec3 albedo_color(0.8f);
      float emission = 0.0f;
      aiColor3D ai_color, ai_emission;
      if (ai_material->Get(AI_MATKEY_COLOR_DIFFUSE, ai_color) == AI_SUCCESS)
        albedo_color = {ai_color.r, ai_color.g, ai_color.b};

      if (ai_material->Get(AI_MATKEY_COLOR_EMISSIVE, ai_emission) == AI_SUCCESS)
        emission = ai_emission.r;

      mi->Set("u_MaterialUniforms.AlbedoColor", albedo_color);
      mi->Set("u_MaterialUniforms.Emission", emission);

      float roughness, metalness;
      if (ai_material->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness) !=
          aiReturn_SUCCESS)
        roughness = 0.5f;  // Default value

      if (ai_material->Get(AI_MATKEY_REFLECTIVITY, metalness) !=
          aiReturn_SUCCESS)
        metalness = 0.0f;

      bool has_albedo_map = ai_material->GetTexture(aiTextureType_DIFFUSE, 0,
                                                    &ai_tex_path) == AI_SUCCESS;
      bool fallback = !has_albedo_map;
      if (has_albedo_map) {
        AssetHandle texture_handle = 0;
        TextureSpecification spec;
        spec.DebugName = ai_tex_path.C_Str();
        spec.SRGB = true;
        if (auto ai_tex_embedded =
                scene->GetEmbeddedTexture(ai_tex_path.C_Str())) {
          spec.Format = ImageFormat::RGBA;
          spec.Width = ai_tex_embedded->mWidth;
          spec.Height = ai_tex_embedded->mHeight;
          texture_handle =
              AssetManager::CreateMemoryOnlyRendererAsset<RendererTexture2D>(
                  spec, Buffer(ai_tex_embedded->pcData, 1));
        } else {
          auto parent_path = path_.parent_path();
          parent_path /= std::string(ai_tex_path.data);
          std::string texture_path = parent_path.string();
          texture_handle =
              AssetManager::CreateMemoryOnlyRendererAsset<RendererTexture2D>(
                  spec, texture_path);
        }

        if (auto texture =
                AssetManager::GetAsset<RendererTexture2D>(texture_handle);
            texture && texture->Loaded()) {
          mi->Set("u_AlbedoTexture", texture);
          mi->Set("u_MaterialUniforms.AlbedoColor", glm::vec3(1.0f));
        } else {
          BE_CORE_ERROR_TAG("Mesh", "Could not load texture: {0}",
                            ai_tex_path.C_Str());
          fallback = true;
        }
      }

      if (fallback) {
        mi->Set("u_AlbedoTexture", white_texture);
      }

      // Normal maps
      bool has_normal_map = ai_material->GetTexture(aiTextureType_NORMALS, 0,
                                                    &ai_tex_path) == AI_SUCCESS;
      fallback = !has_normal_map;
      if (has_normal_map) {
        AssetHandle texture_handle = 0;

        TextureSpecification spec;
        spec.DebugName = ai_tex_path.C_Str();
        if (auto ai_tex_embedded =
                scene->GetEmbeddedTexture(ai_tex_path.C_Str())) {
          spec.Format = ImageFormat::RGB;
          spec.Width = ai_tex_embedded->mWidth;
          spec.Height = ai_tex_embedded->mHeight;
          texture_handle =
              AssetManager::CreateMemoryOnlyRendererAsset<RendererTexture2D>(
                  spec, Buffer(ai_tex_embedded->pcData, 1));
        } else {
          auto parent_path = path_.parent_path();
          parent_path /= std::string(ai_tex_path.data);
          std::string texture_path = parent_path.string();
          texture_handle =
              AssetManager::CreateMemoryOnlyRendererAsset<RendererTexture2D>(
                  spec, texture_path);
        }

        if (auto texture =
                AssetManager::GetAsset<RendererTexture2D>(texture_handle);
            texture && texture->Loaded()) {
          mi->Set("u_NormalTexture", texture);
          mi->Set("u_MaterialUniforms.UseNormalMap", true);
        } else {
          BE_CORE_ERROR_TAG("Mesh", "    Could not load texture: {0}",
                            ai_tex_path.C_Str());
          fallback = true;
        }
      }

      if (fallback) {
        mi->Set("u_NormalTexture", white_texture);
        mi->Set("u_MaterialUniforms.UseNormalMap", false);
      }

      bool has_roughness_map =
          ai_material->GetTexture(aiTextureType_SHININESS, 0, &ai_tex_path) ==
          AI_SUCCESS;
      fallback = !has_roughness_map;
      if (has_roughness_map) {
        AssetHandle texture_handle = 0;
        TextureSpecification spec;
        spec.DebugName = ai_tex_path.C_Str();
        if (auto aiTexEmbedded =
                scene->GetEmbeddedTexture(ai_tex_path.C_Str())) {
          spec.Format = ImageFormat::RGB;
          spec.Width = aiTexEmbedded->mWidth;
          spec.Height = aiTexEmbedded->mHeight;
          texture_handle =
              AssetManager::CreateMemoryOnlyRendererAsset<RendererTexture2D>(
                  spec, Buffer(aiTexEmbedded->pcData, 1));
        } else {
          auto parent_path = path_.parent_path();
          parent_path /= std::string(ai_tex_path.data);
          std::string texture_path = parent_path.string();
          texture_handle =
              AssetManager::CreateMemoryOnlyRendererAsset<RendererTexture2D>(
                  spec, texture_path);
        }

        if (auto texture =
                AssetManager::GetAsset<RendererTexture2D>(texture_handle);
            texture && texture->Loaded()) {
          mi->Set("u_RoughnessTexture", texture);
          mi->Set("u_MaterialUniforms.Roughness", 1.0f);
        } else {
          BE_CORE_ERROR_TAG("Mesh", "    Could not load texture: {0}",
                            ai_tex_path.C_Str());
          fallback = true;
        }
      }

      if (fallback) {
        mi->Set("u_RoughnessTexture", white_texture);
        mi->Set("u_MaterialUniforms.Roughness", roughness);
      }
      bool metalness_texture_found = false;
      for (uint32_t p = 0; p < ai_material->mNumProperties; p++) {
        if (auto material_property = ai_material->mProperties[p];
            material_property->mType == aiPTI_String) {
          uint32_t strLength =
              *reinterpret_cast<uint32_t*>(material_property->mData);
          std::string str(material_property->mData + 4, strLength);

          if (std::string key = material_property->mKey.data;
              key == "$raw.ReflectionFactor|file") {
            AssetHandle texture_handle = 0;
            TextureSpecification spec;
            spec.DebugName = str;
            if (auto ai_tex_embedded = scene->GetEmbeddedTexture(str.data())) {
              spec.Format = ImageFormat::RGB;
              spec.Width = ai_tex_embedded->mWidth;
              spec.Height = ai_tex_embedded->mHeight;
              texture_handle = AssetManager::CreateMemoryOnlyRendererAsset<
                  RendererTexture2D>(spec, Buffer(ai_tex_embedded->pcData, 1));
            } else {
              auto parent_path = path_.parent_path();
              parent_path /= str;
              std::string texture_path = parent_path.string();
              texture_handle = AssetManager::CreateMemoryOnlyRendererAsset<
                  RendererTexture2D>(spec, texture_path);
            }

            if (auto texture =
                    AssetManager::GetAsset<RendererTexture2D>(texture_handle);
                texture && texture->Loaded()) {
              metalness_texture_found = true;
              mi->Set("u_MetalnessTexture", texture);
              mi->Set("u_MaterialUniforms.Metalness", 1.0f);
            } else {
              BE_CORE_ERROR_TAG("Mesh", "    Could not load texture: {0}", str);
            }
            break;
          }
        }
      }

      fallback = !metalness_texture_found;
      if (fallback) {
        mi->Set("u_MetalnessTexture", white_texture);
        mi->Set("u_MaterialUniforms.Metalness", metalness);
      }
    }
  } else {
    auto mi = Material::Create(
        Renderer::GetShaderLibrary()->Get("HazelPBR_Static"), "Hazel-Default");
    mi->Set("u_MaterialUniforms.AlbedoColor", glm::vec3(0.8f));
    mi->Set("u_MaterialUniforms.Emission", 0.0f);
    mi->Set("u_MaterialUniforms.Metalness", 0.0f);
    mi->Set("u_MaterialUniforms.Roughness", 0.8f);
    mi->Set("u_MaterialUniforms.UseNormalMap", false);

    mi->Set("u_AlbedoTexture", white_texture);
    mi->Set("u_MetalnessTexture", white_texture);
    mi->Set("u_RoughnessTexture", white_texture);
    mesh_source->m_Materials.push_back(mi);
  }

  if (!mesh_source->m_Vertices.empty())
    mesh_source->m_VertexBuffer =
        VertexBuffer::Create(mesh_source->m_Vertices.data(),
                             mesh_source->m_Vertices.size() * sizeof(Vertex));

  if (mesh_source->HasSkeleton()) {
    mesh_source->m_BoneInfluenceBuffer = VertexBuffer::Create(
        mesh_source->m_BoneInfluences.data(),
        mesh_source->m_BoneInfluences.size() * sizeof(BoneInfluence));
  }

  if (!mesh_source->m_Indices.empty())
    mesh_source->m_IndexBuffer =
        IndexBuffer::Create(mesh_source->m_Indices.data(),
                            mesh_source->m_Indices.size() * sizeof(Index));

  return mesh_source;
}

bool MeshImporter::ImportSkeleton(std::unique_ptr<Skeleton>& skeleton) const {
  Assimp::Importer importer;
  importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);

  const aiScene* scene = importer.ReadFile(path_.string(), kMeshImportFlags);
  if (!scene) {
    BE_CORE_ERROR_TAG("Animation",
                      "メッシュ ソース ファイルのロードに失敗しました : {0}",
                      path_.string());
    return false;
  }

  skeleton = AnimationImporter::ImportSkeleton(scene);
  return true;
}

bool MeshImporter::ImportAnimation(
    const uint32_t animation_index, const Skeleton& skeleton,
    const bool is_masked_root_motion, const glm::vec3& root_translation_mask,
    const float root_rotation_mask,
    std::unique_ptr<Animation>& animation) const {
  Assimp::Importer importer;
  importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);

  const aiScene* scene = importer.ReadFile(path_.string(), kMeshImportFlags);
  if (!scene) {
    BE_CORE_ERROR_TAG("Animation",
                      "メッシュ ソース ファイルのロードに失敗しました : {0}",
                      path_.string());
    return false;
  }

  if (animation_index >= scene->mNumAnimations) {
    BE_CORE_ERROR_TAG("Animation",
                      "アニメーション インデックス {0} がメッシュ ソース "
                      "ファイルの範囲外です : {1}",
                      animation_index, path_.string());
    return false;
  }

  animation = AnimationImporter::ImportAnimation(
      scene, animation_index, skeleton, is_masked_root_motion,
      root_translation_mask, root_rotation_mask);
  return true;
}

bool MeshImporter::IsCompatibleSkeleton(const uint32_t animation_index,
                                        const Skeleton& skeleton) const {
  Assimp::Importer importer;
  importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);

  const aiScene* scene = importer.ReadFile(path_.string(), kMeshImportFlags);
  if (!scene) {
    BE_CORE_ERROR_TAG("Mesh", "メッシュファイルのロードに失敗しました : {0}", path_.string());
    return false;
  }

  if (scene->mNumAnimations <= animation_index) {
    return false;
  }

  std::vector<std::string> animation_names;
  AnimationImporter::GetAnimationNames(scene, &animation_names);
  if (animation_names.empty()) {
    return false;
  }

  const aiAnimation* anim = scene->mAnimations[animation_index];

  std::unordered_map<std::string_view, uint32_t> boneIndices;
  for (uint32_t i = 0; i < skeleton.GetNumBones(); ++i) {
    boneIndices.emplace(skeleton.GetBoneName(i), i);
  }

  std::set<std::tuple<uint32_t, aiNodeAnim*>> validChannels;
  for (uint32_t channel_index = 0; channel_index < anim->mNumChannels;
       ++channel_index) {
    aiNodeAnim* node_anim = anim->mChannels[channel_index];
    if (auto it = boneIndices.find(node_anim->mNodeName.C_Str());
        it != boneIndices.end()) {
      validChannels.emplace(it->second, node_anim);
    }
  }
  return !validChannels.empty();
}

uint32_t MeshImporter::GetAnimationCount() const {
  Assimp::Importer importer;
  importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);

  const aiScene* scene = importer.ReadFile(path_.string(), kMeshImportFlags);
  if (!scene) {
    BE_CORE_ERROR_TAG("Mesh", "Failed to load mesh file: {0}", path_.string());
    return false;
  }

  return scene->mNumAnimations;
}

void MeshImporter::TraverseNodes(Ref<MeshSource> mesh_source, void* assimp_node,
                                 const uint32_t node_index,
                                 const glm::mat4& parent_transform,
                                 const uint32_t level) {
  const auto ai_node = static_cast<aiNode*>(assimp_node);

  auto& [Parent, Children, Submeshes, Name, LocalTransform] =
      mesh_source->m_Nodes[node_index];
  Name = ai_node->mName.C_Str();
  LocalTransform = utilities::Mat4FromAIMatrix4x4(ai_node->mTransformation);

  const glm::mat4 transform = parent_transform * LocalTransform;
  for (uint32_t i = 0; i < ai_node->mNumMeshes; i++) {
    uint32_t submesh_index = ai_node->mMeshes[i];
    auto& submesh = mesh_source->m_Submeshes[submesh_index];
    submesh.NodeName = ai_node->mName.C_Str();
    submesh.Transform = transform;
    submesh.LocalTransform = LocalTransform;

    Submeshes.push_back(submesh_index);
  }

  const uint32_t parent_node_index = mesh_source->m_Nodes.size() - 1;
  Children.resize(ai_node->mNumChildren);
  for (uint32_t i = 0; i < ai_node->mNumChildren; i++) {
    auto& [Parent, Children, Submeshes, Name, LocalTransform] =
        mesh_source->m_Nodes.emplace_back();
    const uint32_t child_index = mesh_source->m_Nodes.size() - 1;
    Parent = parent_node_index;
    mesh_source->m_Nodes[node_index].Children[i] = child_index;
    TraverseNodes(mesh_source, ai_node->mChildren[i], child_index, transform,
                  level + 1);
  }
}
}  // namespace base_engine
