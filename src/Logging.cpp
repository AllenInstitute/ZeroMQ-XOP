#include "ZeroMQ.h"

namespace
{
constexpr char DEFAULT_TEMPLATE[]     = "{}";
constexpr char SESSION_START_MARKER[] = "{}";

constexpr char TEMPLATE_KEY_JSON[]      = "json";
constexpr char TEMPLATE_KEY_STR[]       = "str";
constexpr char TEMPLATE_KEY_DIRECTION[] = "direction";
constexpr char TEMPLATE_KEY_TS[]        = "ts";

constexpr std::array<const char *, 4> RESERVED_KEYS = {
    TEMPLATE_KEY_DIRECTION, TEMPLATE_KEY_JSON, TEMPLATE_KEY_STR,
    TEMPLATE_KEY_TS};

#ifdef WINIGOR
constexpr const char DIR_SEPARATOR[] = "\\";
#else
#ifdef MACIGOR
constexpr const char DIR_SEPARATOR[] = "/";
#endif // MACIGOR
#endif // WINIGOR

json ParseLoggingTemplate(const std::string &str)
{
  json doc;

  try
  {
    doc = json::parse(str);
  }
  catch(const std::exception &)
  {
    throw IgorException(INVALID_LOGGING_TEMPLATE,
                        "The template string is not a valid JSON text.");
  }

  if(doc.type() != json::value_t::object)
  {
    throw IgorException(
        INVALID_LOGGING_TEMPLATE,
        "The returned JSON text does not have an object at the top-level.");
  }

  for(const auto &it : RESERVED_KEYS)
  {
    if(doc.contains(it))
    {
      throw IgorException(
          INVALID_LOGGING_TEMPLATE,
          fmt::format("The returned JSON text does contain a reserved key. "
                      "The following keys are reserved: {}.",
                      RESERVED_KEYS));
    }
  }

  return doc;
}

} // namespace

template <>
struct fmt::formatter<MessageDirection> : fmt::formatter<std::string>
{
  // parse is inherited from formatter<std::string>.
  template <typename FormatContext>
  auto format(MessageDirection mode, FormatContext &ctx)
  {
    std::string name;
    switch(mode)
    {
    case MessageDirection::Outgoing:
      name = "Outgoing";
      break;
    case MessageDirection::Incoming:
      name = "Incoming";
      break;
    }

    return formatter<std::string>::format(name, ctx);
  }
};

class Logging::Implementation
{
public:
  Implementation(const std::string &package, const std::string &loggingTemplate)
  {
    CreatePackageDirectory(package);

    m_template = ParseLoggingTemplate(loggingTemplate);

    DEBUG_OUTPUT("logging template = {}", m_template.dump());

    // mark start of new experiment/session
    WriteIntoLogfile(SESSION_START_MARKER);
  }

  template <typename T>
  void AddLogEntry(const std::string &key, const T &entry,
                   MessageDirection dir) const
  {
    json j = m_template;

    j[key]                    = entry;
    j[TEMPLATE_KEY_DIRECTION] = fmt::to_string(dir);

    const auto now = std::chrono::system_clock::now();
    // see https://stackoverflow.com/a/67076017
    const auto ts = fmt::format(FMT_STRING("{0:%FT%H:%M:}{1:%S}{0:%z}"), now,
                                now.time_since_epoch());
    j[TEMPLATE_KEY_TS] = ts;

    WriteIntoLogfile(j.dump());
  }

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996)
#endif

  void WriteIntoLogfile(std::string str) const
  {
    if(str.empty())
    {
      return;
    }

    str.append("\n");

    // can't use XOPOpenFile here as we want to open the file for appending
    XOP_FILE_REF fileRef = fopen(m_logFile.c_str(), "ab");
    ASSERT(fileRef != nullptr);

    auto bytesToWrite   = static_cast<SInt64>(str.size());
    SInt64 bytesWritten = 0;
    int rc = XOPWriteFile64(fileRef, bytesToWrite, str.c_str(), &bytesWritten);
    ASSERT(rc == 0 && bytesWritten == bytesToWrite);

    rc = XOPCloseFile(fileRef);
    ASSERT(rc == 0);
  }

#ifdef _MSC_VER
#pragma warning(pop)
#endif

  void CreatePackageDirectory(const std::string &package)
  {
    char pathOut[MAX_PATH_LEN + 1];

    auto rc = SpecialDirPath("Igor Preferences", 0, 0x1, 1, pathOut);
    ASSERT(rc == 0);

    const auto path = fmt::format("{}Packages{}{}{}", pathOut, DIR_SEPARATOR,
                                  package, DIR_SEPARATOR);

    EnsureDirectoryExists(path);

    m_packageDir = path;
    m_logFile    = m_packageDir + "Log.jsonl";

    DEBUG_OUTPUT("log file = {}", m_logFile);
  }

private:
  std::string m_packageDir;
  json m_template;
  std::string m_logFile;
};

Logging::Logging(const std::string &package, const std::string &loggingTemplate)
    : m_impl(
          std::make_unique<Logging::Implementation>(package, loggingTemplate))
{
}

Logging::Logging(const std::string &package)
    : Logging(package, DEFAULT_TEMPLATE)
{
}

Logging::~Logging() = default;

void Logging::AddLogEntry(const json &doc, MessageDirection dir) const
{
  m_impl->AddLogEntry(TEMPLATE_KEY_JSON, doc, dir);
}

void Logging::AddLogEntry(const std::string &str, MessageDirection dir) const
{
  m_impl->AddLogEntry(TEMPLATE_KEY_STR, str, dir);
}
