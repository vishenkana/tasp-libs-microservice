/**
 * @file
 * @brief Реализация интерфейсов работы с запросом HTTP.
 */
#ifndef TASP_HTTP_REQUEST_IMPL_HPP_
#define TASP_HTTP_REQUEST_IMPL_HPP_

#include <evhttp.h>
#include <jsoncpp/json/json.h>

#include <tasp/http/header.hpp>
#include <tasp/http/request.hpp>
#include <tasp/http/uri.hpp>

namespace tasp::http
{

/**
 * @brief Реализация интерфейса для работы с запросом HTTP.
 */
class RequestImpl : public Request
{
public:
    /**
     * @brief Конструктор.
     *
     * @param req Указатель на запрос в библиотеке libevent
     */
    explicit RequestImpl(evhttp_request *req) noexcept;

    /**
     * @brief Деструктор.
     */
    ~RequestImpl() noexcept override;

    /**
     * @brief Запрос указателя на ресурс.
     *
     * @return Указатель на ресурс
     */
    [[nodiscard]] std::shared_ptr<http::Uri> Uri() const noexcept override;

    /**
     * @brief Запрос заголовков запроса.
     *
     * @return Заголовок
     */
    [[nodiscard]] std::shared_ptr<http::Header> Header()
        const noexcept override;

    /**
     * @brief Запрос метода запроса.
     *
     * @return Метод
     */
    [[nodiscard]] Request::Method GetMethod() const noexcept override;

    /**
     * @brief Запрос данных запроса в текстовом представлении.
     *
     * @return Данные
     */
    [[nodiscard]] std::shared_ptr<http::Data> Data() const noexcept override;

    /**
     * @brief Чтение данные запроса из буфера библиотеки libevent во внутренний
     * буфер.
     */
    void ReadInputBuffer() noexcept;

    RequestImpl(const RequestImpl &) = delete;
    RequestImpl(RequestImpl &&) = delete;
    RequestImpl &operator=(const RequestImpl &) = delete;
    RequestImpl &operator=(RequestImpl &&) = delete;

private:
    /**
     * @brief Указатель на запрос в библиотеке libevent.
     */
    evhttp_request *req_;

    /**
     * @brief Указатель на ресурс.
     */
    std::shared_ptr<http::Uri> uri_;

    /**
     * @brief Метод запроса.
     */
    http::Request::Method method_;

    /**
     * @brief Заголовок запроса.
     */
    std::shared_ptr<http::Header> headers_;

    /**
     * @brief Данные запроса.
     */
    std::shared_ptr<http::Data> data_;
};

}  // namespace tasp::http

#endif  // TASP_HTTP_REQUEST_IMPL_HPP_
