/**
 * @file ServerConnection.cpp
 * @brief Реализация класса ServerConnection
 * @author Ладыгин Пётр Александрович
 * @date 20.12.2025
 */

#include "ServerConnection.h"
#include "ErrorHandler.h"
#include "Authenticator.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

/**
 * @brief Конструктор по умолчанию
 */
ServerConnection::ServerConnection() : socketFD(-1) {}

/**
 * @brief Деструктор
 */
ServerConnection::~ServerConnection() {
    closeConnection();
}

/**
 * @brief Устанавливает соединение с сервером
 * @param address IP-адрес сервера
 * @param port Порт сервера
 * @return true если соединение установлено, false в случае ошибки
 */
bool ServerConnection::establishConnection(const std::string& address, int port) {
    socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD < 0) {
        ErrorHandler::logError("Ошибка создания сокета");
        return false;
    }
    
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, address.c_str(), &serverAddr.sin_addr) <= 0) {
        ErrorHandler::logError("Неверный адрес сервера: " + address);
        return false;
    }
    
    if (connect(socketFD, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        ErrorHandler::logError("Не удалось подключиться к серверу " + address + ":" + std::to_string(port));
        return false;
    }
    
    std::cout << "Лог: Установлено соединение с " << address << ":" << port << std::endl;
    return true;
}

/**
 * @brief Отправляет текстовое сообщение серверу
 * @param text Текст для отправки
 * @return true если отправка успешна, false в случае ошибки
 */
bool ServerConnection::sendText(const std::string& text) {
    std::string message = text + "\n";
    ssize_t bytesSent = send(socketFD, message.c_str(), message.length(), 0);
    
    if (bytesSent != static_cast<ssize_t>(message.length())) {
        ErrorHandler::logError("Ошибка отправки текста: " + text);
        return false;
    }
    
    return true;
}

/**
 * @brief Принимает текстовое сообщение от сервера
 * @param text Ссылка для сохранения принятого текста
 * @return true если прием успешен, false в случае ошибки
 */
bool ServerConnection::receiveText(std::string& text) {
    char buffer[1024];
    ssize_t bytesReceived = recv(socketFD, buffer, sizeof(buffer) - 1, 0);
    
    if (bytesReceived <= 0) {
        ErrorHandler::logError("Ошибка получения текста от сервера");
        return false;
    }
    
    buffer[bytesReceived] = '\0';
    text = buffer;
    
    size_t newlinePos = text.find('\n');
    if (newlinePos != std::string::npos) {
        text = text.substr(0, newlinePos);
    }
    
    return true;
}

/**
 * @brief Отправляет бинарные данные серверу
 * @param data Указатель на данные
 * @param size Размер данных в байтах
 * @return true если отправка успешна, false в случае ошибки
 */
bool ServerConnection::sendBinaryData(const void* data, size_t size) {
    ssize_t totalSent = 0;
    const char* dataPtr = static_cast<const char*>(data);
    
    while (totalSent < static_cast<ssize_t>(size)) {
        ssize_t sent = send(socketFD, dataPtr + totalSent, size - totalSent, 0);
        if (sent <= 0) {
            ErrorHandler::logError("Ошибка отправки бинарных данных");
            return false;
        }
        totalSent += sent;
    }
    
    return true;
}

/**
 * @brief Принимает бинарные данные от сервера
 * @param data Указатель на буфер для данных
 * @param size Ожидаемый размер данных в байтах
 * @return true если прием успешен, false в случае ошибки
 */
bool ServerConnection::receiveBinaryData(void* data, size_t size) {
    ssize_t totalReceived = 0;
    char* dataPtr = static_cast<char*>(data);
    
    while (totalReceived < static_cast<ssize_t>(size)) {
        ssize_t received = recv(socketFD, dataPtr + totalReceived, size - totalReceived, 0);
        if (received <= 0) {
            ErrorHandler::logError("Ошибка получения бинарных данных");
            return false;
        }
        totalReceived += received;
    }
    
    return true;
}

/**
 * @brief Выполняет аутентификацию на сервере
 * @param login Логин пользователя
 * @param password Пароль пользователя
 * @return true если аутентификация успешна, false в случае ошибки
 */
bool ServerConnection::authenticate(const std::string& login, const std::string& password) {
    // 1. Отправка LOGIN
    std::cout << "Лог: Отправка LOGIN: " << login << std::endl;
    if (!sendText(login)) {
        ErrorHandler::logError("Ошибка отправки LOGIN");
        return false;
    }
    
    // 2. Получение SALT16 или ERR
    std::string saltResponse;
    if (!receiveText(saltResponse)) {
        ErrorHandler::logError("Ошибка получения ответа от сервера");
        return false;
    }
    
    if (saltResponse == "ERR") {
        ErrorHandler::logError("Сервер отверг идентификацию");
        return false;
    }
    
    if (saltResponse.length() != 16) {
        ErrorHandler::logError("Неверный формат SALT16: " + saltResponse);
        return false;
    }
    
    // 3. Вычисление и отправка HASH (MD5 от SALT || PASSWORD)
    std::string hash = Authenticator::computeHash(saltResponse, password);
    std::cout << "Лог: Отправка HASH: " << hash << std::endl;
    
    if (!sendText(hash)) {
        ErrorHandler::logError("Ошибка отправки HASH");
        return false;
    }
    
    // 4. Получение ответа об аутентификации
    std::string authResponse;
    if (!receiveText(authResponse)) {
        ErrorHandler::logError("Ошибка получения ответа аутентификации");
        return false;
    }
    
    if (authResponse != "OK") {
        ErrorHandler::logError("Ошибка аутентификации: " + authResponse);
        return false;
    }
    
    std::cout << "Лог: Аутентификация успешна" << std::endl;
    return true;
}

