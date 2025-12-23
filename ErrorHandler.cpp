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
 */
void ErrorHandler::logError(const std::string& message) {
    std::cerr << "[ОШИБКА] " << message << std::endl;
}

/**
 * @brief Логирует ошибку и завершает программу
 * @param message Сообщение об ошибке
 */
void ErrorHandler::exitWithError(const std::string& message) {
    logError(message);
    std::exit(EXIT_FAILURE);
}

/**
 * @brief Выводит справку по использованию программы
 */
void ErrorHandler::printHelp() {
    std::cout << "Использование: ./client <адрес_сервера> <входной_файл> <выходной_файл> [опции]\n";
    std::cout << "Опции:\n";
    std::cout << "  -p <порт>          Порт сервера (по умолчанию: 33333)\n";
    std::cout << "  -c <файл_конфига>  Файл с логином и паролем (по умолчанию: ~/.config/velient.conf)\n";
    std::cout << "  -h                 Показать эту справку\n";
}