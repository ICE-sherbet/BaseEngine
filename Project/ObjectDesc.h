// @ObjectDesc.h
// @brief
// @author ICE
// @date 2024/03/23
// 
// @details

#pragma once
#include <cstdint>

struct ObjDesc {
  int txtOffset;             // Texture index offset in the array of textures
  uint64_t vertexAddress;    // Address of the Vertex buffer
  uint64_t indexAddress;     // Address of the index buffer
  uint64_t materialAddress;  // Address of the material buffer
  uint64_t
      materialIndexAddress;  // Address of the triangle material index buffer
};

struct WaveFrontMaterial {
  glm::vec3 ambient = glm::vec3(0.1f, 0.1f, 0.1f);
  glm::vec3 diffuse = glm::vec3(0.7f, 0.7f, 0.7f);
  glm::vec3 specular = glm::vec3(1.0f, 1.0f, 1.0f);
  glm::vec3 transmittance = glm::vec3(0.0f, 0.0f, 0.0f);
  glm::vec3 emission = glm::vec3(0.0f, 0.0f, 0.10);
  float shininess = 0.f;
  float ior = 1.0f;      // index of refraction
  float dissolve = 1.f;  // 1 == opaque; 0 == fully transparent
                         // illumination model (see
                         // http://www.fileformat.info/format/material/)
  int illum = 0;
  int textureID = -1;
};