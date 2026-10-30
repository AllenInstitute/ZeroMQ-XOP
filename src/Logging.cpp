#include "ZeroMQ.h"

#include <optional>

namespace
{
constexpr const char *DEFAULT_TEMPLATE     = "{}";
constexpr const char *SESSION_START_MARKER = "{}";
constexpr const char *IDENTITY_KEY         = "remote identity";

constexpr const char *TEMPLATE_KEY_JSON      = "json";
constexpr const char *TEMPLATE_KEY_STR       = "str";
constexpr const char *TEMPLATE_KEY_DIRECTION = "direction";
constexpr const char *TEMPLATE_KEY_TS        = "ts";

constexpr std::array<const char *, 4> RESERVED_KEYS = {
    TEMPLATE_KEY_DIRECTION, TEMPLATE_KEY_JSON, TEMPLATE_KEY_STR,
    TEMPLATE_KEY_TS};

#ifdef WINIGOR
constexpr const char *DIR_SEPARATOR = "\\";
#else
#ifdef MACIGOR
constexpr const char *DIR_SEPARATOR = "/";
#endif // MACIGOR
#endif // WINIGOR

bool IsValidUTF8(const std::string &str)
{
  try
  {
    json(str).dump();
    return true;
  }
  catch(const json::type_error &)
  {
    return false;
  }
}

/// ZeroMQ generates binary identities, starting with a zero byte, for clients
/// which don't set one, see ZMQ_ROUTING_ID. A JSON string can only hold UTF-8.
std::string ToLoggableIdentity(const std::string &identity)
{
  ASSERT(!identity.empty());

  if(identity.front() != '\0' && IsValidUTF8(identity))
  {
    return identity;
  }

  const auto *bytes = reinterpret_cast<const unsigned char *>(identity.data());

  return fmt::format(FMT_STRING("0x{:02x}"),
                     fmt::join(bytes, bytes + identity.size(), ""));
}

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
  auto format(const MessageDirection &mode, FormatContext &ctx) const
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
                   const std::optional<std::string> &identity,
                   std::optional<MessageDirection> dir) const
  {
    json doc = m_template;

    doc[key] = entry;

    if(dir.has_value())
    {
      doc[TEMPLATE_KEY_DIRECTION] = fmt::to_string(dir.value());
    }

    if(identity.has_value() && !identity->empty())
    {
      doc[IDENTITY_KEY] = ToLoggableIdentity(identity.value());
    }

    const auto now = std::chrono::system_clock::now();
    // see https://stackoverflow.com/a/67076017
    const auto timestamp = fmt::format(FMT_STRING("{0:%FT%H:%M:}{1:%S}{0:%z}"),
                                       now, now.time_since_epoch());
    doc[TEMPLATE_KEY_TS] = timestamp;

    WriteIntoLogfile(doc.dump(-1, ' ', false, json::error_handler_t::replace));
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
    std::array<char, MAX_PATH_LEN + 1> pathOut{};

    auto rc = SpecialDirPath("Igor Preferences", 0, 0x1, 1, pathOut.data());
    ASSERT(rc == 0);

    const auto path = fmt::format("{}Packages{}{}{}", pathOut.data(),
                                  DIR_SEPARATOR, package, DIR_SEPARATOR);

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
  const std::optional<std::string> identity;
  m_impl->AddLogEntry(TEMPLATE_KEY_JSON, doc, identity, dir);
}

void Logging::AddLogEntry(const json &doc, const std::string &identity,
                          MessageDirection dir) const
{
  m_impl->AddLogEntry(TEMPLATE_KEY_JSON, doc, identity, dir);
}

void Logging::AddLogEntry(const std::string &str) const
{
  const std::optional<std::string> identity;
  const std::optional<MessageDirection> dir;
  m_impl->AddLogEntry(TEMPLATE_KEY_STR, str, identity, dir);
}

void Logging::AddLogEntry(const std::string &str, MessageDirection dir) const
{
  const std::optional<std::string> identity;
  m_impl->AddLogEntry(TEMPLATE_KEY_STR, str, identity, dir);
}

void Logging::AddLogEntry(const std::string &str, const std::string &identity,
                          MessageDirection dir) const
{
  m_impl->AddLogEntry(TEMPLATE_KEY_STR, str, identity, dir);
}
