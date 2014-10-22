#include "settings.h"

using namespace Windows::Storage;
using Windows::Foundation::PropertyValue;
using Windows::Foundation::IPropertyValue;

static Platform::String^ toString(const std::wstring& s) {
  return ref new Platform::String(s.c_str());
}

namespace midi {

  struct Settings::Impl {
    ApplicationDataCompositeValue^ composite;
    Platform::String^ key;
  };

  Settings::~Settings() = default;

  Settings::Settings(Settings&& o)
    : pimpl(std::move(o.pimpl))
  {
  }

  Settings::Settings(std::wstring name) {
    pimpl = std::make_unique<Impl>();
    auto pstring = ref new Platform::String(name.c_str());
    pimpl->composite = safe_cast<ApplicationDataCompositeValue^>(ApplicationData::Current->LocalSettings->Values->Lookup(pstring));

    pimpl->key = pstring;

    if (!pimpl->composite)
      pimpl->composite = ref new ApplicationDataCompositeValue();
  }

  void Settings::save() {
    ApplicationData::Current->LocalSettings->Values->Insert(pimpl->key, pimpl->composite);
  }

  std::wstring Settings::getInner(std::wstring key, std::wstring default, StringType) {
    auto value = safe_cast<Platform::String^>(pimpl->composite->Lookup(toString(key)));
    if (value)
      return{ value->Begin(), value->End() };
    else
      return default;
  }

  int Settings::getInner(std::wstring key, int default, IntType) {
    auto value = safe_cast<IPropertyValue^>(pimpl->composite->Lookup(toString(key)));
    if (value)
      return value->GetInt32();
    else
      return default;
  }

  void Settings::setInner(std::wstring key, std::wstring value, StringType) {
    pimpl->composite->Insert(toString(key), dynamic_cast<PropertyValue^>(PropertyValue::CreateString(toString(value))));
  }

  void Settings::setInner(std::wstring key, int value, IntType) {
    pimpl->composite->Insert(toString(key), dynamic_cast<PropertyValue^>(PropertyValue::CreateInt32(value)));
  }

  Settings getSettings(std::wstring name) {
    return { std::move(name) };
  }

}