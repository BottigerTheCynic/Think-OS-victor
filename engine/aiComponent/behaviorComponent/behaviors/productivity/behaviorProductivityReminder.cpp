/**
 * File: BehaviorProductivityReminder.cpp
 *
 * Author: bottiger
 * Created: 2026-03-04
 *
 * Description: Hourly productivity reminder with voice response and reward
 *
 * Copyright: Anki, Inc. 2026
 *
 **/

#include "engine/aiComponent/behaviorComponent/behaviors/productivity/behaviorProductivityReminder.h"
#include "engine/aiComponent/behaviorComponent/userIntents.h"
#include "engine/aiComponent/behaviorComponent/userIntentComponent.h"
#include "engine/aiComponent/behaviorComponent/behaviorExternalInterface/behaviorExternalInterface.h"
#include "engine/aiComponent/behaviorComponent/behaviorExternalInterface/beiRobotInfo.h"
#include "engine/faceWorld.h"
#include "engine/actions/animActions.h"
#include "engine/actions/sayTextAction.h"
#include "clad/types/animationTrigger.h"
#include "util/time/baseStationTimer.h"

namespace Anki {
namespace Vector {

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BehaviorProductivityReminder::InstanceConfig::InstanceConfig()
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BehaviorProductivityReminder::DynamicVariables::DynamicVariables()
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BehaviorProductivityReminder::BehaviorProductivityReminder(const Json::Value& config)
 : ICozmoBehavior(config)
{
  if (!config["reminderIntervalMinutes"].isNull()) {
    _iConfig.reminderIntervalSec = config["reminderIntervalMinutes"].asFloat() * 60.f;
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BehaviorProductivityReminder::~BehaviorProductivityReminder()
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BehaviorProductivityReminder::GetBehaviorOperationModifiers(BehaviorOperationModifiers& modifiers) const
{
  modifiers.wantsToBeActivatedWhenOnCharger = true;
  modifiers.wantsToBeActivatedWhenOffTreads = true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BehaviorProductivityReminder::GetAllDelegates(std::set<IBehavior*>& delegates) const
{
  // No sub-behaviors, only actions
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BehaviorProductivityReminder::GetBehaviorJsonKeys(std::set<const char*>& expectedKeys) const
{
  const char* list[] = {
    "reminderIntervalMinutes"
  };
  expectedKeys.insert(std::begin(list), std::end(list));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool BehaviorProductivityReminder::WantsToBeActivatedBehavior() const
{
  return true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BehaviorProductivityReminder::OnBehaviorActivated()
{
  _dVars = DynamicVariables();
  _dVars.startTime = BaseStationTimer::getInstance()->GetCurrentTimeInSeconds();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BehaviorProductivityReminder::OnBehaviorDeactivated()
{
  _dVars = DynamicVariables();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string BehaviorProductivityReminder::GetRegisteredName() const
{
  const auto& faceWorld = GetBEI().GetFaceWorld();
  const auto faceIDs = faceWorld.GetFaceIDs();
  for (const auto& faceID : faceIDs) {
    const auto* face = faceWorld.GetFace(faceID);
    if (face != nullptr && !face->GetName().empty()) {
      return face->GetName();
    }
  }
  return "";
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BehaviorProductivityReminder::BehaviorUpdate()
{
  if (!IsActivated()) {
    return;
  }

  if (IsControlDelegated()) {
    return;
  }

  switch (_dVars.state)
  {
    case State::Idle:
    {
      // Let user set a custom study session via voice ("hey Vector, set a timer for 3 hours")
      UserIntentComponent& uic = GetBehaviorComp<UserIntentComponent>();
      if (uic.IsUserIntentPending(USER_INTENT(set_timer))) {
        const UserIntentPtr intentData = SmartActivateUserIntent(USER_INTENT(set_timer));
        if (intentData) {
          _dVars.customIntervalSec = intentData->Get_set_timer().time_s;
        }
        _dVars.startTime = BaseStationTimer::getInstance()->GetCurrentTimeInSeconds();
        DelegateIfInControl(
          new SayTextAction("Got it! I'll check in with you when your time is up."),
          SimpleCallback()
        );
        break;
      }

      // Use custom time if set, otherwise use JSON config default
      const float interval = (_dVars.customIntervalSec > 0.f)
        ? _dVars.customIntervalSec
        : _iConfig.reminderIntervalSec;

      const float now = BaseStationTimer::getInstance()->GetCurrentTimeInSeconds();
      if ((now - _dVars.startTime) >= interval) {
        TransitionToAskIfDone();
      }
      break;
    }

    case State::AskingIfDone:
    {
      UserIntentComponent& uic = GetBehaviorComp<UserIntentComponent>();

      if (uic.IsUserIntentPending(USER_INTENT(imperative_affirmative))) {
        SmartActivateUserIntent(USER_INTENT(imperative_affirmative));
        TransitionToReward();
      }
      else if (uic.IsUserIntentPending(USER_INTENT(imperative_negative))) {
        SmartActivateUserIntent(USER_INTENT(imperative_negative));
        DelegateIfInControl(
          new SayTextAction("Okay, keep going! You've got this!"),
          [this]() { TransitionToIdle(); }
        );
      }
      break;
    }

    case State::Rewarding:
      // handled by delegate callbacks in TransitionToReward()
      break;
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BehaviorProductivityReminder::TransitionToAskIfDone()
{
  _dVars.state = State::AskingIfDone;

  DelegateIfInControl(
    new TriggerAnimationAction(AnimationTrigger::HeldOnPalmPutDownRelaxed),
    [this]() {
      DelegateIfInControl(
        new SayTextAction("Are you done with your work?"),
        SimpleCallback()
      );
    }
  );
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BehaviorProductivityReminder::TransitionToReward()
{
  _dVars.state = State::Rewarding;

  const std::string name = GetRegisteredName();
  const std::string rewardText = name.empty()
    ? "Good job! I'm proud of you, and remember to stay productive!"
    : "Good job, " + name + "! I'm proud of you, and remember to stay productive!";

  DelegateIfInControl(
    new TriggerAnimationAction(AnimationTrigger::GreetAfterLongTime),
    [this, rewardText]() {
      DelegateIfInControl(
        new SayTextAction(rewardText),
        [this]() { TransitionToIdle(); }
      );
    }
  );
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BehaviorProductivityReminder::TransitionToIdle()
{
  _dVars.state = State::Idle;
  _dVars.customIntervalSec = 0.f;
  _dVars.startTime = BaseStationTimer::getInstance()->GetCurrentTimeInSeconds();
}

} // namespace Vector
} // namespace Anki