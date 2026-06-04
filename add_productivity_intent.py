#!/usr/bin/env python3
"""
Adds productivity_session UserIntent and wires ProductivityReminder into the
behavior tree. Run from repo root:

    python3 add_productivity_intent.py

Patches these files:
  clad/src/clad/types/behaviorComponent/userIntent.clad
  generated/clad/engine/clad/types/behaviorComponent/userIntentTag.h
  generated/clad/engine/clad/types/behaviorComponent/userIntent.h
  generated/clad/engine/clad/types/behaviorComponent/userIntent.cpp
  generated/clad/engine/clad/types/behaviorComponent/userIntent_declarations.def
  generated/clad/engine/clad/types/behaviorComponent/userIntent_switch.def
  generated/cladPython/clad/types/behaviorComponent/userIntent.py
  generated/cladCSharp/clad/types/behaviorComponent/userIntent.cs
  resources/config/engine/behaviorComponent/user_intent_map.json
  resources/config/engine/behaviorComponent/behaviors/productivity/behaviorProductivityReminder.json
  engine/aiComponent/behaviorComponent/behaviors/productivity/behaviorProductivityReminder.cpp
  resources/config/engine/behaviorComponent/behaviors/victorBehaviorTree/globalInterruptions.json
"""

import re, sys

PROD_NUM = 70  # After test_timeWithUnits (69); INVALID is explicit at 255 so no conflict.

# ── helpers ──────────────────────────────────────────────────────────────────

def read(path):
    with open(path, encoding='utf-8') as f:
        return f.read()

def write(path, content):
    with open(path, 'w', encoding='utf-8') as f:
        f.write(content)
    print(f"  ✓  {path}")

def multi_patch(path, *pairs):
    """Apply multiple (old, new) string replacements to a single file."""
    c = read(path)
    if 'productivity_session' in c:
        print(f"  ↷  skip (already done): {path}")
        return
    changed = False
    for old, new in pairs:
        if old in c:
            c = c.replace(old, new)
            changed = True
        else:
            print(f"  ✗  anchor not found in {path}")
            print(f"     {repr(old[:90])}")
    if changed:
        write(path, c)

# ── 1. CLAD source ────────────────────────────────────────────────────────────
print("[1/12] CLAD source")
multi_patch(
    'clad/src/clad/types/behaviorComponent/userIntent.clad',
    (
        '  UserIntent_Test_TimeWithUnits test_timeWithUnits,\n}',
        '  UserIntent_Test_TimeWithUnits test_timeWithUnits,\n  UserIntent_Void             productivity_session,\n}'
    ),
)

# ── 2. C++ tag enum ───────────────────────────────────────────────────────────
print("[2/12] userIntentTag.h")
multi_patch(
    'generated/clad/engine/clad/types/behaviorComponent/userIntentTag.h',
    (
        '  test_timeWithUnits,                 // 69\n  INVALID',
        f'  test_timeWithUnits,                 // 69\n  productivity_session,               // {PROD_NUM}\n  INVALID'
    ),
)

