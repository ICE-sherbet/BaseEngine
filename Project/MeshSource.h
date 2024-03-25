// @MeshSource.h
// @brief
// @author ICE
// @date 2024/02/26
//
// @details

#pragma once
#include <glm/glm.hpp>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>

#include "AABB.h"
#include "Animation.h"
#include "Bone.h"
#include "IndexBuffer.h"
#include "Material.h"
#include "Skeleton.h"
#include "SubMesh.h"
#include "VertexBuffer.h"

namespace base_engine {
struct Vertex {
  glm::vec3 Position;
  glm::vec3 Normal;
  glm::vec3 Tangent;
  glm::vec3 Binormal;
  glm::vec2 Texcoord;
};
static const int NumAttributes = 5;

struct Index {
  uint32_t V1, V2, V3;
};

struct Triangle {
  Vertex V0, V1, V2;

  Triangle(const Vertex& v0, const Vertex& v1, const Vertex& v2)
      : V0(v0), V1(v1), V2(v2) {}
};
struct MeshNode {
  uint32_t Parent = 0xffffffff;
  std::vector<uint32_t> Children;
  std::vector<uint32_t> Submeshes;

  std::string Name;
  glm::mat4 LocalTransform;

  inline bool IsRoot() const { return Parent == 0xffffffff; }
};
class MeshSource : public Asset {
 public:
  MeshSource() = default;
  MeshSource(const std::vector<Vertex>& vertices,
             const std::vector<Index>& indices, const glm::mat4& transform);
  MeshSource(const std::vector<Vertex>& vertices,
             const std::vector<Index>& indices,
             const std::vector<SubMesh>& submeshes);
  virtual ~MeshSource();

  void DumpVertexBuffer();

  std::vector<SubMesh>& GetSubmeshes() { return m_Submeshes; }
  const std::vector<SubMesh>& GetSubmeshes() const { return m_Submeshes; }

  const std::vector<Vertex>& GetVertices() const { return m_Vertices; }
  const std::vector<Index>& GetIndices() const { return m_Indices; }

  bool HasSkeleton() const { return (bool)m_Skeleton; }
  bool IsSubmeshRigged(uint32_t submeshIndex) const {
    return m_Submeshes[submeshIndex].IsRigged;
  }

  void SetSkeleton(std::unique_ptr<Skeleton> skeleton) const {
    m_Skeleton = std::move(skeleton);
  }

  const Skeleton& GetSkeleton() const {
    BE_CORE_ASSERT(m_Skeleton, "Attempted to access null skeleton!");
    return *m_Skeleton;
  }
  bool IsCompatibleSkeleton(const uint32_t animationIndex,
                            const Skeleton& skeleton) const;

  std::vector<std::string> GetAnimationNames() const;

  const Animation* GetAnimation(const uint32_t animationIndex,
                                const Skeleton& skeleton,
                                const bool isMaskedRootMotion,
                                const glm::vec3& rootTranslationMask,
                                float rootRotationMask) const;


  const std::vector<BoneInfluence>& GetBoneInfluences() const {
    return m_BoneInfluences;
  }

  std::vector<Ref<Material>>& GetMaterials() { return m_Materials; }
  const std::vector<Ref<Material>>& GetMaterials() const { return m_Materials; }
  const std::string& GetFilePath() const { return m_FilePath; }

  const std::vector<Triangle>& GetTriangleCache(uint32_t index) const
  {
    return m_TriangleCache.at(index);
  }

  Ref<VertexBuffer> GetVertexBuffer() { return m_VertexBuffer; }
  Ref<VertexBuffer> GetBoneInfluenceBuffer() { return m_BoneInfluenceBuffer; }
  Ref<IndexBuffer> GetIndexBuffer() { return m_IndexBuffer; }

  static AssetType GetStaticType() { return AssetType::kMeshSource; }
  AssetType GetAssetType() const override { return GetStaticType(); }
  const AABB& GetBoundingBox() const { return m_BoundingBox; }

  const MeshNode& GetRootNode() const { return m_Nodes[0]; }
  const std::vector<MeshNode>& GetNodes() const { return m_Nodes; }

 private:
  std::vector<SubMesh> m_Submeshes;

  Ref<VertexBuffer> m_VertexBuffer;
  Ref<VertexBuffer> m_BoneInfluenceBuffer;
  Ref<IndexBuffer> m_IndexBuffer;

  std::vector<Vertex> m_Vertices;
  std::vector<Index> m_Indices;

  std::vector<BoneInfluence> m_BoneInfluences;
  std::vector<BoneInfo> m_BoneInfo;
  mutable std::unique_ptr<Skeleton> m_Skeleton;
  std::vector<std::string> m_AnimationNames;
  mutable std::vector<std::unique_ptr<Animation>> m_Animations;

  std::vector<Ref<Material>> m_Materials;

  std::unordered_map<uint32_t, std::vector<Triangle>> m_TriangleCache;

  AABB m_BoundingBox;

  std::string m_FilePath;

  std::vector<MeshNode> m_Nodes;

  bool m_Runtime = false;
  friend class MeshImporter;
};
}  // namespace base_engine
