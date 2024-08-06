#include "tasp/microservice.hpp"

#include "microservice_impl.hpp"

using std::make_unique;
using std::string_view;

namespace tasp
{
/*------------------------------------------------------------------------------
    MicroService
------------------------------------------------------------------------------*/
MicroService::MicroService(int argc, const char **argv) noexcept
: impl_(make_unique<MicroServiceImpl>(argc, argv))
{
}

//------------------------------------------------------------------------------
MicroService::~MicroService() noexcept = default;

//------------------------------------------------------------------------------
int MicroService::Exec() const noexcept
{
    return impl_->Exec();
}

//------------------------------------------------------------------------------
void MicroService::AddHandler(http::Request::Method method,
                              string_view path,
                              const Handler &func) const noexcept
{
    impl_->AddHandler(method, path, func);
}

//------------------------------------------------------------------------------
void MicroService::AddCheckFunctions(
    const std::vector<CheckFunction> &check_functions) noexcept
{
    impl_->AddCheckFunctions(check_functions);
}

}  // namespace tasp