# ── 3. userIntent.h (TagToType, union member, method decls) ───────────────────
print("[3/12] userIntent.h")
multi_patch(
    'generated/clad/engine/clad/types/behaviorComponent/userIntent.h',
    # TagToType specialization
    (
        'template<>\nstruct UserIntent_TagToType<UserIntentTag::explore_start> {\n  using type = Anki::Vector::UserIntent_Void;\n};',
        'template<>\nstruct UserIntent_TagToType<UserIntentTag::explore_start> {\n  using type = Anki::Vector::UserIntent_Void;\n};\ntemplate<>\nstruct UserIntent_TagToType<UserIntentTag::productivity_session> {\n  using type = Anki::Vector::UserIntent_Void;\n};'
    ),
    # Union member variable
    (
        '    Anki::Vector::UserIntent_Void _explore_start;\n    Anki::Vector::UserIntent_GlobalStop _global_stop;',
        '    Anki::Vector::UserIntent_Void _explore_start;\n    Anki::Vector::UserIntent_Void _productivity_session;\n    Anki::Vector::UserIntent_GlobalStop _global_stop;'
    ),
    # Method declarations (insert new block between explore_start and global_stop)
    (
        '  /** explore_start **/\n  static UserIntent Createexplore_start(Anki::Vector::UserIntent_Void&& new_explore_start);\n  const Anki::Vector::UserIntent_Void& Get_explore_start() const;\n  void Set_explore_start(const Anki::Vector::UserIntent_Void& new_explore_start);\n  void Set_explore_start(Anki::Vector::UserIntent_Void&& new_explore_start);\n\n  /** global_stop **/\n',
        '  /** explore_start **/\n  static UserIntent Createexplore_start(Anki::Vector::UserIntent_Void&& new_explore_start);\n  const Anki::Vector::UserIntent_Void& Get_explore_start() const;\n  void Set_explore_start(const Anki::Vector::UserIntent_Void& new_explore_start);\n  void Set_explore_start(Anki::Vector::UserIntent_Void&& new_explore_start);\n\n  /** productivity_session **/\n  static UserIntent Createproductivity_session(Anki::Vector::UserIntent_Void&& new_productivity_session);\n  const Anki::Vector::UserIntent_Void& Get_productivity_session() const;\n  void Set_productivity_session(const Anki::Vector::UserIntent_Void& new_productivity_session);\n  void Set_productivity_session(Anki::Vector::UserIntent_Void&& new_productivity_session);\n\n  /** global_stop **/\n'
    ),
)

