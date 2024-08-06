/**
 * @file
 * @brief Интерфейсы для создания микросервисов.
 */
#ifndef TASP_MICROSERVICE_HPP_
#define TASP_MICROSERVICE_HPP_

#include <functional>
#include <memory>
#include <string_view>
#include <vector>

#include <tasp/health.hpp>
#include <tasp/http/request.hpp>
#include <tasp/http/response.hpp>

namespace tasp
{

class MicroServiceImpl;

/**
 * @brief Формат функции обработчика запросов.
 */
using Handler = std::function<void(const http::Request &, http::Response &)>;

/**
 * @brief Формат функции проверки состояния компонента микросервиса.
 */
using CheckFunction = std::function<HealthReport()>;

/**
 * @brief Интерфейс основного класса микросервиса.
 *
 * Класс скрывает от пользователя реализацию с помощью идиомы PIMPL
 * (Pointer to Implementation – указатель на реализацию).
 *
 */
class [[gnu::visibility("default")]] MicroService
{
public:
    /**
     * @brief Конструктор.
     *
     * @param argc Количество аргументов
     * @param argv Аргументы
     */
    MicroService(int argc, const char **argv) noexcept;

    /**
     * @brief Деструктор.
     */
    virtual ~MicroService() noexcept;

    /**
     * @brief Запуск микросервиса.
     *
     * @return Код завершения. 0 - при нормальном завершении.
     */
    [[nodiscard]] int Exec() const noexcept;

    /**
     * @brief Установка обработчика запроса в виде статической функции, лямбды.
     *
     * @param method Метод
     * @param path Путь запроса
     * @param func Обработчик
     */
    void AddHandler(http::Request::Method method,
                    std::string_view path,
                    const Handler &func) const noexcept;

    /**
     * @brief Установка обработчика запроса в виде функции члена класса.
     *
     * @param method Метод
     * @param path Путь запроса
     * @param object Объект класса
     * @param func Обработчик
     */
    template<typename Name>
    void AddHandler(http::Request::Method method,
                    std::string_view path,
                    Name *const object,
                    void (Name::*const func)(const http::Request &,
                                             http::Response &)) const noexcept
    {
        AddHandler(
            method,
            path,
            // NOLINTNEXTLINE(modernize-avoid-bind)
            std::bind(
                func, object, std::placeholders::_1, std::placeholders::_2));
    }

    MicroService(const MicroService &) = delete;
    MicroService(MicroService &&) = delete;
    MicroService &operator=(const MicroService &) = delete;
    MicroService &operator=(MicroService &&) = delete;

protected:
    /**
     * @brief Установка проверок состояния компонентов микросервиса.
     *
     * @param check_functions Набор проверок состояния компонентов микросервиса.
     */
    void AddCheckFunctions(
        const std::vector<CheckFunction> &check_functions) noexcept;

private:
    /**
     * @brief Указатель на реализацию.
     */
    std::unique_ptr<MicroServiceImpl> impl_;
};

}  // namespace tasp

#endif  // TASP_MICROSERVICE_HPP_
