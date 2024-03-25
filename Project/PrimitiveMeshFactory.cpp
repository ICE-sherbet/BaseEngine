#include "PrimitiveMeshFactory.h"

#include <soloud.h>

#include <numbers>

#include "AssetManager.h"
#include "MeshSource.h"
#include "StaticMesh.h"

namespace base_engine {
AssetHandle PrimitiveMeshFactory::CreateBox(const glm::vec3& size) {
  std::vector<Vertex> vertices;
  vertices.resize(8);
  const float half_x = size.x / 2.0f;
  const float half_y = size.y / 2.0f;
  const float half_z = size.z / 2.0f;
  vertices[0].Position = {-half_x, -half_y, half_z};
  vertices[1].Position = {half_x, -half_y, half_z};
  vertices[2].Position = {half_x, half_y, half_z};
  vertices[3].Position = {-half_x, half_y, half_z};
  vertices[4].Position = {-half_x, -half_y, -half_z};
  vertices[5].Position = {half_x, -half_y, -half_z};
  vertices[6].Position = {half_x, half_y, -half_z};
  vertices[7].Position = {-half_x, half_y, -half_z};

  vertices[0].Normal = {-1.0f, -1.0f, 1.0f};
  vertices[1].Normal = {1.0f, -1.0f, 1.0f};
  vertices[2].Normal = {1.0f, 1.0f, 1.0f};
  vertices[3].Normal = {-1.0f, 1.0f, 1.0f};
  vertices[4].Normal = {-1.0f, -1.0f, -1.0f};
  vertices[5].Normal = {1.0f, -1.0f, -1.0f};
  vertices[6].Normal = {1.0f, 1.0f, -1.0f};
  vertices[7].Normal = {-1.0f, 1.0f, -1.0f};

  std::vector<Index> indices;
  indices.resize(12);
  indices[0] = {0, 1, 2};
  indices[1] = {2, 3, 0};
  indices[2] = {1, 5, 6};
  indices[3] = {6, 2, 1};
  indices[4] = {7, 6, 5};
  indices[5] = {5, 4, 7};
  indices[6] = {4, 0, 3};
  indices[7] = {3, 7, 4};
  indices[8] = {4, 5, 1};
  indices[9] = {1, 0, 4};
  indices[10] = {3, 2, 6};
  indices[11] = {6, 7, 3};

  const AssetHandle mesh_source_handle =
      AssetManager::CreateMemoryOnlyAsset<MeshSource>(vertices, indices,
                                                      glm::mat4(1.0f));
  Ref<MeshSource> mesh_source =
      AssetManager::GetAsset<MeshSource>(mesh_source_handle);
  return AssetManager::CreateMemoryOnlyAsset<StaticMesh>(mesh_source);
}

AssetHandle PrimitiveMeshFactory::CreateSphere(const float radius,
                                               const int latitude_bands,
                                               const int longitude_bands) {
  std::vector<Vertex> vertices;
  std::vector<Index> indices;

  for (float latitude = 0.0f; latitude <= latitude_bands; latitude++) {
    const float theta = latitude * std::numbers::pi_v<float> / latitude_bands;
    const float sin_theta = glm::sin(theta);
    const float cos_theta = glm::cos(theta);

    for (float longitude = 0.0f; longitude <= longitude_bands; longitude++) {
      const float phi =
          longitude * 2.f * std::numbers::pi_v<float> / longitude_bands;
      const float sin_phi = glm::sin(phi);
      const float cos_phi = glm::cos(phi);

      Vertex vertex;
      vertex.Normal = {cos_phi * sin_theta, cos_theta, sin_phi * sin_theta};
      vertex.Position = {radius * vertex.Normal.x, radius * vertex.Normal.y,
                         radius * vertex.Normal.z};
      vertices.push_back(vertex);
    }
  }

  for (uint32_t latitude = 0; latitude < static_cast<uint32_t>(latitude_bands);
       latitude++) {
    for (uint32_t longitude = 0;
         longitude < static_cast<uint32_t>(longitude_bands); longitude++) {
      const uint32_t first =
          (latitude * (static_cast<uint32_t>(longitude_bands) + 1)) + longitude;
      const uint32_t second =
          first + static_cast<uint32_t>(longitude_bands) + 1;

      indices.push_back({first, second, first + 1});
      indices.push_back({second, second + 1, first + 1});
    }
  }

  const AssetHandle mesh_source_handle =
      AssetManager::CreateMemoryOnlyAsset<MeshSource>(vertices, indices,
                                                      glm::mat4(1.0f));
  Ref<MeshSource> mesh_source =
      AssetManager::GetAsset<MeshSource>(mesh_source_handle);
  return AssetManager::CreateMemoryOnlyAsset<StaticMesh>(mesh_source);
}

static void CalculateRing(size_t segments, float radius, float y, float dy,
                          float height, float actualRadius,
                          std::vector<Vertex>& vertices) {
  float segIncr = 1.0f / (float)(segments - 1);
  for (size_t s = 0; s < segments; s++) {
    const float x =
        glm::cos(std::numbers::pi_v<float> * 2 * s * segIncr) * radius;
    const float z =
        glm::sin(std::numbers::pi_v<float> * 2 * s * segIncr) * radius;

    auto& [Position, Normal, Tangent, Binormal, Texcoord] =
        vertices.emplace_back();
    Position = glm::vec3(actualRadius * x, actualRadius * y + height * dy,
                         actualRadius * z);
  }
}

AssetHandle PrimitiveMeshFactory::CreateCapsule(const float radius,
                                                const float height) {
  constexpr size_t subdivisions_height = 8;
  constexpr size_t rings_body = subdivisions_height + 1;
  constexpr size_t rings_total = subdivisions_height + rings_body;
  constexpr size_t num_segments = 12;
  constexpr float radius_modifier = 0.021f;

  std::vector<Vertex> vertices;
  std::vector<Index> indices;

  vertices.reserve(num_segments * rings_total);
  indices.reserve((num_segments - 1) * (rings_total - 1) * 2);

  constexpr float body_incr = 1.0f / static_cast<float>(rings_body - 1);
  constexpr float ring_incr =
      1.0f / static_cast<float>(subdivisions_height - 1);

  for (int r = 0; r < subdivisions_height / 2; r++)
    CalculateRing(num_segments,
                  glm::sin(std::numbers::pi_v<float> * r * ring_incr),
                  glm::sin(std::numbers::pi_v<float> * (r * ring_incr - 0.5f)),
                  -0.5f, height, radius + radius_modifier, vertices);

  for (int r = 0; r < rings_body; r++)
    CalculateRing(num_segments, 1.0f, 0.0f, r * body_incr - 0.5f, height,
                  radius + radius_modifier, vertices);

  for (int r = subdivisions_height / 2; r < subdivisions_height; r++)
    CalculateRing(num_segments,
                  glm::sin(std::numbers::pi_v<float> * r * ring_incr),
                  glm::sin(std::numbers::pi_v<float> * (r * ring_incr - 0.5f)),
                  0.5f, height, radius + radius_modifier, vertices);

  for (int r = 0; r < rings_total - 1; r++) {
    for (int s = 0; s < num_segments - 1; s++) {
      auto& [V11, V12, V13] = indices.emplace_back();
      V11 = (r * num_segments + s + 1);
      V12 = (r * num_segments + s + 0);
      V13 = ((r + 1) * num_segments + s + 1);

      auto& [V21, V22, V23] = indices.emplace_back();
      V21 = ((r + 1) * num_segments + s + 0);
      V22 = ((r + 1) * num_segments + s + 1);
      V23 = (r * num_segments + s);
    }
  }

  const AssetHandle mesh_source_handle =
      AssetManager::CreateMemoryOnlyAsset<MeshSource>(vertices, indices,
                                                      glm::mat4(1.0f));
  Ref<MeshSource> mesh_source =
      AssetManager::GetAsset<MeshSource>(mesh_source_handle);
  return AssetManager::CreateMemoryOnlyAsset<StaticMesh>(mesh_source);
}
}  // namespace base_engine
