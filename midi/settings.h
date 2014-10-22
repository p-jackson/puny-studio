#include <string>
#include <memory>

namespace midi {

  struct StringType {};
  struct IntType {};

  template<typename T> struct GetTypeTag;
  template<> struct GetTypeTag<std::wstring> { using type = StringType; };
  template<> struct GetTypeTag<int> { using type = IntType; };

  class Settings {
    struct Impl;
    std::unique_ptr<Impl> pimpl;

  public:
    ~Settings();

    Settings(Settings&&);

    void save();

    template<typename T>
    T get(std::wstring key, T default = {}) {
      return getInner(std::move(key), std::move(default), GetTypeTag<T>::type{});
    }

    template<typename T>
    void set(std::wstring key, T value) {
      setInner(std::move(key), std::move(value), GetTypeTag<T>::type{});
    }

  private:
    friend Settings getSettings(std::wstring);

    Settings(std::wstring name);

    std::wstring getInner(std::wstring key, std::wstring default, StringType);
    int getInner(std::wstring key, int default, IntType);

    void setInner(std::wstring key, std::wstring value, StringType);
    void setInner(std::wstring key, int value, IntType);
  };

  Settings getSettings(std::wstring name);

}