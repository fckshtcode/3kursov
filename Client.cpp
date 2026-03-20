/**
 * @file Client.cpp
 * @brief Реализация класса Client и структуры ClientConfig
 * @author Ладыгин Пётр Александрович
 * @date 20.12.2025
 * 
 * @details Этот файл содержит реализацию клиентского приложения для взаимодействия
 * с сервером обработки векторных данных. Класс Client управляет всем жизненным циклом
 * клиента: от парсинга аргументов командной строки до сохранения результатов.
 */

#include "Client.h"
#include "ErrorHandler.h"
#include "DataProcessor.h"
#include "ServerConnection.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <pwd.h>

/**
 * @brief Конструктор по умолчанию для ClientConfig
 * 
 * Инициализирует конфигурацию значениями по умолчанию:
 * - Порт сервера: 33333
 * - Файл конфигурации: ~/.config/vclient.conf
 * - Остальные поля остаются пустыми
 */
ClientConfig::ClientConfig() : serverPort(33333), configFileName("~/.config/vclient.conf") {}

/**
 * @brief Парсит аргументы командной строки
 * @param argc Количество аргументов командной строки
 * @param argv Массив аргументов командной строки
 * @return true если парсинг успешен, false в случае ошибки
 * 
 * @details Формат командной строки:
 * @code
 * ./client <адрес_сервера> <входной_файл> <выходной_файл> [опции]
 * @endcode
 * 
 * Обязательные параметры (порядок важен):
 * 1. Адрес сервера (IP или доменное имя)
 * 2. Имя входного файла с векторами
 * 3. Имя выходного файла для результатов
 * 
 * Опциональные параметры (любой порядок):
 * - -p <порт> - порт сервера (по умолчанию: 33333)
 * - -c <файл_конфига> - файл с логином и паролем (по умолчанию: ~/.config/vclient.conf)
 * - -h - показать справку
 * 
 * @note При некорректных аргументах или использовании -h выводится справка
 *       и функция возвращает false
 * 
 * @see ErrorHandler::printHelp()
 */
bool Client::parseCommandLineArgs(int argc, char* argv[]) {
    // Проверяем минимальное количество аргументов (3 обязательных)
    if (argc < 4) {
        ErrorHandler::printHelp();
        return false;
    }
    
    // Обязательные параметры (позиционные)
    config.serverAddress = argv[1];
    config.inputFileName = argv[2];
    config.outputFileName = argv[3];
    
    // Обработка опциональных параметров
    for (int i = 4; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            // Параметр порта: -p <номер_порта>
            try {
                config.serverPort = std::stoi(argv[++i]);
            } catch (const std::exception& e) {
                ErrorHandler::logError("Некорректный номер порта: " + std::string(argv[i]));
                ErrorHandler::printHelp();
                return false;
            }
        } else if (strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
            // Параметр конфигурационного файла: -c <путь_к_файлу>
            config.configFileName = argv[++i];
        } else if (strcmp(argv[i], "-h") == 0) {
            // Параметр справки: -h
            ErrorHandler::printHelp();
            return false;
        } else {
            // Неизвестный параметр
            ErrorHandler::logError("Неизвестный параметр: " + std::string(argv[i]));
            ErrorHandler::printHelp();
            return false;
        }
    }
    
    // Валидация обязательных параметров
    if (config.serverAddress.empty()) {
        ErrorHandler::logError("Адрес сервера не может быть пустым");
        ErrorHandler::printHelp();
        return false;
    }
    
    if (config.inputFileName.empty()) {
        ErrorHandler::logError("Имя входного файла не может быть пустым");
        ErrorHandler::printHelp();
        return false;
    }
    
    if (config.outputFileName.empty()) {
        ErrorHandler::logError("Имя выходного файла не может быть пустым");
        ErrorHandler::printHelp();
        return false;
    }
    
    return true;
}

/**
 * @brief Читает конфигурационный файл с логином и паролем
 * @return true если чтение успешно, false в случае ошибки
 * 
 * @details Формат конфигурационного файла:
 * - Первая непустая строка (не комментарий): логин
 * - Вторая непустая строка (не комментарий): пароль
 * - Строки, начинающиеся с '#', считаются комментариями и игнорируются
 * - Пустые строки игнорируются
 * - Ведущие и завершающие пробелы удаляются
 * 
 * Пример конфигурационного файла:
 * @code
 * # Файл конфигурации клиента
 * myusername    # Логин пользователя
 * mypassword    # Пароль пользователя
 * @endcode
 * 
 * @note Поддерживает символ '~' в пути к файлу конфигурации,
 *       который будет развернут в путь к домашней директории пользователя.
 *       Сначала проверяется переменная окружения HOME, затем системный вызов.
 * 
 * @warning Файл должен иметь разрешения, ограничивающие доступ
 *          для других пользователей (рекомендуется 600)
 * 
 * @see getenv()
 * @see getpwuid()
 * @see getuid()
 */
