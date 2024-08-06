/**
 * @file
 * @brief Реализация интерфейсов работы с ответом HTTP.
 */
#ifndef TASP_HTTP_RESPONSE_IMPL_HPP_
#define TASP_HTTP_RESPONSE_IMPL_HPP_

#include <evhttp.h>

#include <tasp/http/response.hpp>

#include "header_impl.hpp"

namespace tasp::http
{

/**
 * @brief Реализация интерфейса для работы с ответом HTTP.
 */
class ResponseImpl : public Response
{
public:
    /**
     * @brief Конструктор.
     *
     * @param req Указатель на ответ в библиотеке libevent
     */
    explicit ResponseImpl(evhttp_request *req) noexcept;

    /**
     * @brief Деструктор.
     */
    ~ResponseImpl() noexcept override;

    /**
     * @brief Установка кода ответа.
     *
     * @param code Код
     */
    void SetCode(Code code) noexcept override;

    /**
     * @brief Запрос кода ответа.
     *
     * @return Код
     */
    [[nodiscard]] Code GetCode() const noexcept override;

    /**
     * @brief Запрос заголовков ответа.
     *
     * @return Заголовок
     */
    [[nodiscard]] std::shared_ptr<http::Header> Header()
        const noexcept override;

    /**
     * @brief Запрос данных запроса в текстовом представлении.
     *
     * @return Данные
     */
    [[nodiscard]] std::shared_ptr<http::Data> Data() const noexcept override;

    /**
     * @brief Добавление в ответ сообщения об ошибке.
     *
     * @param code Код ответа
     * @param message Сообщение
     */
    void SetError(Code code, std::string_view message) noexcept override;

    /**
     * @brief Отправка ответа клиенту.
     */
    void Send() noexcept;

    ResponseImpl(const ResponseImpl &) = delete;
    ResponseImpl(ResponseImpl &&) = delete;
    ResponseImpl &operator=(const ResponseImpl &) = delete;
    ResponseImpl &operator=(ResponseImpl &&) = delete;

private:
    /**
     * @brief Указатель на ответ в библиотеке libevent.
     */
    evhttp_request *req_;

    /**
     * @brief Код ответа.
     */
    Response::Code code_{Response::Code::Ok};

    /**
     * @brief Заголовок ответа.
     */
    std::shared_ptr<http::Header> headers_;

    /**
     * @brief Данные запроса.
     */
    std::shared_ptr<http::Data> data_;
};

}  // namespace tasp::http

#endif  // TASP_HTTP_RESPONSE_IMPL_HPP_