# ── 4. userIntent.cpp (many switch cases + method impls) ──────────────────────
print("[4/12] userIntent.cpp")
multi_patch(
    'generated/clad/engine/clad/types/behaviorComponent/userIntent.cpp',
    # copy-construct + copy-assign (same pattern, both get patched)
    (
        '  case Tag::explore_start:\n    new(&(this->_explore_start)) Anki::Vector::UserIntent_Void(other._explore_start);\n    break;\n  case Tag::global_stop:\n    new(&(this->_global_stop)) Anki::Vector::UserIntent_GlobalStop(other._global_stop);',
        '  case Tag::explore_start:\n    new(&(this->_explore_start)) Anki::Vector::UserIntent_Void(other._explore_start);\n    break;\n  case Tag::productivity_session:\n    new(&(this->_productivity_session)) Anki::Vector::UserIntent_Void(other._productivity_session);\n    break;\n  case Tag::global_stop:\n    new(&(this->_global_stop)) Anki::Vector::UserIntent_GlobalStop(other._global_stop);'
    ),
    # move-construct + move-assign
    (
        '  case Tag::explore_start:\n    new(&(this->_explore_start)) Anki::Vector::UserIntent_Void(std::move(other._explore_start));\n    break;\n  case Tag::global_stop:\n    new(&(this->_global_stop)) Anki::Vector::UserIntent_GlobalStop(std::move(other._global_stop));',
        '  case Tag::explore_start:\n    new(&(this->_explore_start)) Anki::Vector::UserIntent_Void(std::move(other._explore_start));\n    break;\n  case Tag::productivity_session:\n    new(&(this->_productivity_session)) Anki::Vector::UserIntent_Void(std::move(other._productivity_session));\n    break;\n  case Tag::global_stop:\n    new(&(this->_global_stop)) Anki::Vector::UserIntent_GlobalStop(std::move(other._global_stop));'
    ),
    # Unpack
    (
        '  case Tag::explore_start:\n    if (newTag != oldTag) {\n      new(&(this->_explore_start)) Anki::Vector::UserIntent_Void(buffer);\n    }\n    else {\n      this->_explore_start.Unpack(buffer);\n    }\n    break;\n  case Tag::global_stop:',
        '  case Tag::explore_start:\n    if (newTag != oldTag) {\n      new(&(this->_explore_start)) Anki::Vector::UserIntent_Void(buffer);\n    }\n    else {\n      this->_explore_start.Unpack(buffer);\n    }\n    break;\n  case Tag::productivity_session:\n    if (newTag != oldTag) {\n      new(&(this->_productivity_session)) Anki::Vector::UserIntent_Void(buffer);\n    }\n    else {\n      this->_productivity_session.Unpack(buffer);\n    }\n    break;\n  case Tag::global_stop:'
    ),
    # Pack
    (
        '  case Tag::explore_start:\n    this->_explore_start.Pack(buffer);\n    break;\n  case Tag::global_stop:\n    this->_global_stop.Pack(buffer);',
        '  case Tag::explore_start:\n    this->_explore_start.Pack(buffer);\n    break;\n  case Tag::productivity_session:\n    this->_productivity_session.Pack(buffer);\n    break;\n  case Tag::global_stop:\n    this->_global_stop.Pack(buffer);'
    ),
    # Size
    (
        '  case Tag::explore_start:\n    result += this->_explore_start.Size(); // UserIntent_Void\n    break;\n  case Tag::global_stop:\n    result += this->_global_stop.Size(); // UserIntent_GlobalStop',
        '  case Tag::explore_start:\n    result += this->_explore_start.Size(); // UserIntent_Void\n    break;\n  case Tag::productivity_session:\n    result += this->_productivity_session.Size(); // UserIntent_Void\n    break;\n  case Tag::global_stop:\n    result += this->_global_stop.Size(); // UserIntent_GlobalStop'
    ),
    # Equality
    (
        '  case Tag::explore_start:\n    return this->_explore_start == other._explore_start;\n  case Tag::global_stop:\n    return this->_global_stop == other._global_stop;',
        '  case Tag::explore_start:\n    return this->_explore_start == other._explore_start;\n  case Tag::productivity_session:\n    return this->_productivity_session == other._productivity_session;\n  case Tag::global_stop:\n    return this->_global_stop == other._global_stop;'
    ),
    # GetJSON
    (
        '  case Tag::explore_start:\n    root = this->_explore_start.GetJSON();\n    root["type"] = "explore_start";\n    break;\n  case Tag::global_stop:\n    root = this->_global_stop.GetJSON();',
        '  case Tag::explore_start:\n    root = this->_explore_start.GetJSON();\n    root["type"] = "explore_start";\n    break;\n  case Tag::productivity_session:\n    root = this->_productivity_session.GetJSON();\n    root["type"] = "productivity_session";\n    break;\n  case Tag::global_stop:\n    root = this->_global_stop.GetJSON();'
    ),
    # SetFromJSON
    (
        '      else if(tagStr == "explore_start") {\n        new(&(this->_explore_start)) Anki::Vector::UserIntent_Void;\n        result = this->_explore_start.SetFromJSON(json);\n        _tag = Tag::explore_start;\n      }\n      else if(tagStr == "global_stop") {',
        '      else if(tagStr == "explore_start") {\n        new(&(this->_explore_start)) Anki::Vector::UserIntent_Void;\n        result = this->_explore_start.SetFromJSON(json);\n        _tag = Tag::explore_start;\n      }\n      else if(tagStr == "productivity_session") {\n        new(&(this->_productivity_session)) Anki::Vector::UserIntent_Void;\n        result = this->_productivity_session.SetFromJSON(json);\n        _tag = Tag::productivity_session;\n      }\n      else if(tagStr == "global_stop") {'
    ),
    # Destructor
    (
        '  case Tag::explore_start:\n    _explore_start.~UserIntent_Void();\n    break;\n  case Tag::global_stop:\n    _global_stop.~UserIntent_GlobalStop();',
        '  case Tag::explore_start:\n    _explore_start.~UserIntent_Void();\n    break;\n  case Tag::productivity_session:\n    _productivity_session.~UserIntent_Void();\n    break;\n  case Tag::global_stop:\n    _global_stop.~UserIntent_GlobalStop();'
    ),
    # ToString
    (
        '  case UserIntentTag::explore_start:\n    return "explore_start";\n  case UserIntentTag::global_stop:\n    return "global_stop";',
        '  case UserIntentTag::explore_start:\n    return "explore_start";\n  case UserIntentTag::productivity_session:\n    return "productivity_session";\n  case UserIntentTag::global_stop:\n    return "global_stop";'
    ),
    # Create + Get method impls
    (
        'UserIntent UserIntent::Createexplore_start(Anki::Vector::UserIntent_Void&& new_explore_start)\n{\n  UserIntent m;\n  m.Set_explore_start(new_explore_start);\n  return m;\n}\n\nconst Anki::Vector::UserIntent_Void& UserIntent::Get_explore_start() const\n{\n  assert(_tag == Tag::explore_start);\n  return this->_explore_start;\n}',
        'UserIntent UserIntent::Createexplore_start(Anki::Vector::UserIntent_Void&& new_explore_start)\n{\n  UserIntent m;\n  m.Set_explore_start(new_explore_start);\n  return m;\n}\n\nconst Anki::Vector::UserIntent_Void& UserIntent::Get_explore_start() const\n{\n  assert(_tag == Tag::explore_start);\n  return this->_explore_start;\n}\n\nUserIntent UserIntent::Createproductivity_session(Anki::Vector::UserIntent_Void&& new_productivity_session)\n{\n  UserIntent m;\n  m.Set_productivity_session(new_productivity_session);\n  return m;\n}\n\nconst Anki::Vector::UserIntent_Void& UserIntent::Get_productivity_session() const\n{\n  assert(_tag == Tag::productivity_session);\n  return this->_productivity_session;\n}'
    ),
    # Set_ const ref impl
    (
        'void UserIntent::Set_explore_start(const Anki::Vector::UserIntent_Void& new_explore_start)\n{\n  if(this->_tag == Tag::explore_start) {\n    this->_explore_start = new_explore_start;\n  }\n  else {\n    ClearCurrent();\n    new(&this->_explore_start) Anki::Vector::UserIntent_Void(new_explore_start);\n    _tag = Tag::explore_start;\n  }\n}',
        'void UserIntent::Set_explore_start(const Anki::Vector::UserIntent_Void& new_explore_start)\n{\n  if(this->_tag == Tag::explore_start) {\n    this->_explore_start = new_explore_start;\n  }\n  else {\n    ClearCurrent();\n    new(&this->_explore_start) Anki::Vector::UserIntent_Void(new_explore_start);\n    _tag = Tag::explore_start;\n  }\n}\n\nvoid UserIntent::Set_productivity_session(const Anki::Vector::UserIntent_Void& new_productivity_session)\n{\n  if(this->_tag == Tag::productivity_session) {\n    this->_productivity_session = new_productivity_session;\n  }\n  else {\n    ClearCurrent();\n    new(&this->_productivity_session) Anki::Vector::UserIntent_Void(new_productivity_session);\n    _tag = Tag::productivity_session;\n  }\n}'
    ),
    # Template Get_ specialization
    (
        'template<>\nconst Anki::Vector::UserIntent_Void& UserIntent::Get_<UserIntent::Tag::explore_start>() const\n{\n  assert(_tag == Tag::explore_start);\n  return this->_explore_start;\n}',
        'template<>\nconst Anki::Vector::UserIntent_Void& UserIntent::Get_<UserIntent::Tag::explore_start>() const\n{\n  assert(_tag == Tag::explore_start);\n  return this->_explore_start;\n}\n\ntemplate<>\nconst Anki::Vector::UserIntent_Void& UserIntent::Get_<UserIntent::Tag::productivity_session>() const\n{\n  assert(_tag == Tag::productivity_session);\n  return this->_productivity_session;\n}'
    ),
    # Template Create_ specialization
    (
        'template<>\nUserIntent UserIntent::Create_<UserIntent::Tag::explore_start>(Anki::Vector::UserIntent_Void member)\n{\n  return Createexplore_start(std::move(member));\n}',
        'template<>\nUserIntent UserIntent::Create_<UserIntent::Tag::explore_start>(Anki::Vector::UserIntent_Void member)\n{\n  return Createexplore_start(std::move(member));\n}\n\ntemplate<>\nUserIntent UserIntent::Create_<UserIntent::Tag::productivity_session>(Anki::Vector::UserIntent_Void member)\n{\n  return Createproductivity_session(std::move(member));\n}'
    ),
    # Set_ rvalue ref impl
    (
        'void UserIntent::Set_explore_start(Anki::Vector::UserIntent_Void&& new_explore_start)\n{\n  if (this->_tag == Tag::explore_start) {\n    this->_explore_start = std::move(new_explore_start);\n  }\n  else {\n    ClearCurrent();\n    new(&this->_explore_start) Anki::Vector::UserIntent_Void(std::move(new_explore_start));\n    _tag = Tag::explore_start;\n  }\n}',
        'void UserIntent::Set_explore_start(Anki::Vector::UserIntent_Void&& new_explore_start)\n{\n  if (this->_tag == Tag::explore_start) {\n    this->_explore_start = std::move(new_explore_start);\n  }\n  else {\n    ClearCurrent();\n    new(&this->_explore_start) Anki::Vector::UserIntent_Void(std::move(new_explore_start));\n    _tag = Tag::explore_start;\n  }\n}\n\nvoid UserIntent::Set_productivity_session(Anki::Vector::UserIntent_Void&& new_productivity_session)\n{\n  if (this->_tag == Tag::productivity_session) {\n    this->_productivity_session = std::move(new_productivity_session);\n  }\n  else {\n    ClearCurrent();\n    new(&this->_productivity_session) Anki::Vector::UserIntent_Void(std::move(new_productivity_session));\n    _tag = Tag::productivity_session;\n  }\n}'
    ),
)

