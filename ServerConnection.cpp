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
    // Создаем TCP сокет
    socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD < 0) {
        ErrorHandler::logError("Не удалось создать сокет");
        return false;
    }
    
    // Настраиваем структуру с адресом сервера
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr)); // Очищаем структуру
    serverAddr.sin_family = AF_INET; // Используем IPv4
    serverAddr.sin_port = htons(port); // Переводим порт в сетевой порядок байт
    
    // Конвертируем строку IP-адреса в бинарный формат
    if (inet_pton(AF_INET, address.c_str(), &serverAddr.sin_addr) <= 0) {
        ErrorHandler::logError("Неверный формат адреса сервера: " + address);
        return false;
    }
    
    // Пытаемся подключиться к серверу
    if (connect(socketFD, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        ErrorHandler::logError("Не удалось подключиться к серверу " + address + ":" + std::to_string(port));
        return false;
    }
    
    std::cout << "Лог: Соединение с сервером " << address << ":" << port << " успешно установлено" << std::endl;
    return true;
}

/**
 * @brief Отправляет текстовое сообщение серверу
 * @param text Текст для отправки
 * @return true если отправка успешна, false в случае ошибки
 */
bool ServerConnection::sendText(const std::string& text) {
    // Добавляем символ переноса строки в конец сообщения
    std::string message = text + "\n";
    // Отправляем данные в сеть
    ssize_t bytesSent = send(socketFD, message.c_str(), message.length(), 0);
    
    if (bytesSent != static_cast<ssize_t>(message.length())) {
        ErrorHandler::logError("Ошибка при отправке текста: " + text);
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
    // Буфер для получения текстового ответа
    char buffer[1024];
    ssize_t bytesReceived = recv(socketFD, buffer, sizeof(buffer) - 1, 0);
    
    if (bytesReceived <= 0) {
        ErrorHandler::logError("Ошибка получения текста от сервера");
        return false;
    }
    
    // Ставим нуль-терминатор для корректной работы со строкой
    buffer[bytesReceived] = '\0';
    text = buffer;
    
    // Отрезаем символ переноса строки, если он есть
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
    ssize_t totalSent = 0; // Счетчик отправленных байт
    const char* dataPtr = static_cast<const char*>(data); // Приводим к массиву байт
    
    // Цикл гарантирует отправку всех байт
    while (totalSent < static_cast<ssize_t>(size)) {
        // Отправляем оставшуюся часть данных
        ssize_t sent = send(socketFD, dataPtr + totalSent, size - totalSent, 0);
        if (sent <= 0) {
            ErrorHandler::logError("Ошибка при отправке бинарных данных");
            return false;
        }
        totalSent += sent; // Обновляем счетчик
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
    ssize_t totalReceived = 0; // Счетчик полученных байт
    char* dataPtr = static_cast<char*>(data);
    
    // Цикл гарантирует получение точного количества байт
    while (totalReceived < static_cast<ssize_t>(size)) {
        ssize_t received = recv(socketFD, dataPtr + totalReceived, size - totalReceived, 0);
        if (received <= 0) {
            ErrorHandler::logError("Ошибка при получении бинарных данных");
            return false;
        }
        totalReceived += received; // Обновляем счетчик
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
    // 1. Отправка логина (шаг из ТЗ)
    std::cout << "Лог: Отправка логина: " << login << std::endl;
    if (!sendText(login)) {
        ErrorHandler::logError("Не получилось отправить логин");
        return false;
    }
    
    // 2. Ждем от сервера ответ: соль (16 символов) или слово ERR
    std::string saltResponse;
    if (!receiveText(saltResponse)) {
        ErrorHandler::logError("Сервер ничего не ответил на логин");
        return false;
    }
    
    if (saltResponse == "ERR") {
        ErrorHandler::logError("Сервер не узнал этот логин (ответил ERR)");
        return false;
    }
    
    if (saltResponse.length() != 16) {
        ErrorHandler::logError("Сервер прислал соль странной длины (не 16 символов): " + saltResponse);
        return false;
    }
    
    // 3. Высчитываем хеш-пароль (смешиваем полученную соль и наш пароль через MD5)
    std::string hash = Authenticator::computeHash(saltResponse, password);
    std::cout << "Лог: Отправка зашифрованного пароля (хеша): " << hash << std::endl;
    
    if (!sendText(hash)) {
        ErrorHandler::logError("Не получилось отправить хеш-пароль");
        return false;
    }
    
    // 4. Ждем финальный ответ: пустил нас сервер (OK) или нет
    std::string authResponse;
    if (!receiveText(authResponse)) {
        ErrorHandler::logError("Сервер не ответил после отправки пароля");
        return false;
    }
    
    if (authResponse != "OK") {
        ErrorHandler::logError("Сервер отклонил пароль (ответ: " + authResponse + ")");
        return false;
    }
    
    std::cout << "Лог: Мы успешно авторизовались на сервере!" << std::endl;
    return true;
}

/**
 * @brief Отправляет векторы на сервер и получает результаты
 * @param vectors Векторы для отправки
 * @param results Ссылка для сохранения результатов
 * @return true если операция успешна, false в случае ошибки
 */
bool ServerConnection::sendVectors(const std::vector<std::vector<uint32_t>>& vectors, std::vector<uint32_t>& results) {
    // Очищаем вектор результатов и резервируем память
    results.clear();
    results.reserve(vectors.size());
    
    // 6. Отправка количества векторов
    uint32_t numVectors = static_cast<uint32_t>(vectors.size());
    
    // Отладочная информация
    std::cout << "Отладка: Отправляем " << numVectors << " векторов" << std::endl;
    std::cout << "Отладка: Байты количества векторов (шестнадцатеричный вид памяти): ";
    unsigned char* bytes = reinterpret_cast<unsigned char*>(&numVectors);
    for (size_t i = 0; i < sizeof(numVectors); i++) {
        printf("%02X ", bytes[i]);
    }
    std::cout << std::endl;
    
    // Отправляем число без изменения порядка байт (как лежит в памяти)
    // Сервер ожидает данные именно в таком виде
    if (!sendBinaryData(&numVectors, sizeof(numVectors))) {
        ErrorHandler::logError("Ошибка отправки количества векторов");
        return false;
    }
    
    // 7-10. Обрабатываем каждый вектор
    for (size_t i = 0; i < vectors.size(); ++i) {
        const auto& vec = vectors[i];
        uint32_t vecSize = static_cast<uint32_t>(vec.size());
        
        std::cout << "Отладка: Вектор " << i << ", его размер: " << vecSize << std::endl;
        std::cout << "Отладка: Байты размера вектора: ";
        bytes = reinterpret_cast<unsigned char*>(&vecSize);
        for (size_t j = 0; j < sizeof(vecSize); j++) {
            printf("%02X ", bytes[j]);
        }
        std::cout << std::endl;
        
        // 7. Отправляем размер вектора
        if (!sendBinaryData(&vecSize, sizeof(vecSize))) {
            ErrorHandler::logError("Ошибка отправки размера вектора " + std::to_string(i));
            return false;
        }
        
        // 8. Отправляем элементы вектора
        if (vecSize > 0) {
            if (i == 0 && vecSize > 0) {
                std::cout << "Отладка: Первые 2 значения вектора 0: ";
                for (size_t j = 0; j < std::min(vecSize, (uint32_t)2); ++j) {
                    std::cout << vec[j] << " ";
                }
                std::cout << std::endl;
                
                if (vecSize > 0) {
                    std::cout << "Отладка: Байты первого значения: ";
                    bytes = reinterpret_cast<unsigned char*>(const_cast<uint32_t*>(&vec[0]));
                    for (size_t j = 0; j < sizeof(uint32_t); j++) {
                        printf("%02X ", bytes[j]);
                    }
                    std::cout << std::endl;
                }
            }
            
            // Отправляем весь вектор одним бинарным блоком
            if (!sendBinaryData(vec.data(), vecSize * sizeof(uint32_t))) {
                ErrorHandler::logError("Ошибка отправки элементов вектора " + std::to_string(i));
                return false;
            }
        }
        
        // 9. Получаем результат обработки текущего вектора от сервера
        uint32_t result;
        if (!receiveBinaryData(&result, sizeof(result))) {
            ErrorHandler::logError("Ошибка получения результата для вектора " + std::to_string(i));
            return false;
        }
        
        // Сохраняем результат в вектор
        results.push_back(result);
        
        std::cout << "Отладка: Результат вычислений для вектора " << i << ": " << result << std::endl;
    }
    
    std::cout << "Лог: Успешно обработано " << vectors.size() << " векторов" << std::endl;
    return true;
}

/**
 * @brief Закрывает соединение с сервером
 */
void ServerConnection::closeConnection() {
    if (socketFD >= 0) { // Проверяем, открыт ли сокет
        std::cout << "Лог: Закрытие соединения с сервером" << std::endl;
        close(socketFD); // Системный вызов для закрытия сокета
        socketFD = -1; // Сбрасываем дескриптор в начальное состояние
    }
}
