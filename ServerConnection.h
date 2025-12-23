/**
 * @file ServerConnection.h
 * @brief Заголовочный файл класса ServerConnection
 * @author Ладыгин Пётр Александрович
 * @date 20.12.2025
 */

#ifndef SERVERCONNECTION_H
#define SERVERCONNECTION_H

#include <string>
#include <vector>
#include <cstdint>

/**
 * @class ServerConnection
 * @brief Класс для управления соединением с сервером
 * 
 * Обеспечивает установку TCP соединения, аутентификацию,
 * отправку и получение данных в текстовом и бинарном форматах.
 */
class ServerConnection {
private:
    int socketFD; ///< Дескриптор сокета
    
    /**
     * @brief Отправляет текстовое сообщение серверу
     * @param text Текст для отправки
     * @return true если отправка успешна, false в случае ошибки
     */
    bool sendText(const std::string& text);
    
    /**
     * @brief Принимает текстовое сообщение от сервера
     * @param text Ссылка для сохранения принятого текста
     * @return true если прием успешен, false в случае ошибки
     */
    bool receiveText(std::string& text);
    
    /**
     * @brief Отправляет бинарные данные серверу
     * @param data Указатель на данные
     * @param size Размер данных в байтах
     * @return true если отправка успешна, false в случае ошибки
     */
    bool sendBinaryData(const void* data, size_t size);
    
    /**
     * @brief Принимает бинарные данные от сервера
     * @param data Указатель на буфер для данных
     * @param size Ожидаемый размер данных в байтах
     * @return true если прием успешен, false в случае ошибки
     */
    bool receiveBinaryData(void* data, size_t size);
    
public:
    /**
     * @brief Конструктор по умолчанию
     */
    ServerConnection();
    
    /**
     * @brief Деструктор
     * @details Автоматически закрывает соединение при уничтожении объекта
     */
    ~ServerConnection();
    
    /**
     * @brief Устанавливает соединение с сервером
     * @param address IP-адрес сервера
     * @param port Порт сервера
     * @return true если соединение установлено, false в случае ошибки
     */
    bool establishConnection(const std::string& address, int port);
    
    /**
     * @brief Выполняет аутентификацию на сервере
     * @param login Логин пользователя
     * @param password Пароль пользователя
     * @return true если аутентификация успешна, false в случае ошибки
     * @details Протокол аутентификации:
     *          1. Отправка LOGIN
     *          2. Получение SALT16 или ERR
     *          3. Вычисление и отправка HASH (MD5 от SALT || PASSWORD)
     *          4. Получение OK или сообщения об ошибке
     */
    bool authenticate(const std::string& login, const std::string& password);
    
    /**
     * @brief Отправляет векторы на сервер и получает результаты
     * @param vectors Векторы для отправки
     * @param results Ссылка для сохранения результатов
     * @return true если операция успешна, false в случае ошибки
     * @details Данные отправляются в формате little-endian
     */
    bool sendVectors(const std::vector<std::vector<uint32_t>>& vectors, std::vector<uint32_t>& results);
    
    /**
     * @brief Закрывает соединение с сервером
     */
    void closeConnection();
};

#endif