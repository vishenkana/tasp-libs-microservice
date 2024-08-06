/**
 * @file
 * @brief Реализация интерфейса для создания микросервисов.
 */
#ifndef TASP_MICROSERVICE_IMPL_HPP_
#define TASP_MICROSERVICE_IMPL_HPP_

#include <string_view>
#include <vector>

#include <tasp/config.hpp>
#include <tasp/daemon.hpp>
#include <tasp/health.hpp>
#include <tasp/microservice.hpp>

#include "connection.hpp"

namespace tasp
{

/**
 * @brief Реализация интерфейса основного класса микросервиса.
 *
 */
class MicroServiceImpl final : public Daemon
{
public:
    /**
     * @brief Конструктор.
     *
     * @param argc Количество аргументов
     * @param argv Аргументы
     */
    MicroServiceImpl(int argc, const char **argv) noexcept;

    /**
     * @brief Деструктор.
     */
    ~MicroServiceImpl() noexcept override;

    /**
     * @brief Обновление конфигурации.
     */
    void Reload() noexcept override;

    /**
     * @brief Установка обработчика запроса.
     *
     * @param method Метод
     * @param path Путь запроса
     * @param func Обработчик
     */
    void AddHandler(http::Request::Method method,
                    std::string_view path,
                    const Handler &func) noexcept;

    /**
     * @brief Установка проверок состояния компонентов микросервиса.
     *
     * @param check_functions Набор проверок состояния компонентов микросервиса
     */
    void AddCheckFunctions(
        const std::vector<CheckFunction> &check_functions) noexcept;

    MicroServiceImpl(const MicroServiceImpl &) = delete;
    MicroServiceImpl(MicroServiceImpl &&) = delete;
    MicroServiceImpl &operator=(const MicroServiceImpl &) = delete;
    MicroServiceImpl &operator=(MicroServiceImpl &&) = delete;

private:
    /**
     * @brief Главный обработчик запросов. Производит поиск обработчика на
     * запрос и вызывает обработчик.
     *
     * @param request Запрос
     * @param response Ответ
     */
    void Request(const http::Request &request,
                 http::Response &response) noexcept;

    /**
     * @brief Установка обработчика запроса состояния работоспособности
     * микросервиса (GET /health).
     */
    void AddHealthHandler() noexcept;

    /**
     * @brief Функция проверки работоспособности
     * микросервиса.
     *
     * Вызывается каждый раз при обработке запроса GET /health.
     *
     * @return Отчёт о состоянии микросервиса.
     */
    [[nodiscard]] Json::Value HealthCheck() const noexcept;

    /**
     * @brief Проверка прав доступа к директориям.
     *
     * Необходимые для проверки пути директорий и соответствующие им права
     * доступа указываются в общем конфигурационом файле.
     *
     * @param dirs_ids Конфигурация из путей и соответствующих им прав доступа
     *
     * @return Отчёт о проверке прав доступа к директориям
     */
    [[nodiscard]] static HealthReport DirectoriesCheck(
        const std::vector<std::string> &dirs_ids) noexcept;

    /**
     * @brief Набор проверок состояния компонентов микросервиса.
     */
    std::vector<CheckFunction> check_functions_;

    /**
     * @brief Установка проверок состояния компонентов микросервиса по
     * умолчанию.
     */
    void AddDefaultCheckFunctions() noexcept;

    /**
     * @brief Имя микросервиса.
     */
    std::string name_{"Неизвестный"};

    /**
     * @brief Префикс пути.
     */
    std::string prefix_{"/api/v1"};

    /**
     * @brief Список подключений к серверу.
     */
    std::vector<ev::Connection> pool_;

    /**
     * @brief Список обработчиков.
     */
    std::vector<ev::HandlerImpl> handlers_;
};

}  // namespace tasp

#endif  // TASP_MICROSERVICE_IMPL_HPP_
