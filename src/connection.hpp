/**
 * @file
 * @brief Классы HTTP-сервера.
 */
#ifndef TASP_CONNECTION_HPP_
#define TASP_CONNECTION_HPP_

#include <evhttp.h>

#include <string>
#include <thread>

#include "tasp/microservice.hpp"

namespace tasp::ev
{
class Connection;
class HandlerImpl;

/**
 * @brief Умный указатель конфигурации библиотеки libevent.
 */
using EvConf = std::unique_ptr<event_config, decltype(&event_config_free)>;

/**
 * @brief Умный указатель основной структуры библиотеки libevent.
 */
using EvBase = std::unique_ptr<event_base, decltype(&event_base_free)>;

/**
 * @brief Умный указатель HTTP-сервера библиотеки libevent.
 */
using EvHttp = std::unique_ptr<evhttp, decltype(&evhttp_free)>;

/**
 * @brief Умный указатель события библиотеки libevent.
 */
using EvEvent = std::unique_ptr<event, decltype(&event_free)>;

/**
 * @brief Класс подключения к HTTP-серверу.
 *
 */
class Connection final
{
public:
    /**
     * @brief Конструктор главного подключения.
     *
     * @param address Адрес для прослушивания
     * @param port Порт сервера
     * @param func Функция формирующая запрос
     */
    Connection(std::string_view address,
               uint16_t port,
               const Handler &func) noexcept;

    /**
     * @brief Конструктор дополнительных подключения.
     *
     * @param socket Сокет основного подключения
     * @param func Функция формирующая запрос
     */
    Connection(evutil_socket_t socket, const Handler &func) noexcept;

    /**
     * @brief Деструктор.
     */
    ~Connection() noexcept;

    /**
     * @brief Запрос сокета подключения.
     *
     * @return Сокет
     */
    evutil_socket_t GetSocket() const noexcept;

    /**
     * @brief Конструктор копирования.
     *
     * @param conn Подключение для копирования
     */
    Connection(Connection &&conn) = default;

    Connection(const Connection &) = delete;
    Connection &operator=(const Connection &) = delete;
    Connection &operator=(Connection &&) = delete;

private:
    /**
     * @brief Конструктор с общими действиями как для основного соединения, так
     * и для дополнительных соединений.
     *
     * @param func Функция формирующая запрос
     */
    explicit Connection(Handler func) noexcept;

    /**
     * @brief Обработчик запроса передаваемый в библиотеку libevent.
     *
     * @param req Запрос
     * @param arg Дополнительный аргумент, указатель на подключение.
     */
    static void Request(evhttp_request *req, void *arg) noexcept;

    /**
     * @brief Поток для обработки событий подключения.
     */
    std::unique_ptr<std::thread> thread_{nullptr};

    /**
     * @brief Главный цикл обработки событий.
     */
    EvBase event_{nullptr, nullptr};

    /**
     * @brief Http-сервер.
     */
    EvHttp server_{nullptr, nullptr};

    /**
     * @brief Событие для отслеживания выхода.
     */
    EvEvent event_exit_{nullptr, nullptr};

    /**
     * @brief Сокет подключения.
     */
    evutil_socket_t socket_{};

    /**
     * @brief Функция формирующая запрос.
     */
    Handler func_;
};

/**
 * @brief Класс с обработчиками запросов HTTP-сервера.
 *
 */
class HandlerImpl final
{
public:
    /**
     * @brief Конструктор.
     *
     * @param method Метод запроса
     * @param path URL-путь запроса
     * @param func Обработчик запроса
     */
    HandlerImpl(http::Request::Method method,
                std::string_view path,
                Handler func) noexcept;

    /**
     * @brief Деструктор.
     */
    ~HandlerImpl() noexcept;

    /**
     * @brief Запрос метода обработчика.
     *
     * @return Метод
     */
    [[nodiscard]] http::Request::Method Method() const noexcept;

    /**
     * @brief Запрос URL-пути обработчика.
     *
     * @return Путь
     */
    [[nodiscard]] const std::string &Path() const noexcept;

    /**
     * @brief Вызов обработчика запроса.
     *
     * @param request Запрос
     * @param response Ответ
     */
    void Exec(const http::Request &request, http::Response &response) noexcept;

    /**
     * @brief Конструктор перемещения.
     *
     * @param handler Обработчик для перемещения
     */
    HandlerImpl(HandlerImpl &&handler) = default;

    HandlerImpl(const HandlerImpl &) = delete;
    HandlerImpl &operator=(const HandlerImpl &) = delete;
    HandlerImpl &operator=(HandlerImpl &&) = delete;

private:
    /**
     * @brief Метод запроса.
     */
    http::Request::Method method_;

    /**
     * @brief URL-путь запроса.
     */
    std::string path_;

    /**
     * @brief Обработчик запроса.
     */
    Handler func_;
};

}  // namespace tasp::ev

#endif  // TASP_CONNECTION_HPP_
