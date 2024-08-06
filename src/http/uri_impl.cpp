#include "uri_impl.hpp"

#include <regex>

using std::regex;
using std::regex_match;
using std::smatch;
using std::string;
using std::string_view;

namespace tasp::http
{
/*------------------------------------------------------------------------------
    UriImpl
------------------------------------------------------------------------------*/
UriImpl::UriImpl(const evhttp_request *req) noexcept
{
    const auto *uri = evhttp_request_get_evhttp_uri(req);

    const auto *url{evhttp_request_get_uri(req)};
    if (url != nullptr)
    {
        url_ = url;
    }

    const auto *path{evhttp_uri_get_path(uri)};
    if (uri != nullptr)
    {
        path_ = path;
    }

    evkeyvalq params{};
    evhttp_parse_query_str(evhttp_uri_get_query(uri), &params);

    for (evkeyval *param = params.tqh_first; param != nullptr;
         param = param->next.tqe_next)
    {
        query_params_.insert(
            {param->key,
             std::make_shared<tasp::http::url::ParamValue>(param->value)});
    }

    evhttp_clear_headers(&params);
}

//------------------------------------------------------------------------------
UriImpl::~UriImpl() noexcept = default;

//------------------------------------------------------------------------------
const string &UriImpl::Url() const noexcept
{
    return url_;
}

//------------------------------------------------------------------------------
const string &UriImpl::Path() const noexcept
{
    return path_;
}

//------------------------------------------------------------------------------
void UriImpl::ChangePath(string_view path) noexcept
{
    path_ = path;
}

//------------------------------------------------------------------------------
url::ParamValueVector UriImpl::ParamValues(string_view name) const noexcept
{
    url::ParamValueVector param_values;

    auto param_values_range = query_params_.equal_range(name.data());
    for (auto param_value_it = param_values_range.first;
         param_value_it != param_values_range.second;
         param_value_it++)
    {
        param_values.push_back(param_value_it->second);
    }

    return param_values;
}

//------------------------------------------------------------------------------
bool UriImpl::Match(string_view expr) noexcept
{
    smatch pieces_match;
    auto res = regex_match(path_, pieces_match, regex(expr.data()));
    if (res)
    {
        matches_.reserve(pieces_match.size());
        for (auto &&match : pieces_match)
        {
            matches_.push_back(match.str());
        }
    }

    return res;
}

//------------------------------------------------------------------------------
const string &UriImpl::SubMatch(size_t number) const noexcept
{
    if (number >= matches_.size())
    {
        static const string empty_value;
        return empty_value;
    }

    return matches_.at(number);
}

//------------------------------------------------------------------------------
std::string UriImpl::ToSQLCondition() const noexcept
{
    std::string sql_condition;

    // filter
    auto param_values_range = query_params_.equal_range("filter");
    for (auto param_value_it = param_values_range.first;
         param_value_it != param_values_range.second;
         param_value_it++)
    {
        sql_condition += param_value_it->second->ToSQLCondition();
        if (std::next(param_value_it) != param_values_range.second)
        {
            sql_condition += " AND ";
        }
    }

    return sql_condition;
}

}  // namespace tasp::http