bool Client::readConfigFile() {
    // Разворачиваем символ '~' в домашней директории пользователя
    if (config.configFileName.find("~/") == 0) {
        // Пытаемся получить домашнюю директорию из переменной окружения
        const char* homeDir = getenv("HOME");
        if (!homeDir) {
            // Если переменной нет, используем системный вызов
            struct passwd* pw = getpwuid(getuid());
            if (pw) {
                homeDir = pw->pw_dir;
            } else {
                ErrorHandler::logError("Не удалось определить домашнюю директорию");
                return false;
            }
        }
        // Заменяем '~' на путь к домашней директории
        config.configFileName = std::string(homeDir) + config.configFileName.substr(1);
    }
    
    // Открываем файл конфигурации для чтения
    std::ifstream configFile(config.configFileName);
    if (!configFile.is_open()) {
        ErrorHandler::logError("Не удалось открыть файл конфигурации: " + config.configFileName);
        return false;
    }
    
    std::string login, password;
    std::string line;
    bool loginFound = false;
    bool passwordFound = false;
    
    // Чтение файла построчно
    while (std::getline(configFile, line)) {
        // Удаляем ведущие и завершающие пробельные символы
        size_t start = line.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) {
            // Пустая строка, пропускаем
            continue;
        }
        size_t end = line.find_last_not_of(" \t\r\n");
        line = line.substr(start, end - start + 1);
        
        // Проверяем на комментарий
        if (line.empty() || line[0] == '#') {
            // Комментарий или пустая строка после удаления пробелов
            continue;
        }
        
        // Первая непустая строка - логин
        if (!loginFound) {
            login = line;
            loginFound = true;
        } 
        // Вторая непустая строка - пароль
        else if (!passwordFound) {
            password = line;
            passwordFound = true;
            break; // Больше читать не нужно
        }
    }
    
    // Закрываем файл
    configFile.close();
    
    // Проверяем, что оба параметра найдены
    if (!loginFound) {
        ErrorHandler::logError("Логин не найден в файле конфигурации: " + config.configFileName);
        return false;
    }
    
    if (!passwordFound) {
        ErrorHandler::logError("Пароль не найден в файле конфигурации: " + config.configFileName);
        return false;
    }
    
    // Сохраняем полученные данные
    config.login = login;
    config.password = password;
    
    std::cout << "Лог: Прочитан логин: " << config.login << std::endl;
    std::cout << "Лог: Пароль прочитан успешно (длина: " << config.password.length() << " символов)" << std::endl;
    
    return true;
}

/**
 * @brief Основной метод запуска клиента
 * @param argc Количество аргументов командной строки
 * @param argv Массив аргументов командной строки
 * @return true если выполнение успешно, false в случае ошибки
 * 
 * @details Последовательность выполнения:
 * 1. **Парсинг аргументов командной строки** - извлечение параметров запуска
 * 2. **Чтение конфигурационного файла** - получение учетных данных
 * 3. **Загрузка данных** - чтение векторов из бинарного файла
 * 4. **Валидация данных** - проверка корректности загруженных данных
 * 5. **Установка соединения** - TCP подключение к серверу
 * 6. **Аутентификация** - прохождение процедуры аутентификации
 * 7. **Отправка данных** - передача векторов на сервер в формате little-endian
 * 8. **Получение результатов** - получение обработанных данных от сервера
 * 9. **Сохранение результатов** - запись результатов в выходной файл
 * 10. **Завершение работы** - закрытие соединения и вывод итогов
 * 
 * @note В случае ошибки на любом этапе программа завершается
 *       с соответствующим сообщением об ошибке и кодом EXIT_FAILURE
 * 
 * @note Для отладки выводится подробная информация о каждом этапе работы
 * 
 * @see DataProcessor::readVectorsFromFile()
 * @see DataProcessor::validateData()
 * @see ServerConnection::establishConnection()
 * @see ServerConnection::authenticate()
 * @see ServerConnection::sendVectors()
 * @see DataProcessor::saveResults()
 */
