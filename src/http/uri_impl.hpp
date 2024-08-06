/**
 * @file
 * @brief Реализация интерфейсов работы с указателями на ресурс (URI).
 */
#ifndef TASP_HTTP_URI_IMPL_HPP_
#define TASP_HTTP_URI_IMPL_HPP_

#include <evhttp.h>

#include <unordered_map>
#include <vector>

#include <tasp/http/uri.hpp>

namespace tasp::http
{

/**
 * @brief Реализация интерфейса для работы с указателями на ресурс (URI).
 */
class UriImpl : public Uri
{
public:
    /**
     * @brief Конструктор.
     *
     * @param req Указатель на запрос в библиотеке libevent
     */
    explicit UriImpl(const evhttp_request *req) noexcept;

    /**
     * @brief Деструктор.
     */
    ~UriImpl() noexcept override;

    /**
     * @brief Запрос полного идентификатора ресурса.
     *
     * @return Путь
     */
    [[nodiscard]] const std::string &Url() const noexcept override;

    /**
     * @brief Запрос URL-пути.
     *
     * @return Путь
     */
    [[nodiscard]] const std::string &Path() const noexcept override;

    /**
     * @brief Смена пути запроса.
     *
     * @param path Путь запроса
     */
    void ChangePath(std::string_view path) noexcept override;

    /**
     * @brief Запрос значений параметра.
     *
     * @param name Название параметра
     *
     * @return Массив значений, переданных в URL
     */
    [[nodiscard]] url::ParamValueVector ParamValues(
        std::string_view name) const noexcept override;

    /**
     * @brief Проверка соответствия URL-пути регулярному выражению и
     * формирование подгрупп пути.
     *
     * @param expr Регулярное выражение
     *
     * @return Результат сравнения
     */
    [[nodiscard]] bool Match(std::string_view expr) noexcept override;

    /**
     * @brief Запрос значения подгруппы пути. Если подгруппа отсутствует,
     * возвращается пустое значение.
     *
     * @param number Номер подгруппы
     *
     * @return Значение подгруппы
     */
    [[nodiscard]] const std::string &SubMatch(
        size_t number) const noexcept override;

    /**
     * @brief Преобразование параметров запроса в SQL-условие (для конкатенации
     * с 'WHERE').
     *
     * @return SQL-условие.
     */
    [[nodiscard]] std::string ToSQLCondition() const noexcept override;

    UriImpl(const UriImpl &) = delete;
    UriImpl(UriImpl &&) = delete;
    UriImpl &operator=(const UriImpl &) = delete;
    UriImpl &operator=(UriImpl &&) = delete;

private:
    /**
     * @brief Параметры запроса.
     */
    std::unordered_multimap<std::string,
                            std::shared_ptr<tasp::http::url::ParamValue>>
        query_params_;

    /**
     * @brief Полный идентификатор ресурса.
     */
    std::string url_{"localhost"};

    /**
     * @brief URL-путь запроса.
     */
    std::string path_{"/"};

    /**
     * @brief Значения подгрупп пути.
     */
    std::vector<std::string> matches_;
};

}  // namespace tasp::http

#endif  // TASP_HTTP_URI_IMPL_HPP_