# ── 5. _declarations.def ──────────────────────────────────────────────────────
print("[5/12] userIntent_declarations.def")
multi_patch(
    'generated/clad/engine/clad/types/behaviorComponent/userIntent_declarations.def',
    (
        'void Process_explore_start(const Anki::Vector::UserIntent_Void& msg);\nvoid Process_global_stop(',
        'void Process_explore_start(const Anki::Vector::UserIntent_Void& msg);\nvoid Process_productivity_session(const Anki::Vector::UserIntent_Void& msg);\nvoid Process_global_stop('
    ),
)

# ── 6. _switch.def ────────────────────────────────────────────────────────────
print("[6/12] userIntent_switch.def")
multi_patch(
    'generated/clad/engine/clad/types/behaviorComponent/userIntent_switch.def',
    (
        'case Anki::Vector::UserIntent::Tag::explore_start:\n  Process_explore_start(msg.Get_explore_start());\n  break;\ncase Anki::Vector::UserIntent::Tag::global_stop:',
        'case Anki::Vector::UserIntent::Tag::explore_start:\n  Process_explore_start(msg.Get_explore_start());\n  break;\ncase Anki::Vector::UserIntent::Tag::productivity_session:\n  Process_productivity_session(msg.Get_productivity_session());\n  break;\ncase Anki::Vector::UserIntent::Tag::global_stop:'
    ),
)

