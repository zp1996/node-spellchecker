#include "worker.h"
#include "spellchecker.h"

#include <string>
#include <vector>
#include <utility>
#include <napi.h>

CheckSpellingWorker::CheckSpellingWorker(
  std::vector<uint16_t>&& corpus,
  SpellcheckerImplementation* impl,
  Function& callback
) : AsyncWorker(callback), corpus(std::move(corpus)), impl(impl)
{
  // No-op
}

CheckSpellingWorker::~CheckSpellingWorker()
{
  // No-op
}

void CheckSpellingWorker::Execute() {
  std::unique_ptr<SpellcheckerThreadView> view = impl->CreateThreadView();
  misspelled_ranges = view->CheckSpelling(corpus.data(), corpus.size());
}

void CheckSpellingWorker::OnOK() {
  HandleScope scope(Env());

  Array result = Array::New(Env(), misspelled_ranges.size());
  size_t i = 0;
  for (const MisspelledRange& range: misspelled_ranges) {
    Object obj = Object::New(Env());

    obj.Set(String::New(Env(), "start"), Number::New(Env(), range.start));
    obj.Set(String::New(Env(), "end"), Number::New(Env(), range.end));

    result[i++] = obj;
  }

  Callback().Call({ Env().Null(), result });
}
