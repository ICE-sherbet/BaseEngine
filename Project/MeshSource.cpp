#include "MeshSource.h"

namespace base_engine {
MeshSource::MeshSource(const std::vector<Vertex>& vertices,
                       const std::vector<Index>& indices,
                       const glm::mat4& transform)
    : m_Vertices(vertices), m_Indices(indices) {
  handle_ = {};

  SubMesh submesh;
  submesh.BaseVertex = 0;
  submesh.BaseIndex = 0;
  submesh.IndexCount = indices.size() * 3u;
  submesh.Transform = transform;
  m_Submeshes.push_back(submesh);

  m_VertexBuffer = VertexBuffer::Create(m_Vertices.data(),
                                        m_Vertices.size() * sizeof(Vertex));
  m_IndexBuffer =
      IndexBuffer::Create(m_Indices.data(), m_Indices.size() * sizeof(Index));
}

MeshSource::MeshSource(const std::vector<Vertex>& vertices,
                       const std::vector<Index>& indices,
                       const std::vector<SubMesh>& submeshes)
    : m_Submeshes(submeshes), m_Vertices(vertices), m_Indices(indices) {
  handle_ = {};

  m_VertexBuffer = VertexBuffer::Create(
      m_Vertices.data(), (uint32_t)(m_Vertices.size() * sizeof(Vertex)));
  m_IndexBuffer = IndexBuffer::Create(
      m_Indices.data(), (uint32_t)(m_Indices.size() * sizeof(Index)));
}

MeshSource::~MeshSource() {}

void MeshSource::DumpVertexBuffer() {}

bool MeshSource::IsCompatibleSkeleton(const uint32_t animationIndex,
                                      const Skeleton& skeleton) const
{
  throw std::runtime_error("Not implemented");
}

std::vector<std::string> MeshSource::GetAnimationNames() const
{
  return m_AnimationNames;
}

const Animation* MeshSource::GetAnimation(const uint32_t animationIndex,
                                          const Skeleton& skeleton,
                                          const bool isMaskedRootMotion,
                                          const glm::vec3& rootTranslationMask,
                                          float rootRotationMask) const
{
  if (!m_Animations[animationIndex]) {
  }
  if (animationIndex < m_Animations.size()) {
    return m_Animations[animationIndex].get();
  }
  return nullptr;
}
}  // namespace base_engine