/**
 * @brief Отправляет векторы на сервер и получает результаты
 * @param vectors Векторы для отправки
 * @param results Ссылка для сохранения результатов
 * @return true если операция успешна, false в случае ошибки
 */
bool ServerConnection::sendVectors(const std::vector<std::vector<uint32_t>>& vectors, std::vector<uint32_t>& results) {
    results.clear();
    results.reserve(vectors.size());
    
    // 6. Отправка количества векторов
    uint32_t numVectors = static_cast<uint32_t>(vectors.size());
    
    // Отладочная информация - показываем что отправляем
    std::cout << "Отладка: Отправляем " << numVectors << " векторов" << std::endl;
    std::cout << "Отладка: Байты количества векторов (hex little-endian): ";
    unsigned char* bytes = reinterpret_cast<unsigned char*>(&numVectors);
    for (size_t i = 0; i < sizeof(numVectors); i++) {
        printf("%02X ", bytes[i]);
    }
    std::cout << std::endl;
    
    // ВАЖНО: отправляем КАК ЕСТЬ (little-endian), без htonl!
    // Сервер ожидает little-endian данные
    if (!sendBinaryData(&numVectors, sizeof(numVectors))) {
        ErrorHandler::logError("Ошибка отправки количества векторов");
        return false;
    }
    
    // 7-10. Для каждого вектора
    for (size_t i = 0; i < vectors.size(); ++i) {
        const auto& vec = vectors[i];
        uint32_t vecSize = static_cast<uint32_t>(vec.size());
        
        std::cout << "Отладка: Вектор " << i << ", размер: " << vecSize << std::endl;
        std::cout << "Отладка: Байты размера вектора (hex little-endian): ";
        bytes = reinterpret_cast<unsigned char*>(&vecSize);
        for (size_t j = 0; j < sizeof(vecSize); j++) {
            printf("%02X ", bytes[j]);
        }
        std::cout << std::endl;
        
        // 7. Отправка размера вектора (КАК ЕСТЬ, без htonl!)
        if (!sendBinaryData(&vecSize, sizeof(vecSize))) {
            ErrorHandler::logError("Ошибка отправки размера вектора " + std::to_string(i));
            return false;
        }
        
        // 8. Отправка значений вектора (КАК ЕСТЬ, без htonl!)
        if (vecSize > 0) {
            // Выводим первые значения для отладки
            if (i == 0 && vecSize > 0) {
                std::cout << "Отладка: Первые 2 значения вектора 0: ";
                for (size_t j = 0; j < std::min(vecSize, (uint32_t)2); ++j) {
                    std::cout << vec[j] << " ";
                }
                std::cout << std::endl;
                
                // Показываем байты первого значения
                if (vecSize > 0) {
                    std::cout << "Отладка: Байты первого значения (hex): ";
                    bytes = reinterpret_cast<unsigned char*>(const_cast<uint32_t*>(&vec[0]));
                    for (size_t j = 0; j < sizeof(uint32_t); j++) {
                        printf("%02X ", bytes[j]);
                    }
                    std::cout << std::endl;
                }
            }
            
            // Отправляем значения как есть (little-endian)
            if (!sendBinaryData(vec.data(), vecSize * sizeof(uint32_t))) {
                ErrorHandler::logError("Ошибка отправки значений вектора " + std::to_string(i));
                return false;
            }
        }
        
        // 9. Получение результата от сервера
        // ВАЖНО: результат приходит в little-endian
        uint32_t result;
        if (!receiveBinaryData(&result, sizeof(result))) {
            ErrorHandler::logError("Ошибка получения результата для вектора " + std::to_string(i));
            return false;
        }
        
        // Результат уже в правильном формате (little-endian)
        results.push_back(result);
        
        std::cout << "Отладка: Получен результат для вектора " << i << ": " << result << std::endl;
    }
    
    std::cout << "Лог: Успешно обработано " << vectors.size() << " векторов" << std::endl;
    return true;
}

/**
 * @brief Закрывает соединение с сервером
 */
void ServerConnection::closeConnection() {
    if (socketFD >= 0) {
        std::cout << "Лог: Закрытие соединения" << std::endl;
        close(socketFD);
        socketFD = -1;
    }
}