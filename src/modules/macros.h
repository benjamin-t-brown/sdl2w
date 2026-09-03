#pragma once

// Macros cannot be exported from C++ modules. Include this after `import sdl2w`
// (or `import sdl2w.logger` / `import sdl2w.l10n`).
// Logging also has an exported function API: sdl2w::log / logAt / fail / endl.
// TRANSLATE must stay a macro (L10nScanner looks for TRANSLATE("...")).

#define LOG(level) sdl2w::log(sdl2w::LogType::level)
#define LOG_LINE(level) sdl2w::logAt(sdl2w::LogType::level)
#define LOG_ENDL sdl2w::endl
#define THROW_RUNTIME_ERROR(msg) sdl2w::fail(msg)

#define TRANSLATE(text)                                                        \
  (sdl2w::L10n::isEnabled()                                                    \
       ? sdl2w::L10n::trans(sdl2w::L10n::hash(text)).cStr()                    \
       : (text))
#define DISABLE_TRANSLATIONS "default"
