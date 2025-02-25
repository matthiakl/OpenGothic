#pragma once

#include "abstracttrigger.h"

class World;

class TriggerWorldStart : public AbstractTrigger {
  public:
    TriggerWorldStart(Vob* parent, World& world, ZenLoad::zCVobData&& data, Flags flags);

    void onTrigger(const TriggerEvent& evt) override;
  };
