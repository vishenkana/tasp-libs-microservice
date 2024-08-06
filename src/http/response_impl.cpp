#include "response_impl.hpp"

#include <tasp/logging.hpp>

using std::make_shared;
using std::shared_ptr;
using std::string;
using std::string_view;

namespace tasp::http
{

/**
 * @brief Умный указатель буфера данных библиотеки libevent.
 */
using EvBuffer = std::unique_ptr<evbuffer, decltype(&evbuffer_free)>;

/*------------------------------------------------------------------------------
    ResponseImpl
------------------------------------------------------------------------------*/
ResponseImpl::ResponseImpl(evhttp_request *req) noexcept
: req_(req)
, headers_(make_shared<HeaderImpl>(req_, Header::Type::Output))
, data_(make_shared<http::Data>())
{
    headers_->Set("Access-Control-Allow-Origin", "*");
}

//------------------------------------------------------------------------------
ResponseImpl::~ResponseImpl() noexcept = default;

//------------------------------------------------------------------------------
void ResponseImpl::SetCode(Code code) noexcept
{
    code_ = code;
}

//------------------------------------------------------------------------------
Response::Code ResponseImpl::GetCode() const noexcept
{
    return code_;
}

//------------------------------------------------------------------------------
shared_ptr<Header> ResponseImpl::Header() const noexcept
{
    return headers_;
}

//------------------------------------------------------------------------------
shared_ptr<Data> ResponseImpl::Data() const noexcept
{
    return data_;
}

//------------------------------------------------------------------------------
void ResponseImpl::SetError(Code code, string_view message) noexcept
{
    SetCode(code);

    Json::Value root;
    root["message"] = message.data();
    Data()->Set(root);
}

//------------------------------------------------------------------------------
void ResponseImpl::Send() noexcept
{
    Logging::Info("HTTP-ответ {} клиенту {}",
                  static_cast<int>(code_),
                  headers_->Get("client"));

    headers_->Set("Content-Type", data_->GetType() + "; charset=UTF-8");

    const EvBuffer buffer{evbuffer_new(), evbuffer_free};

    auto data = data_->Get<string>();
    evbuffer_add(buffer.get(), data.data(), data.length());

    evhttp_send_reply(req_, static_cast<int>(code_), nullptr, buffer.get());
}

}  // namespace tasp::http
