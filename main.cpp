/**
 * @file main.cpp
 * @brief Главный файл клиентского приложения
 * @author Ладыгин Пётр Александрович
 * @date 20.12.2025
 * 
 * @details Точка входа в программу. Создает экземпляр клиента
 *          и запускает его выполнение.
 */

#include "Client.h"
#include "ErrorHandler.h"

/**
 * @brief Точка входа в программу
 * @param argc Количество аргументов командной строки
 * @param argv Массив аргументов командной строки
 * @return Код завершения программы:
 *         - EXIT_SUCCESS (0) при успешном выполнении
 *         - EXIT_FAILURE (1) при возникновении ошибки
 * 
 * @details Функция выполняет следующие действия:
 *          1. Если аргументы не переданы, выводит справку
 *          2. Создает экземпляр класса Client
 *          3. Запускает основной цикл работы клиента
 *          4. Возвращает код завершения в зависимости от результата
 * 
 * @example Примеры использования:
 * @code
 * // Базовое использование
 * ./client 192.168.1.100 input.bin output.txt
 * 
 * // С указанием порта
 * ./client 192.168.1.100 input.bin output.txt -p 44444
 * 
 * // С указанием конфигурационного файла
 * ./client 192.168.1.100 input.bin output.txt -c /path/to/config.conf
 * 
 * // Показать справку
 * ./client -h
 * @endcode
 */
int main(int argc, char* argv[]) {
    // Если аргументы не переданы, показываем справку
    if (argc == 1) {
        ErrorHandler::printHelp();
        return EXIT_SUCCESS;
    }
    
    // Создаем и запускаем клиент
    Client client;
    if (!client.run(argc, argv)) {
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}