#include "PhysicsWorld.h"

#include "PhysicsBody2D.h"
#include "PhysicsContact.h"
#include "PhysicsFixture.h"
#include "PhysicsWorldCallBack.h"

namespace base_engine::physics {
PhysicsWorld::PhysicsWorld(const PVec2& gravity) {
  gravity_ = gravity;

  contact_manager_.m_allocator = &block_allocator_;
}

PhysicsBody* PhysicsWorld::CreateBody(const BodyDef* def) {
  void* mem = block_allocator_.Allocate(sizeof(PhysicsBody));
  const auto b = new (mem) PhysicsBody(def, this);
  
  b->m_prev = nullptr;
  b->m_next = body_list_;
  if (body_list_) {
    body_list_->m_prev = b;
  }
  body_list_ = b;
  ++body_count_;

  return b;
}

void PhysicsWorld::DestroyBody(PhysicsBody* body) {
	const ContactEdge* ce = body->m_contactList;
  while (ce) {
	  const ContactEdge* ce0 = ce;
    ce = ce->next;
    contact_manager_.Destroy(ce0->contact);
  }
  body->m_contactList = nullptr;

  PhysicsFixture* f = body->m_fixtureList;
  while (f) {
    PhysicsFixture* f0 = f;
    f = f->m_next;

    f0->DestroyProxies(&contact_manager_.m_broadPhase);
    f0->Destroy(&block_allocator_);
    f0->~PhysicsFixture();
    block_allocator_.Free(f0, sizeof(PhysicsFixture));

    body->m_fixtureList = f;
    body->m_fixtureCount -= 1;
  }
  body->m_fixtureList = nullptr;
  body->m_fixtureCount = 0;

  // Remove world body list.
  if (body->m_prev) {
    body->m_prev->m_next = body->m_next;
  }

  if (body->m_next) {
    body->m_next->m_prev = body->m_prev;
  }

  if (body == body_list_) {
    body_list_ = body->m_next;
  }

  --body_count_;
  body->~PhysicsBody();
  block_allocator_.Free(body, sizeof(PhysicsBody));
}

void PhysicsWorld::Step(float timeStep) {
  if (new_contacts_) {
    contact_manager_.FindNewContacts();
    new_contacts_ = false;
  }
  { contact_manager_.Collide(); }
}

struct b2WorldQueryWrapper {
  bool QueryCallback(int32_t proxyId) {
    auto proxy =
        static_cast<PhysicsFixtureProxy*>(broadPhase->GetUserData(proxyId));
    return callback->ReportFixture(proxy->fixture);
  }

  const bp::BroadPhase* broadPhase;
  PhysicsQueryCallback* callback;
};

void PhysicsWorld::QueryAABB(PhysicsQueryCallback* callback,
                             const PhysicsAABB& aabb) const {
  b2WorldQueryWrapper wrapper{&contact_manager_.m_broadPhase, callback};
  wrapper.broadPhase = &contact_manager_.m_broadPhase;
  wrapper.callback = callback;
  contact_manager_.m_broadPhase.Query(&wrapper, aabb);
}

struct b2WorldRayCastWrapper {
  [[nodiscard]] float RayCastCallback(const PhysicsRayCastInput& input,
                                      const int32_t proxyId) const {
    void* userData = broadPhase->GetUserData(proxyId);
    const auto proxy = static_cast<PhysicsFixtureProxy*>(userData);
    PhysicsFixture* fixture = proxy->fixture;
    const int32_t index = proxy->childIndex;
    PhysicsRayCastOutput output{};

    if (fixture->RayCast(&output, input, index)) {
      const float fraction = output.fraction;
      const PVec2 point = (1.0f - fraction) * input.p1 + fraction * input.p2;
      return callback->ReportFixture(fixture, point, output.normal, fraction);
    }

    return input.maxFraction;
  }

  const bp::BroadPhase* broadPhase;
  PhysicsRayCastCallback* callback;
};

void PhysicsWorld::RayCast(PhysicsRayCastCallback* callback, const PVec2& point1,
                           const PVec2& point2) const
{
  b2WorldRayCastWrapper wrapper{&contact_manager_.m_broadPhase, callback};
  const PhysicsRayCastInput input{point1, point2,1.0f};
  contact_manager_.m_broadPhase.RayCast(&wrapper, input);
}

PhysicsWorld::~PhysicsWorld() {
  PhysicsBody* b = body_list_;
  while (b) {
    PhysicsBody* bNext = b->m_next;

    auto f = b->m_fixtureList;
    while (f) {
      auto fNext = f->m_next;
      f->m_proxyCount = 0;
      f->Destroy(&block_allocator_);
      f = fNext;
    }

    b = bNext;
  }
}
}  // namespace base_engine::physics
