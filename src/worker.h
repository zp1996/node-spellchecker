#ifndef WORKER_H
#define WORKER_H

#include "spellchecker.h"

#include <vector>
#include <napi.h>

using namespace spellchecker;
using namespace Napi;

class CheckSpellingWorker : public AsyncWorker {
public:
  CheckSpellingWorker(std::vector<uint16_t> &&corpus, SpellcheckerImplementation* impl, Function& callback);
  ~CheckSpellingWorker();

  void Execute();
  void OnOK();
private:
  const std::vector<uint16_t> corpus;
  SpellcheckerImplementation* impl;
  std::vector<MisspelledRange> misspelled_ranges;
};

#endif
