#include "spellchecker.h"
#include "worker.h"

#include <vector>
#include <utility>
#include <v8.h>
#include <napi.h>

using namespace Napi;
using namespace spellchecker;

#define CHECKARG(num)                                                       \
  if (info.Length() < num) {                                                \
    Napi::TypeError::New(env, "Bad argument").ThrowAsJavaScriptException(); \
    return env.Null();                                                      \
  }                                                                         \

SpellcheckerImplementation* impl = SpellcheckerFactory::CreateSpellchecker();

Value IsMisspelled(const CallbackInfo& info) {
  Env env = info.Env();

  CHECKARG(1);

  return Boolean::New(env, impl->IsMisspelled(info[0].As<String>()));
}

Value SetDictionary(const CallbackInfo& info) {
  Env env = info.Env();

  CHECKARG(1);

  std::string language = info[0].As<String>();
  std::string directory = ".";
  if (info.Length() > 1) {
    directory = info[1].As<String>();
  }

  return Boolean::New(env, impl->SetDictionary(language, directory));
}

Value Add(const CallbackInfo& info) {
  Env env = info.Env();

  CHECKARG(1);

  impl->Add(info[0].As<String>());
  return env.Null();
}

Value BatchAdd(const CallbackInfo& info) {
  Env env = info.Env();

  CHECKARG(1);

  Array words = info[0].As<Array>();
  uint32_t length = words.Length();

  for (uint32_t i = 0; i < length; i++) {
    Value word = words[i];
    impl->Add(word.As<String>());
  }

  return env.Null();
}

Value Remove(const CallbackInfo& info) {
  Env env = info.Env();

  CHECKARG(1);

  impl->Remove(info[0].As<String>());
  return env.Null();
}

Array ConvertStringVector(Env env, std::vector<std::string> &vec) {
  size_t size = vec.size();
  Array result = Array::New(env, size);
  size_t i = 0;
  for (const std::string& v: vec) {
    result[i++] = String::New(env, v);
  }

  return result;
}

Value GetCorrectionsForMisspelling(const CallbackInfo& info) {
  Env env = info.Env();

  CHECKARG(1);

  std::vector<std::string> corrections = impl->GetCorrectionsForMisspelling(
    info[0].As<String>()
  );

  return ConvertStringVector(env, corrections);
}

Value GetAvailableDictionaries(const CallbackInfo& info) {
  Env env = info.Env();

  std::string path = ".";
  if (info.Length() > 0) {
    path = info[0].As<String>();
  }

  std::vector<std::string> dictionaries = impl->GetAvailableDictionaries(path);

  return ConvertStringVector(env, dictionaries);
}

inline std::vector<uint16_t> ConvertToUint16(Value value) {
  std::u16string str = value.ToString().Utf16Value();
  std::vector<uint16_t> text{
    (uint16_t*) &*str.begin(),
    (uint16_t*) &*str.end()
  };

  return text;
}

Value CheckSpelling(const CallbackInfo& info) {
  Env env = info.Env();

  CHECKARG(1);

  if (!info[0].IsString()) {
    TypeError::New(
      env, "argument shoule be a string").ThrowAsJavaScriptException();
    return env.Null();
  }

  std::vector<uint16_t> corpus = ConvertToUint16(info[0]);
  std::vector<MisspelledRange> ranges = impl->CheckSpelling(corpus.data(), corpus.size());

  Array result = Array::New(env, ranges.size());
  size_t i = 0;
  for (const MisspelledRange& range: ranges) {
    Object obj = Object::New(env);

    obj.Set(String::New(env, "start"), Number::New(env, range.start));
    obj.Set(String::New(env, "end"), Number::New(env, range.end));

    result[i++] = obj;
  }

  return result;
}

Value CheckSpellingAsync(const CallbackInfo& info) {
  Env env = info.Env();

  CHECKARG(2);

  if (!info[0].IsString()) {
    TypeError::New(
      env, "argument shoule be a string").ThrowAsJavaScriptException();
    return env.Null();
  }

  Function callback = info[1].As<Function>();

  std::vector<uint16_t> corpus = ConvertToUint16(info[0]);
  CheckSpellingWorker* worker = new CheckSpellingWorker(std::move(corpus), impl, callback);
  worker->Queue();

  return env.Null();
}

Object Init(Env env, Object exports) {
  exports.Set(String::New(env, "isMisspelled"), Function::New(env, IsMisspelled));
  exports.Set(String::New(env, "setDictionary"), Function::New(env, SetDictionary));
  exports.Set(String::New(env, "add"), Function::New(env, Add));
  exports.Set(String::New(env, "batchAdd"), Function::New(env, BatchAdd));
  exports.Set(String::New(env, "remove"), Function::New(env, Remove));
  exports.Set(
    String::New(env, "getCorrectionsForMisspelling"),
    Function::New(env, GetCorrectionsForMisspelling)
  );
  exports.Set(
    String::New(env, "getAvailableDictionaries"),
    Function::New(env, GetAvailableDictionaries)
  );
  exports.Set(String::New(env, "checkSpelling"), Function::New(env, CheckSpelling));
  exports.Set(
    String::New(env, "checkSpellingAsync"), Function::New(env, CheckSpellingAsync)
  );

  return exports;
}

NODE_API_MODULE(spellchecker, Init);
