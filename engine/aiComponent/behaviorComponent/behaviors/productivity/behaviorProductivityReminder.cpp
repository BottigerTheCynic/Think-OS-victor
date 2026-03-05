/**
 * File: BehaviorProductivityReminder.cpp
 *
 * Author: BottigerTheCynic
 * Created: 2026-03-04
 *
 * Description: Hourly productivity reminder with voice response and reward
 *
 * Copyright: Anki, Inc. 2026
 *
 **/

#include "engine/aiComponent/behaviorComponent/behaviors/productivity/behaviorProductivityReminder.h"
#include "engine/aiComponent/behaviorComponent/userIntents.h"
#include "engine/aiComponent/behaviorComponent/behaviorExternalInterface/behaviorExternalInterface.h"
#include "engine/aiComponent/behaviorComponent/behaviorExternalInterface/beiRobotInfo.h"
#include "engine/faceWorld.h"
#include "engine/actions/animActions.h"
#include "engine/actions/sayTextAction.h"
#include "clad/types/animationTrigger.h"

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
  modifiers.wantsToBeActivatedWhenOnCharger  = true;
  modifiers.wantsToBeActivatedWhenOffTreads  = true;
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
  const auto& namedFaces = faceWorld.GetNamedFaceIDs();
  for (const auto& faceID : namedFaces) {
    std::string name;
    if (faceWorld.GetNameForFaceID(faceID, name) && !name.empty()) {
      return name;
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
      // Let user set a custom study session length via voice ("hey Vector, set a timer for 3 hours")
      auto& uic = GetBEI().GetUserIntentComponent();
      if (uic.IsUserIntentPending(USER_INTENT(set_timer))) {
        UserIntent_set_timer intentData;
        SmartActivateUserIntent(USER_INTENT(set_timer), intentData);
        _dVars.customIntervalSec = intentData.time_s;
        _dVars.secondsSinceLastReminder = 0.f;
        DelegateIfInControl(
          new SayTextAction("Got it! I'll check in with you when your time is up."),
          nullptr
        );
        break;
      }

      // Use custom time if set, otherwise fall back to JSON config default
      const float interval = (_dVars.customIntervalSec > 0.f)
        ? _dVars.customIntervalSec
        : _iConfig.reminderIntervalSec;

      _dVars.secondsSinceLastReminder += GetBEI().GetRobotInfo().GetDeltaTimeSeconds();
      if (_dVars.secondsSinceLastReminder >= interval) {
        _dVars.secondsSinceLastReminder = 0.f;
        TransitionToAskIfDone();
      }
      break;
    }

    case State::AskingIfDone:
    {
      auto& uic = GetBEI().GetUserIntentComponent();

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
    new TriggerAnimationAction(AnimationTrigger::ReactToHeldOnPalmPutDown),
    [this]() {
      DelegateIfInControl(
        new SayTextAction("Are you done with your work?"),
        [this]() {
          // Now listening — BehaviorUpdate() will catch the yes/no intent
        }
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
  _dVars.secondsSinceLastReminder = 0.f;
}

} // namespace Vector
} // namespace Anki