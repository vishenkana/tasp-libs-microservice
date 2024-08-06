#include "header_impl.hpp"

#include <string>

using std::string;
using std::string_view;

namespace tasp::http
{

/*------------------------------------------------------------------------------
    HeaderImpl
------------------------------------------------------------------------------*/
HeaderImpl::HeaderImpl(evhttp_request *req, Header::Type type) noexcept
{
    if (type == Header::Type::Input)
    {
        ev_headers_ = evhttp_request_get_input_headers(req);
    }
    else
    {
        ev_headers_ = evhttp_request_get_output_headers(req);
    }

    for (evkeyval *header = ev_headers_->tqh_first; header != nullptr;
         header = header->next.tqe_next)
    {
        headers_.insert_or_assign(header->key, header->value);
    }

    char *client_ip{};
    u_short client_port{};

    evhttp_connection_get_peer(
        evhttp_request_get_connection(req), &client_ip, &client_port);

    headers_.insert_or_assign("client", client_ip);
}

//------------------------------------------------------------------------------
HeaderImpl::~HeaderImpl() noexcept = default;

//------------------------------------------------------------------------------
const string &HeaderImpl::Get(string_view name) const noexcept
{
    auto param{headers_.find(name.data())};
    if (param != headers_.end())
    {
        return param->second;
    }

    static const string empty_value;
    return empty_value;
}

//------------------------------------------------------------------------------
void HeaderImpl::Set(string_view name, string_view value) noexcept
{
    if (headers_.find(name.data()) != headers_.end())
    {
        evhttp_remove_header(ev_headers_, name.data());
    }

    headers_.insert_or_assign(name.data(), value.data());
    evhttp_add_header(ev_headers_, name.data(), value.data());
}

}  // namespace tasp::http