# ── 7. Python enum ────────────────────────────────────────────────────────────
print("[7/12] userIntent.py")
py = read('generated/cladPython/clad/types/behaviorComponent/userIntent.py')
m = re.search(r'(    test_timeWithUnits\s*=\s*\d+[^\n]*\n)', py)
if m and 'productivity_session' not in py:
    twu_line = m.group(1)
    py_num = int(re.search(r'=\s*(\d+)', twu_line).group(1)) + 1
    write(
        'generated/cladPython/clad/types/behaviorComponent/userIntent.py',
        py.replace(twu_line, twu_line + f'    productivity_session         = {py_num} # Anki.Vector.UserIntent_Void\n')
    )
elif 'productivity_session' in py:
    print("  ↷  skip (already done)")
else:
    print("  ✗  test_timeWithUnits not found in Python file")

# ── 8. C# enum ────────────────────────────────────────────────────────────────
print("[8/12] userIntent.cs")
cs = read('generated/cladCSharp/clad/types/behaviorComponent/userIntent.cs')
m = re.search(r'(    test_timeWithUnits\s*=\s*\d+[^\n]*\n)', cs)
if m and 'productivity_session' not in cs:
    twu_line = m.group(1)
    cs_num = int(re.search(r'=\s*(\d+)', twu_line).group(1)) + 1
    write(
        'generated/cladCSharp/clad/types/behaviorComponent/userIntent.cs',
        cs.replace(twu_line, twu_line + f'    productivity_session = {cs_num},\n')
    )
