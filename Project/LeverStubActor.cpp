﻿#include "LeverStubActor.h"

#include "BaseEngineCore.h"
#include "Circle.h"
#include "CollisionComponent.h"
#include "CollisionLayer.h"
#include "ComponentServiceLocator.h"
#include "DrawOrder.h"
#include "ElectronicsPower.h"
#include "GimmickCreateHelper.h"
#include "GridPosition.h"
#include "GridSnapComponent.h"
#include "IBaseEngineTexture.h"
#include "LeverStubReceiver.h"
#include "LoadObjectParameter.h"
#include "MachineConst.h"
#include "ObjectTileMapComponent.h"
#include "ReceiverComponent.h"
#include "ResourceContainer.h"
#include "SignboardReceiver.h"
#include "StageConstitution.h"
#include "TexturePaths.h"
#include "TransmitterComponent.h"
using namespace base_engine;
using namespace draw_order;
using RC = ResourceContainer;
LeverStubActor::LeverStubActor(base_engine::Game* game) : Actor(game) {}

void LeverStubActor::Create(const LoadObject& object) {
  {
    constexpr auto cell = stage::kStageCellSize<Floating>;
    const auto rect = std::make_shared<Rect>(0, 0, cell.x, cell.y);

    const auto collision = new CollisionComponent(this);
    collision->SetShape(rect);
    collision->SetObjectFilter(kLeverObjectFilter);
    collision->SetTargetFilter(kLeverTargetFilter);
    collision->SetTrigger(true);
  }
  {
    GimmickCreateHelper::AnimationCreateKey(
        this, "Lever", object.object.color_type, true, 130);
  }
  { auto lever = new LeverStubComponent(this); }
  {
    const auto transmitter = new TransmitterComponent(this, 100);
    transmitter->Create<LeverStubTransmitter>(this);

    const auto receiver = new ReceiverComponent(this, 100);
    receiver->Create<LeverStubReceiver>(this);
  }
  {
    const auto grid = new grid::GridSnapComponent(this);
    grid->SetAutoSnap(grid::AutoSnap::No)
        .SetSnapGridPosition({object.object.x, object.object.y});
    ComponentServiceLocator::Instance()
        .Resolve<tile_map::ObjectTileMapComponent>()
        ->SetCell(object.object.x, object.object.y, 1);
  }
  SetName("LeverStubActor");
}

void LeverStubComponent::Action(base_engine::Actor*) {
  if (const auto lever = dynamic_cast<LeverStubActor*>(owner_); lever) {
    if (!lever->GetElectric()) return;
    const auto animation = owner_->GetComponent<MofSpriteAnimationComponent>();
    if (animation.expired()) return;
    if (!animation.lock()->IsEndMotion()) return;
    const Actor* target = lever->GetTarget();
    const auto receiver = target->GetComponent<ReceiverComponent>();
    const auto sender = owner_->GetComponent<TransmitterComponent>().lock();
    animation.lock()->Stop(true);
    animation.lock()->Play("Lever");
    sender->AddTarget(receiver);
  }
}

bool LeverStubComponent::CanInteractive(base_engine::Actor* actor) {
  const auto lever = dynamic_cast<LeverStubActor*>(owner_);
  if (!lever) return false;
  if (!lever->GetElectric()) return false;
  const auto animation = owner_->GetComponent<MofSpriteAnimationComponent>();
  if (animation.expired()) return false;
  if (!animation.lock()->IsEndMotion()) return false;
  return true;
}