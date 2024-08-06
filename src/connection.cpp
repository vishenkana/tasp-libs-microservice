#include "connection.hpp"

#include <tasp/logging.hpp>

#include "http/request_impl.hpp"
#include "http/response_impl.hpp"

using std::make_unique;
using std::string;
using std::string_view;
using std::thread;

using tasp::http::RequestImpl;
using tasp::http::ResponseImpl;

namespace tasp::ev
{
/*------------------------------------------------------------------------------
    Connection
------------------------------------------------------------------------------*/
Connection::Connection(string_view address,
                       uint16_t port,
                       const Handler &func) noexcept
: Connection(func)
{
    auto *info =
        evhttp_bind_socket_with_handle(server_.get(), address.data(), port);
    if (info == nullptr)
    {
        Logging::Error("Ошибка привязки адреса и порта");
        return;
    }

    socket_ = evhttp_bound_socket_get_fd(info);
}

//------------------------------------------------------------------------------
Connection::Connection(evutil_socket_t socket, const Handler &func) noexcept
: Connection(func)
{
    const int res{evhttp_accept_socket(server_.get(), socket)};
    if (res != 0)
    {
        Logging::Error("Ошибка привязки HTTP-сервера с сокетом");
    }
}

//------------------------------------------------------------------------------
Connection::Connection(Handler func) noexcept
: func_(std::move(func))
{
    const int flags{EVENT_BASE_FLAG_EPOLL_USE_CHANGELIST |
                    EVENT_BASE_FLAG_NO_CACHE_TIME | EVENT_BASE_FLAG_IGNORE_ENV};

    const EvConf config{event_config_new(), &event_config_free};
    event_config_set_flag(config.get(), flags);

    event_ = EvBase(event_base_new_with_config(config.get()), event_base_free);
    server_ = EvHttp(evhttp_new(event_.get()), evhttp_free);

    const uint16_t all_methods{511};
    evhttp_set_allowed_methods(server_.get(), all_methods);

    evhttp_set_gencb(server_.get(), &Connection::Request, this);

    auto *event{event_new(
        event_.get(), -1, EV_PERSIST, [](int, int16_t, void *) {}, nullptr)};

    event_exit_ = EvEvent(event, event_free);
    const timeval sec = {0, 1};
    event_add(event_exit_.get(), &sec);

    thread_ = make_unique<thread>(&event_base_dispatch, event_.get());
}

//------------------------------------------------------------------------------
Connection::~Connection() noexcept
{
    event_base_loopexit(event_.get(), nullptr);

    thread_->join();

    event_exit_.reset(nullptr);
    server_.reset(nullptr);
    event_.reset(nullptr);
}

//------------------------------------------------------------------------------
void Connection::Request(evhttp_request *req, void *arg) noexcept
{
    auto *server = static_cast<Connection *>(arg);

    const RequestImpl request(req);
    ResponseImpl response(req);

    server->func_(request, response);

    response.Send();
}

//------------------------------------------------------------------------------
evutil_socket_t Connection::GetSocket() const noexcept
{
    return socket_;
}

/*------------------------------------------------------------------------------
    HandlerImpl
------------------------------------------------------------------------------*/
HandlerImpl::HandlerImpl(http::Request::Method method,
                         std::string_view path,
                         Handler func) noexcept
: method_(method)
, path_(path)
, func_(std::move(func))
{
}

//------------------------------------------------------------------------------
HandlerImpl::~HandlerImpl() noexcept = default;

//------------------------------------------------------------------------------
http::Request::Method HandlerImpl::Method() const noexcept
{
    return method_;
}

//------------------------------------------------------------------------------
const string &HandlerImpl::Path() const noexcept
{
    return path_;
}

//------------------------------------------------------------------------------
void HandlerImpl::Exec(const http::Request &request,
                       http::Response &response) noexcept
{
    func_(request, response);
}

}  // namespace tasp::ev
