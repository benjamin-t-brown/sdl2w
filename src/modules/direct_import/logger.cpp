import sdl2w.logger;

auto loggerImportProbe() { return INFO; }

void loggerConvenienceProbe() {
  LOG(DEBUG) << "debug" << LOG_ENDL;
  LOG_LINE(WARN) << "warning" << LOG_ENDL;
  if (false) {
    THROW_RUNTIME_ERROR("not thrown");
  }
}
