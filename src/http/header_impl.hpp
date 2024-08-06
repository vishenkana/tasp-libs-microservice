/**
 * @file
 * @brief Реализация интерфейсов работы с заголовком HTTP.
 */
#ifndef TASP_HTTP_HEADER_IMPL_HPP_
#define TASP_HTTP_HEADER_IMPL_HPP_

#include <evhttp.h>

#include <map>

#include <tasp/http/header.hpp>

namespace tasp::http
{

/**
 * @brief Реализация интерфейса для работы с заголовком HTTP-запроса.
 */
class HeaderImpl : public Header
{
public:
    /**
     * @brief Конструктор.
     *
     * @param req Указатель на запрос или ответ в библиотеке libevent
     * @param type Тип заголовка
     */
    explicit HeaderImpl(evhttp_request *req,
                        Header::Type type = Header::Type::Input) noexcept;

    /**
     * @brief Деструктор.
     */
    ~HeaderImpl() noexcept override;

    /**
     * @brief Запрос параметра заголовка.
     *
     * @param name Название параметра
     *
     * @return Значение
     */
    [[nodiscard]] const std::string &Get(
        std::string_view name) const noexcept override;

    /**
     * @brief Установка нового значения параметра заголовка.
     *
     * @param name Название параметра
     * @param value Новое значение
     */
    void Set(std::string_view name, std::string_view value) noexcept override;

    HeaderImpl(const HeaderImpl &) = delete;
    HeaderImpl(HeaderImpl &&) = delete;
    HeaderImpl &operator=(const HeaderImpl &) = delete;
    HeaderImpl &operator=(HeaderImpl &&) = delete;

private:
    /**
     * @brief Указатель на заголовки.
     */
    evkeyvalq *ev_headers_{nullptr};

    /**
     * @brief Значения заголовка.
     */
    std::map<std::string, std::string> headers_;
};

}  // namespace tasp::http

#endif  // TASP_HTTP_HEADER_IMPL_HPP_
