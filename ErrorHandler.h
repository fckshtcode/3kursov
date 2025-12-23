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
    static void logError(const std::string& message);
    
    /**
     * @brief Логирует ошибку и завершает программу
     * @param message Сообщение об ошибке
     * @details Выводит сообщение об ошибке и завершает программу 
     *          с кодом EXIT_FAILURE
     */
    static void exitWithError(const std::string& message);
    
    /**
     * @brief Выводит справку по использованию программы
     * @details Отображает информацию о параметрах командной строки
     *          и способах использования клиента
     */
    static void printHelp();
};

#endif // ERRORHANDLER_H
