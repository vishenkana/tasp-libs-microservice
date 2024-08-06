#include "request_impl.hpp"

#include <tasp/logging.hpp>

#include "header_impl.hpp"
#include "uri_impl.hpp"

using std::make_shared;
using std::shared_ptr;
using std::string;

namespace tasp::http
{

/*------------------------------------------------------------------------------
    RequestImpl
------------------------------------------------------------------------------*/
RequestImpl::RequestImpl(evhttp_request *req) noexcept
: req_(req)
, uri_(make_shared<UriImpl>(req_))
, method_(static_cast<Request::Method>(req_->type))
, headers_(make_shared<HeaderImpl>(req_))
, data_(make_shared<http::Data>())
{
    const string &url = uri_->Url();
    const string &client = headers_->Get("client");
    const string &method = MethodToString(method_);

    Logging::Info("HTTP-запрос {} {} от клиента {}", method, url, client);

    ReadInputBuffer();
}

//------------------------------------------------------------------------------
RequestImpl::~RequestImpl() noexcept = default;

//------------------------------------------------------------------------------
shared_ptr<Uri> RequestImpl::Uri() const noexcept
{
    return uri_;
}

//------------------------------------------------------------------------------
shared_ptr<Header> RequestImpl::Header() const noexcept
{
    return headers_;
}

//------------------------------------------------------------------------------
Request::Method RequestImpl::GetMethod() const noexcept
{
    return method_;
}

//------------------------------------------------------------------------------
shared_ptr<Data> RequestImpl::Data() const noexcept
{
    return data_;
}

//------------------------------------------------------------------------------
void RequestImpl::ReadInputBuffer() noexcept
{
    struct evbuffer *buf = evhttp_request_get_input_buffer(req_);

    auto length = evbuffer_get_length(buf);
    if (length > 0)
    {
        Logging::Debug("Размер данных: {}", length);

        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        const string str{reinterpret_cast<char *>(evbuffer_pullup(buf, -1)),
                         length};
        data_->Set(str);
    }
}

}  // namespace tasp::http
