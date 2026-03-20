/**
 * @file ErrorHandler.cpp
 * @brief Реализация класса ErrorHandler
 * @author Ладыгин Пётр Александрович
 * @date 20.12.2025
 */

#include "ErrorHandler.h"
#include <iostream>
#include <cstdlib>

/**
 * @brief Логирует сообщение об ошибке
 * @param message Сообщение об ошибке для вывода
 * @param funcName Имя функции, в которой произошла ошибка
 */
void ErrorHandler::logErrorImpl(const std::string& message, const char* funcName) {
    std::cerr << "[ОШИБКА в " << funcName << "] " << message << std::endl;
}

/**
 * @brief Логирует ошибку и завершает программу
 * @param message Сообщение об ошибке
 * @param funcName Имя функции, в которой произошла ошибка
 */
void ErrorHandler::exitWithErrorImpl(const std::string& message, const char* funcName) {
    logErrorImpl(message, funcName);
    std::exit(EXIT_FAILURE);
}

/**
 * @brief Выводит справку по использованию программы
 */
void ErrorHandler::printHelp() {
    std::cout << "Использование: ./client <адрес_сервера> <входной_файл> <выходной_файл> [опции]\n";
    std::cout << "Опции:\n";
    std::cout << "  -p <порт>          Порт сервера (по умолчанию: 33333)\n";
    std::cout << "  -c <файл_конфига>  Файл с логином и паролем (по умолчанию: ~/.config/vclient.conf)\n";
    std::cout << "  -h                 Показать эту справку\n";
}