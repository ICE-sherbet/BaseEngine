// @PhysicsWorld.h
// @brief
// @author ICE
// @date 2022/10/17
//
// @details

#pragma once
#include "PhysicsBlockAllocator.h"
#include "PhysicsBody2D.h"
#include "PhysicsContactManager.h"
#include "PhysicsFixture.h"
#include "PhysicsVector2.h"
namespace base_engine::physics {
class PhysicsQueryCallback;
class PhysicsRayCastCallback;

struct BodyDef;

class PhysicsWorld {
 public:
  PhysicsWorld(const PVec2& gravity);

  ~PhysicsWorld();

  PhysicsBody* CreateBody(const BodyDef* def);

  void DestroyBody(PhysicsBody* body);

  void Step(float timeStep);

  template <class T>
  void DebugRender(T* renderer) {
    PhysicsBody* b = body_list_;
    while (b) {
      PhysicsBody* bNext = b->m_next;

      PhysicsFixture* f = b->m_fixtureList;
      while (f) {
        PhysicsFixture* fNext = f->m_next;
        renderer->Render(f);

        f = fNext;
      }

      b = bNext;
    }
  }

  /// Query the world for all fixtures that may overlap in AABB.
  /// @param callback User-implemented callback class.
  /// @param aabb query box.
  void QueryAABB(PhysicsQueryCallback* callback, const PhysicsAABB& aabb) const;

  /**
   * \brief レイのパスにあるすべてのフィクスチャのワールドをレイキャストします。
   * 開始点のシェイプは無視されます。
   * \param callback ユーザーが実装したコールバック
   * \param point1 レイの始点
   * \param point2 レイの終点
   */
  void RayCast(PhysicsRayCastCallback* callback, const PVec2& point1,
               const PVec2& point2) const;

  friend class PhysicsBody;
  PhysicsBlockAllocator block_allocator_;
  PhysicsContactManager contact_manager_;
  PhysicsBody* body_list_ = nullptr;
  int32_t body_count_ = 0;

  PVec2 gravity_{};
  bool new_contacts_;
};
}  // namespace base_engine::physics
