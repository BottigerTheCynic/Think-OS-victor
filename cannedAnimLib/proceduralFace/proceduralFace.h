/**
 * File: ProceduralFace.h
 *
 * Author: Lee Crippen
 * Created: 11/17/15
 *
 * Description: Holds and sets the face rig data used by ProceduralFace.
 *
 * Copyright: Anki, Inc. 2015
 *
 **/

#ifndef __Anki_Cozmo_ProceduralFace_H__
#define __Anki_Cozmo_ProceduralFace_H__

#include "coretech/common/shared/types.h"
#include "coretech/common/shared/math/point.h"
#include "coretech/vision/engine/image.h"
#include "anki/cozmo/shared/cozmoConfig.h"
#include "clad/types/proceduralFaceTypes.h"
#include "util/console/consoleInterface.h"
#include "util/logging/logging.h"
#include <array>
#include <list>
#include <vector>

#define PROCEDURALFACE_NOISE_FEATURE         1
#define PROCEDURALFACE_ANIMATED_SATURATION   0
#define PROCEDURALFACE_PROCEDURAL_SATURATION 1
#define PROCEDURALFACE_GLOW_FEATURE          0
#define PROCEDURALFACE_SCANLINE_FEATURE      0

namespace Json {
  class Value;
}

namespace CozmoAnim {
  struct ProceduralFace;
}

namespace Anki {

namespace Util {
  class IConsoleFunction;
  class IConsoleVariable;
}

namespace Vision{
  class HueSatWrapper;
}

namespace Vector {

namespace ExternalInterface {
  struct DisplayProceduralFace;
}

class ScanlineDistorter;

class ProceduralFace
{
public:

  static const int WIDTH;
  static const int HEIGHT;

  static const s32 NominalEyeHeight;
  static const s32 NominalEyeWidth;

  static constexpr f32 DefaultHue = 0.45f;
  static constexpr f32 DefaultSaturation = 1.0f;

  using Value = f32;
  using Parameter = ProceduralEyeParameter;

  using EyeParamArray = std::array<Value, static_cast<size_t>(Parameter::NumParameters)>;

  enum WhichEye {
    Left,
    Right
  };

  ProceduralFace();
  ProceduralFace(const ProceduralFace& other);
  ProceduralFace& operator=(const ProceduralFace& other);

  ~ProceduralFace();

  bool operator==(const ProceduralFace& other) const;

  static void SetResetData(const ProceduralFace& newResetData);
  static void SetBlankFaceData(const ProceduralFace& blankFace);

  void Reset(bool withBlankFace = false);

  void SetFromFlatBuf(const CozmoAnim::ProceduralFace* procFaceKeyframe);
  void SetFromJson(const Json::Value &jsonRoot);
  void SetFromValues(const std::vector<f32>& leftEyeData, const std::vector<f32>& rightEyeData,
                     f32 faceAngle_deg, f32 faceCenterX, f32 faceCenterY, f32 faceScaleX, f32 faceScaleY,
                     f32 scanlineOpacity);
  void SetFromMessage(const ProceduralFaceParameters& msg);

  static s32 GetNominalLeftEyeX();
  static s32 GetNominalRightEyeX();
  static s32 GetNominalEyeY();

  void  SetParameter(WhichEye whichEye, Parameter param, Value value);
  Value GetParameter(WhichEye whichEye, Parameter param) const;
  const EyeParamArray& GetParameters(WhichEye whichEye) const;
  void SetParameters(WhichEye whichEye, const EyeParamArray& params);

  void SetParameterBothEyes(Parameter param, Value value);

  void SetFaceAngle(Value value);
  Value GetFaceAngle() const;

  void SetFacePosition(Point<2,Value> center);
  void SetFacePositionAndKeepCentered(Point<2, Value> center);
  Point<2,Value> const& GetFacePosition() const;

  void SetFaceScale(Point<2,Value> scale);
  Point<2,Value> const& GetFaceScale() const;

  void SetScanlineOpacity(Value opacity);
  Value GetScanlineOpacity() const;

  static void  SetHue(Value hue);
  static Value GetHue();
  static void ResetHueToDefault();

  static void  SetSaturation(Value saturation);
  static Value GetSaturation();

  static Vision::Image& GetHueImage();
  static Vision::Image* GetHueImagePtr();

  static Vision::Image& GetSaturationImage();
  static Vision::Image* GetSaturationImagePtr();

  static std::shared_ptr<Vision::HueSatWrapper> GetHueSatWrapper();

  void InitScanlineDistorter(s32 maxAmount_pix, f32 noiseProb);
  void RemoveScanlineDistorter();

  ScanlineDistorter* GetScanlineDistorter()             { return _scanlineDistorter.get(); }
  const ScanlineDistorter* GetScanlineDistorter() const { return _scanlineDistorter.get(); }

  void Interpolate(const ProceduralFace& face1,
                   const ProceduralFace& face2,
                   float fraction,
                   bool usePupilSaccades = false);

  void LookAt(f32 x, f32 y, f32 xmax, f32 ymax,
              f32 lookUpMaxScale = 1.1f, f32 lookDownMinScale=0.85f, f32 outerEyeScaleIncrease=0.1f);

  ProceduralFace& Combine(const ProceduralFace& otherFace);

  static void EnableClippingWarning(bool enable);

  void GetEyeBoundingBox(Value& xmin, Value& xmax, Value& ymin, Value& ymax);

  void RegisterFaceWithConsoleVars();

private:
  static Vision::Image _hueImage;
  static Vision::Image _satImage;

  std::array<EyeParamArray, 2> _eyeParams{{}};

  std::unique_ptr<ScanlineDistorter> _scanlineDistorter;

