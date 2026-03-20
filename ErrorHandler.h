/**
 * @file ErrorHandler.h
 * @brief Заголовочный файл класса ErrorHandler
 * @author Ладыгин Пётр Александрович
 * @date 20.12.2025
 */

#ifndef ERRORHANDLER_H
#define ERRORHANDLER_H

#include <string>

/**
 * @class ErrorHandler
 * @brief Класс для обработки ошибок и вывода сообщений
 * 
 * Предоставляет статические методы для логирования ошибок, 
 * аварийного завершения программы и вывода справки.
 * Все методы являются статическими, создание экземпляра не требуется.
 */
class ErrorHandler {
public:
    /**
     * @brief Логирует сообщение об ошибке
     * @param message Сообщение об ошибке для вывода
     * @details Выводит сообщение в стандартный поток ошибок (stderr)
     *          с префиксом "[ОШИБКА]"
     */
    static void logErrorImpl(const std::string& message, const char* funcName);
    
    /**
     * @brief Логирует ошибку и завершает программу
     * @param message Сообщение об ошибке
     * @param funcName Имя функции, в которой произошла ошибка
     * @details Выводит сообщение об ошибке и завершает программу 
     *          с кодом EXIT_FAILURE
     */
    static void exitWithErrorImpl(const std::string& message, const char* funcName);
    
    /**
     * @brief Выводит справку по использованию программы
     * @details Отображает информацию о параметрах командной строки
     *          и способах использования клиента
     */
    static void printHelp();
};

// Макросы для автоматического захвата имени функции
#define logError(msg) logErrorImpl(msg, __func__)
#define exitWithError(msg) exitWithErrorImpl(msg, __func__)

#endif // ERRORHANDLER_H
