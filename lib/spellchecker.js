const path = require('path');
const spellchecker = require('../build/Release/spellchecker.node');

let hasInit = false;

function getDictionaryPath() {
  let dict = path.join(__dirname, '..', 'vendor', 'hunspell_dictionaries');
  try {
    // HACK: Special case being in an asar archive
    const unpacked = dict.replace('.asar' + path.sep, '.asar.unpacked' + path.sep);
    if (require('fs').statSync(unpacked)) {
      dict = unpacked;
    }
  } catch (error) {
  }
  return dict;
}

function ensureDefaultSpellCheck(lang = process.env.LANG) {
  if (hasInit) return;

  lang = lang ? lang.split('.')[0] : 'en_US';
  hasInit = true;

  spellchecker.setDictionary(lang, getDictionaryPath());
}

function checkSpellingAsync(corpus) {
  return new Promise((resolve, reject) => {
    ensureDefaultSpellCheck();
    spellchecker.checkSpellingAsync(corpus, (err, data) => {
      if (err) {
        reject(err);
      } else {
        resolve(data);
      }
    });
  });
}

module.exports = Object.keys(spellchecker)
  .filter(key => key !== 'checkSpellingAsync' && key !== 'setDictionary')
  .reduce((res, key) => {
    res[key] = function () {
      ensureDefaultSpellCheck();
      return spellchecker[key].apply(spellchecker, Array.prototype.slice.call(arguments));
    };
    return res;
  }, {
    setDictionary: spellchecker.setDictionary,
    ensureDefaultSpellCheck,
    getDictionaryPath,
    checkSpellingAsync
  });