  Value           _faceAngle_deg   = 0.0f;
  Point<2,Value>  _faceScale       = 1.0f;
  Point<2,Value>  _faceCenter      = 0.0f;
#if PROCEDURALFACE_SCANLINE_FEATURE
  Value           _scanlineOpacity;
#endif

  static Value    _hue;
  static Value    _saturation;

  static void HueConsoleFunction(ConsoleFunctionContextRef context);
  static void SaturationConsoleFunction(ConsoleFunctionContextRef context);

  static std::unique_ptr<Anki::Util::IConsoleFunction> _hueConsoleFunc;
  static std::unique_ptr<Anki::Util::IConsoleFunction> _saturationConsoleFunc;

  void SetEyeArrayHelper(WhichEye eye, const std::vector<Value>& eyeArray);
  void CombineEyeParams(EyeParamArray& eyeArray0, const EyeParamArray& eyeArray1);

  Value Clip(WhichEye eye, Parameter whichParam, Value value) const;

  static ProceduralFace* _resetData;
  static ProceduralFace* _blankFaceData;
  static std::function<void(const char*,Value,Value,Value)> ClipWarnFcn;

  using ConsoleVarPtr = std::unique_ptr<Anki::Util::IConsoleVariable>;
  using ConsoleVarPtrList = std::list<ConsoleVarPtr>;
  ConsoleVarPtrList _consoleVars;

  template<class T>
  void AddConsoleVar(T & var, const char * name, const char * group, const T & minVal, const T & maxVal);

}; // class ProceduralFace

#pragma mark Inlined Methods

inline void ProceduralFace::SetParameter(WhichEye whichEye, Parameter param, Value value)
{
  _eyeParams[whichEye][static_cast<size_t>(param)] = Clip(whichEye, param, value);
}

inline ProceduralFace::Value ProceduralFace::GetParameter(WhichEye whichEye, Parameter param) const
{
  return _eyeParams[whichEye][static_cast<size_t>(param)];
}

inline const ProceduralFace::EyeParamArray& ProceduralFace::GetParameters(WhichEye whichEye) const
{
  return _eyeParams[whichEye];
}

inline void ProceduralFace::SetParameters(WhichEye eye, const EyeParamArray& params) {
  _eyeParams[eye] = params;
}

inline void ProceduralFace::SetParameterBothEyes(Parameter param, Value value)
{
  SetParameter(WhichEye::Left,  param, value);
  SetParameter(WhichEye::Right, param, value);
}

inline ProceduralFace::Value ProceduralFace::GetFaceAngle() const {
  return _faceAngle_deg;
}

inline void ProceduralFace::SetFaceAngle(Value angle_deg) {
  _faceAngle_deg = angle_deg;
}

inline Point<2,ProceduralFace::Value> const& ProceduralFace::GetFacePosition() const {
  return _faceCenter;
}

inline void ProceduralFace::SetFaceScale(Point<2,Value> scale) {
  if(scale.x() < 0) {
    ClipWarnFcn("FaceScaleX", scale.x(), 0, std::numeric_limits<Value>::max());
    scale.x() = 0;
  }
  if(scale.y() < 0) {
    ClipWarnFcn("FaceScaleY", scale.y(), 0, std::numeric_limits<Value>::max());
    scale.y() = 0;
  }
  _faceScale = scale;
}

inline Point<2,ProceduralFace::Value> const& ProceduralFace::GetFaceScale() const {
  return _faceScale;
}

inline void ProceduralFace::SetScanlineOpacity(Value opacity)
{
#if PROCEDURALFACE_SCANLINE_FEATURE
  _scanlineOpacity = opacity;
  if(!Util::InRange(_scanlineOpacity, Value(0), Value(1)))
  {
    ClipWarnFcn("ScanlineOpacity", _scanlineOpacity, Value(0), Value(1));
    _scanlineOpacity = Util::Clamp(_scanlineOpacity, Value(0), Value(1));
  }
#endif
}

inline ProceduralFace::Value ProceduralFace::GetScanlineOpacity() const {
#if PROCEDURALFACE_SCANLINE_FEATURE
  return _scanlineOpacity;
#else
  return 1.0f;
#endif
}

inline void ProceduralFace::SetHue(Value hue) {
  _hue = hue;
  if(!Util::InRange(_hue, Value(0), Value(1)))
  {
    ClipWarnFcn("Hue", _hue, Value(0), Value(1));
    _hue = Util::Clamp(_hue, Value(0), Value(1));
  }
  GetHueImage().FillWith(static_cast<u8>(_hue * std::numeric_limits<u8>::max()));
}

inline ProceduralFace::Value ProceduralFace::GetHue() {
  return _hue;
}

inline void ProceduralFace::ResetHueToDefault() {
  _hue = DefaultHue;
}

inline void ProceduralFace::SetSaturation(Value saturation) {
  _saturation = saturation;
  if(!Util::InRange(_saturation, Value(0), Value(1)))
  {
    ClipWarnFcn("Saturation", _saturation, Value(0), Value(1));
    _saturation = Util::Clamp(_saturation, Value(0), Value(1));
  }
  GetSaturationImage().FillWith(static_cast<u8>(_saturation * std::numeric_limits<u8>::max()));
}

inline ProceduralFace::Value ProceduralFace::GetSaturation() {
  return _saturation;
}

inline Vision::Image& ProceduralFace::GetHueImage() {
  return _hueImage;
}

inline Vision::Image* ProceduralFace::GetHueImagePtr() {
  return &_hueImage;
}

inline Vision::Image& ProceduralFace::GetSaturationImage() {
  return _satImage;
}

inline Vision::Image* ProceduralFace::GetSaturationImagePtr() {
  return &_satImage;
}


} // namespace Vector
} // namespace Anki

#endif // __Anki_Cozmo_ProceduralFace_H__