bool Client::run(int argc, char* argv[]) {
    std::cout << "========================================" << std::endl;
    std::cout << "Запуск клиента для обработки векторов" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // 1. Парсинг аргументов командной строки
    std::cout << "\n[1/10] Парсинг аргументов командной строки..." << std::endl;
    if (!parseCommandLineArgs(argc, argv)) {
        ErrorHandler::exitWithError("Ошибка парсинга аргументов командной строки");
        return false;
    }
    
    std::cout << "  ✓ Сервер: " << config.serverAddress << ":" << config.serverPort << std::endl;
    std::cout << "  ✓ Входной файл: " << config.inputFileName << std::endl;
    std::cout << "  ✓ Выходной файл: " << config.outputFileName << std::endl;
    std::cout << "  ✓ Файл конфигурации: " << config.configFileName << std::endl;
    
    // 2. Чтение конфигурационного файла
    std::cout << "\n[2/10] Чтение конфигурационного файла..." << std::endl;
    if (!readConfigFile()) {
        ErrorHandler::exitWithError("Ошибка чтения конфигурационного файла");
        return false;
    }
    
    std::cout << "  ✓ Логин: " << config.login << std::endl;
    std::cout << "  ✓ Пароль: [скрыто]" << std::endl;
    
    // 3. Загрузка данных
    std::cout << "\n[3/10] Загрузка векторных данных из файла..." << std::endl;
    DataProcessor dataProcessor;
    if (!dataProcessor.readVectorsFromFile(config.inputFileName)) {
        ErrorHandler::exitWithError("Ошибка чтения векторов из файла: " + config.inputFileName);
        return false;
    }
    
    // 4. Валидация данных
    std::cout << "\n[4/10] Валидация загруженных данных..." << std::endl;
    if (!dataProcessor.validateData()) {
        ErrorHandler::exitWithError("Ошибка валидации данных");
        return false;
    }
    
    size_t vectorsCount = dataProcessor.getVectorsCount();
    std::cout << "  ✓ Загружено векторов: " << vectorsCount << std::endl;
    
    // Выводим информацию о первом векторе для отладки
    if (vectorsCount > 0) {
        const auto& firstVector = dataProcessor.getVectors()[0];
        std::cout << "  ✓ Размер первого вектора: " << firstVector.size() << std::endl;
        if (firstVector.size() > 0) {
            std::cout << "  ✓ Первое значение: " << firstVector[0] << std::endl;
        }
    }
    
    // 5. Установка соединения с сервером
    std::cout << "\n[5/10] Установка соединения с сервером..." << std::endl;
    ServerConnection connection;
    if (!connection.establishConnection(config.serverAddress, config.serverPort)) {
        ErrorHandler::exitWithError("Ошибка установки соединения с сервером");
        return false;
    }
    
    std::cout << "  ✓ Соединение установлено" << std::endl;
    
    // 6. Аутентификация
    std::cout << "\n[6/10] Аутентификация на сервере..." << std::endl;
    if (!connection.authenticate(config.login, config.password)) {
        ErrorHandler::exitWithError("Ошибка аутентификации");
        connection.closeConnection();
        return false;
    }
    
    std::cout << "  ✓ Аутентификация успешна" << std::endl;
    
    // 7. Отправка данных
    std::cout << "\n[7/10] Отправка векторных данных на сервер..." << std::endl;
    const std::vector<std::vector<uint32_t>>& vectors = dataProcessor.getVectors();
    std::vector<uint32_t> results;
    
    if (!connection.sendVectors(vectors, results)) {
        ErrorHandler::exitWithError("Ошибка отправки векторов на сервер");
        connection.closeConnection();
        return false;
    }
    
    std::cout << "  ✓ Данные отправлены успешно" << std::endl;
    
    // 8. Получение результатов
    std::cout << "\n[8/10] Получение результатов от сервера..." << std::endl;
    std::cout << "  ✓ Получено результатов: " << results.size() << std::endl;
    
    // Проверяем соответствие количества результатов количеству векторов
    if (results.size() != vectorsCount) {
        std::cout << "  ⚠  Предупреждение: получено " << results.size() 
                  << " результатов, ожидалось " << vectorsCount << std::endl;
    }
    
    // Выводим первые несколько результатов для отладки
    if (!results.empty()) {
        std::cout << "  ✓ Первые 3 результата: ";
        for (size_t i = 0; i < std::min(results.size(), (size_t)3); ++i) {
            std::cout << results[i];
            if (i < std::min(results.size(), (size_t)3) - 1) {
                std::cout << ", ";
            }
        }
        std::cout << std::endl;
    }
    
    // 9. Сохранение результатов
    std::cout << "\n[9/10] Сохранение результатов в файл..." << std::endl;
    if (!dataProcessor.saveResults(config.outputFileName, results)) {
        ErrorHandler::exitWithError("Ошибка сохранения результатов");
        connection.closeConnection();
        return false;
    }
    
    std::cout << "  ✓ Результаты сохранены в: " << config.outputFileName << std::endl;
    
    // 10. Завершение работы
    std::cout << "\n[10/10] Завершение работы..." << std::endl;
    connection.closeConnection();
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "Программа успешно завершена!" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Итоги:" << std::endl;
    std::cout << "  • Обработано векторов: " << vectorsCount << std::endl;
    std::cout << "  • Получено результатов: " << results.size() << std::endl;
    std::cout << "  • Результаты сохранены в: " << config.outputFileName << std::endl;
    std::cout << "  • Сервер: " << config.serverAddress << ":" << config.serverPort << std::endl;
    std::cout << "========================================" << std::endl;
    
    return true;
}