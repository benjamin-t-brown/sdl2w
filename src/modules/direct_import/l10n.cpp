import sdl2w.l10n;

sdl2w::L10n* l10nImportProbe() { return nullptr; }

const char* l10nConvenienceProbe() { return TRANSLATE("module text"); }

static_assert(DISABLE_TRANSLATIONS[0] == 'd');
