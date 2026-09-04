import sdl2w;

int main() {
  static_assert(sizeof(sdl2w::TextSize) >= 1);

  bmin::String title("modules");
  sdl2w::Store store;
  (void)store;
  (void)title;

  sdl2w::Logger::setLogLevel(sdl2w::INFO);
  LOG(INFO) << "sdl2w modules smoke ok" << LOG_ENDL;

  const bmin::String translated(TRANSLATE("module translation"));
  if (translated != "module translation") {
    return 1;
  }

  sdl2w::L10n::setEnabled(true);
  sdl2w::L10n::loadLanguage(
      "en", "[module translation]{translated module text}");
  sdl2w::L10n::setLanguage("en");
  if (bmin::String(TRANSLATE("module translation")) !=
      "translated module text") {
    return 2;
  }
  sdl2w::L10n::setEnabled(false);
  return 0;
}