elif 'productivity_session' in cs:
    print("  ↷  skip (already done)")
else:
    print("  ✗  test_timeWithUnits not found in C# file")

# ── 9. user_intent_map.json ───────────────────────────────────────────────────
print("[9/12] user_intent_map.json")
INTENT_MAP = 'resources/config/engine/behaviorComponent/user_intent_map.json'
multi_patch(
    INTENT_MAP,
    # Remove the simple_voice_response entry you added before
    (
        '    {\n      "cloud_intent": "intent_productivity_reminder",\n      "response": {\n        "anim_group": "GreetAfterLongTime",\n        "emotion_event": "RespondToShortVoiceCommand",\n        "active_feature": "ReactToHello",\n        "disable_wakeword_turn": true\n      }\n    },\n',
        ''
    ),
    # Add proper user_intent mapping (near explore_start for grouping)
    (
        '    {\n      "user_intent": "explore_start",\n      "cloud_intent": "intent_explore_start",',
        '    {\n      "cloud_intent": "intent_productivity_reminder",\n      "user_intent": "productivity_session"\n    },\n    {\n      "user_intent": "explore_start",\n      "cloud_intent": "intent_explore_start",'
    ),
)

# ── 10. behaviorProductivityReminder.json ─────────────────────────────────────
print("[10/12] behaviorProductivityReminder.json")
multi_patch(
    'resources/config/engine/behaviorComponent/behaviors/productivity/behaviorProductivityReminder.json',
    (
        '{"type": "set_timer"}',
        '{"type": "productivity_session"}'
    ),
)

# ── 11. behaviorProductivityReminder.cpp ──────────────────────────────────────
print("[11/12] behaviorProductivityReminder.cpp")
multi_patch(
    'engine/aiComponent/behaviorComponent/behaviors/productivity/behaviorProductivityReminder.cpp',
    (
        'USER_INTENT(set_timer)',
        'USER_INTENT(productivity_session)'
    ),
    (
        'intentData->intent.Get_set_timer().time_s',
        'intentData->intent.Get_productivity_session()'
        # UserIntent_Void has no time_s — we handle this below
    ),
)
# ProductivityReminder no longer reads time_s (it's a Void intent with no parameters).
# The custom interval must now be set via a separate mechanism or removed.
# For now we just remove the time capture since productivity_session carries no data.
multi_patch(
    'engine/aiComponent/behaviorComponent/behaviors/productivity/behaviorProductivityReminder.cpp',
    (
        '    float timerSec = 0.f;\n    const UserIntentData* intentData = uic.GetPendingUserIntent();\n    if (intentData != nullptr) {\n      timerSec = intentData->intent.Get_productivity_session();\n    }\n    SmartActivateUserIntent(USER_INTENT(productivity_session));\n    if (timerSec > 0.f) {\n      sCustomIntervalSec = timerSec;\n      _dVars.customIntervalSec = sCustomIntervalSec;\n      // Restart the session clock from this moment\n      sSessionStartTime = BaseStationTimer::getInstance()->GetCurrentTimeInSeconds();\n      _dVars.startTime = sSessionStartTime;\n    }',
        '    // productivity_session is a Void intent (no parameters) — use the\n    // JSON-configured default interval (reminderIntervalSec).\n    SmartActivateUserIntent(USER_INTENT(productivity_session));\n    sSessionStartTime = BaseStationTimer::getInstance()->GetCurrentTimeInSeconds();\n    _dVars.startTime = sSessionStartTime;'
    ),
)

# ── 12. globalInterruptions.json ──────────────────────────────────────────────
print("[12/12] globalInterruptions.json")
multi_patch(
    'resources/config/engine/behaviorComponent/behaviors/victorBehaviorTree/globalInterruptions.json',
    (
        '"WeatherResponses",\n    "TakeAPhotoCoordinator",',
        '"WeatherResponses",\n    "ProductivityReminder",\n    "TakeAPhotoCoordinator",'
    ),
)

print("\n── All done. Rebuild with: ./project/victor/build-victor.sh -p vicos -f ──")
