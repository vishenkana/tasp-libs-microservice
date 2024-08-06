#include "microservice_impl.hpp"

#include <unistd.h>

#include <cstring>
#include <experimental/filesystem>
#include <iterator>

#include <tasp/arguments.hpp>
#include <tasp/logging.hpp>

using std::string;
using std::string_view;
using std::vector;

using namespace std::string_literals;

namespace tasp
{
/*------------------------------------------------------------------------------
    MicroServiceImpl
------------------------------------------------------------------------------*/
MicroServiceImpl::MicroServiceImpl(int argc, const char **argv) noexcept
: Daemon(argc, argv)
{
    Reload();
}

//------------------------------------------------------------------------------
MicroServiceImpl::~MicroServiceImpl() noexcept = default;

//------------------------------------------------------------------------------
void MicroServiceImpl::Reload() noexcept
{
    const auto &config = ConfigGlobal::Instance();
    name_ = config.Get("service.name", name_);
    prefix_ = config.Get("service.prefix", prefix_);

    const string address = config.Get("service.address", "*"s);
    auto port = config.Get<uint16_t>("service.port", 5555);
    auto pool_size = config.Get<size_t>("service.pool_size", 10);

    Logging::Info("Параметры HTTP-сервера {}:{}", address, port);

    pool_.clear();
    handlers_.clear();

    auto func = [this](auto &&request, auto &&response)
    {
        Request(std::forward<decltype(request)>(request),
                std::forward<decltype(response)>(response));
    };

    pool_.reserve(pool_size);

    auto &primary{pool_.emplace_back(address, port, func)};

    evutil_socket_t socket{primary.GetSocket()};

    for (size_t i = 0; i < pool_size - 1; i++)
    {
        pool_.emplace_back(socket, func);
    }

    handlers_.reserve(10);

    check_functions_.clear();
    AddDefaultCheckFunctions();
    AddHealthHandler();
}

//------------------------------------------------------------------------------
void MicroServiceImpl::AddHandler(http::Request::Method method,
                                  string_view path,
                                  const Handler &func) noexcept
{
    string regex_path{path.data()};
    if (regex_path.back() == '/')
    {
        regex_path.pop_back();
    };
    regex_path.append("/?");

    handlers_.emplace_back(method, prefix_ + regex_path, func);
}

//------------------------------------------------------------------------------
void MicroServiceImpl::AddHealthHandler() noexcept
{
    handlers_.emplace_back(
        http::Request::Method::Get,
        prefix_ + "/health",
        [this]([[maybe_unused]] auto &&request, auto &&response)
        {
            response.Data()->Set(HealthCheck());
        });
}

//------------------------------------------------------------------------------
void MicroServiceImpl::AddCheckFunctions(
    const vector<CheckFunction> &check_functions) noexcept
{
    check_functions_.insert(check_functions_.end(),
                            std::make_move_iterator(check_functions.begin()),
                            std::make_move_iterator(check_functions.end()));
}

//------------------------------------------------------------------------------
Json::Value MicroServiceImpl::HealthCheck() const noexcept
{
    HealthReport::Status status{HealthReport::Status::Ok};
    Json::Value checks_json;

    for (auto &&check_function : check_functions_)
    {
        const HealthReport check_report = check_function();
        checks_json.append(check_report.ToJSON());

        // в качестве статуса сервиса выбираем самый критичный статус по всем
        // проверкам
        const auto &check_status = check_report.GetStatus();
        if (check_status > status)
        {
            status = check_status;
        }
    }

    Json::Value health_json = HealthReport{name_, status}.ToJSON();
    if (!checks_json.empty())
    {
        health_json["check_reports"] = checks_json;
    }
    return health_json;
}

//------------------------------------------------------------------------------
// В данный момент единственная проверка по умолчанию - проверка прав доступа к
// директориям.
void MicroServiceImpl::AddDefaultCheckFunctions() noexcept
{
    auto dirs_config = ConfigGlobal::Instance().Get<vector<string>>("dirs");
    if (dirs_config.empty())
    {
        Logging::Warning("Недопустимая конфигурация директорий (\"dirs\")");
    }
    else
    {
        check_functions_.emplace_back(
            [dirs_config]
            {
                return DirectoriesCheck(dirs_config);
            });
    }
}

//------------------------------------------------------------------------------
HealthReport MicroServiceImpl::DirectoriesCheck(
    const vector<string> &dirs_ids) noexcept
{
    auto perms_string_to_int = [](string_view str)
    {
        int perms = -1;
        if (str == "r")
        {
            perms = R_OK;
        }
        else if (str == "w")
        {
            perms = W_OK;
        }
        else if (str == "x")
        {
            perms = X_OK;
        }
        else if (str == "rw")
        {
            perms = R_OK | W_OK;
        }
        else if (str == "rx")
        {
            perms = R_OK | X_OK;
        }
        else if (str == "wx")
        {
            perms = W_OK | X_OK;
        }
        else if (str == "rwx")
        {
            perms = R_OK | W_OK | X_OK;
        }
        return perms;
    };

    HealthReport::Status report_status{HealthReport::Status::Ok};
    string report_message;

    // russian locale for default error messages
    locale_t locale = newlocale(LC_ALL_MASK, "ru_RU.utf-8", nullptr);
    locale_t ru_locale = uselocale(locale);

    const auto &config = ConfigGlobal::Instance();

    for (const auto &dir_id : dirs_ids)
    {
        string message;

        const auto path = config.Get<string>("dirs." + dir_id + ".path");
        const auto perms = config.Get<string>("dirs." + dir_id + ".access");

        if (path.empty() || perms.empty())
        {
            message = "Недопустимая конфигурация директории (" + dir_id +
                      "): путь и/или права доступа не указаны";
        }
        else
        {
            const int access_denied = access(
                path.c_str(),
                perms_string_to_int(perms));  // access returns 0 on success
            if (access_denied != 0)
            {
                report_status = HealthReport::Status::Warning;
                switch (errno)
                {
                    case EACCES:
                        message = "Недопустимые права доступа (" + perms +
                                  ") к директории (";
                        message.append(dir_id).append(")");
                        break;
                    case ENOENT:
                    case ENOTDIR:
                        message =
                            "Указанный путь (" + path + ") к директории (";
                        message.append(dir_id).append(") не существует");
                        break;
                    case EINVAL:
                        message =
                            "Права доступа (" + perms + ") к директории (";
                        message.append(dir_id).append(") указаны неверно");
                        break;
                    default:
                        string const error{strerror_l(errno, ru_locale)};
                        message = "Неизвестная ошибка (";
                        message.append(dir_id).append("): \"");
                        message.append(error).append("\"");
                }
            }
        }
        if (!message.empty())
        {
            report_message += message + "\n";
            Logging::Warning("{}", message);
        }
    }
    return {"Права доступа к директориям", report_status, report_message};
}

//------------------------------------------------------------------------------
void MicroServiceImpl::Request(const http::Request &request,
                               http::Response &response) noexcept
{
    for (auto &handler : handlers_)
    {
        if (handler.Method() == request.GetMethod() &&
            request.Uri()->Match(handler.Path()))
        {
            handler.Exec(request, response);
            return;
        }
    }

    response.SetCode(http::Response::Code::NotFound);
}

}  // namespace tasp
