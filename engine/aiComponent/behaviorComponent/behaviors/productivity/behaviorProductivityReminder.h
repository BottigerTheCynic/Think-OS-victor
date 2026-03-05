/**
 * File: BehaviorProductivityReminder.h
 *
 * Author: bottiger
 * Created: 2026-03-04
 *
 * Description: Hourly productivity reminder with voice response and reward
 *
 * Copyright: Anki, Inc. 2026
 *
 **/

#ifndef __Engine_AiComponent_BehaviorComponent_Behaviors_BehaviorProductivityReminder__
#define __Engine_AiComponent_BehaviorComponent_Behaviors_BehaviorProductivityReminder__
#pragma once

#include "engine/aiComponent/behaviorComponent/behaviors/iCozmoBehavior.h"
#include <string>

namespace Anki {
namespace Vector {

class BehaviorProductivityReminder : public ICozmoBehavior
{
public:
  virtual ~BehaviorProductivityReminder();

protected:

  // Enforce creation through BehaviorFactory
  friend class BehaviorFactory;
  explicit BehaviorProductivityReminder(const Json::Value& config);

  virtual void GetBehaviorOperationModifiers(BehaviorOperationModifiers& modifiers) const override;
  virtual void GetAllDelegates(std::set<IBehavior*>& delegates) const override;
  virtual void GetBehaviorJsonKeys(std::set<const char*>& expectedKeys) const override;

  virtual bool WantsToBeActivatedBehavior() const override;
  virtual void OnBehaviorActivated() override;
  virtual void OnBehaviorDeactivated() override;
  virtual void BehaviorUpdate() override;

private:

  enum class State {
    Idle,         // counting down timer
    AskingIfDone, // asked question, waiting for yes/no
    Rewarding,    // playing reward anim + TTS
  };

  struct InstanceConfig {
    InstanceConfig();
    float reminderIntervalSec = 3120.f; // default 52 minutes, overridable from JSON
  };

  struct DynamicVariables {
    DynamicVariables();
    float startTime = 0.f;         // wall time when current session started
    float customIntervalSec = 0.f; // 0 means use default from JSON config
    State state = State::Idle;
  };

  InstanceConfig _iConfig;
  DynamicVariables _dVars;

  void TransitionToAskIfDone();
  void TransitionToReward();
  void TransitionToIdle();
  std::string GetRegisteredName() const;
};

} // namespace Vector
} // namespace Anki

#endif // __Engine_AiComponent_BehaviorComponent_Behaviors_BehaviorProductivityReminder__