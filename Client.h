/**
 * @file Client.h
 * @brief Заголовочный файл класса Client и структуры ClientConfig
 * @author Ладыгин Пётр Александрович
 * @date 20.12.2025
 */

#ifndef CLIENT_H
#define CLIENT_H

#include <string>

/**
 * @struct ClientConfig
 * @brief Структура для хранения конфигурации клиента
 * 
 * Содержит все параметры, необходимые для работы клиента:
 * адрес сервера, порт, имена файлов, учетные данные.
 */
struct ClientConfig {
    std::string serverAddress;   ///< Адрес сервера (IP или доменное имя)
    int serverPort;              ///< Порт сервера
    std::string inputFileName;   ///< Имя входного файла с векторами
    std::string outputFileName;  ///< Имя выходного файла для результатов
    std::string configFileName;  ///< Имя файла конфигурации
    std::string login;           ///< Логин для аутентификации
    std::string password;        ///< Пароль для аутентификации
    
    /**
     * @brief Конструктор по умолчанию
     * @details Инициализирует значения по умолчанию:
     *          порт 33333, файл конфигурации ~/.config/velient.conf
     */
    ClientConfig();
};

/**
 * @class Client
 * @brief Основной класс клиентского приложения
 * 
 * Координирует всю работу клиента: парсинг аргументов,
 * чтение конфигурации, обработку данных, взаимодействие с сервером.
 */
class Client {
private:
    ClientConfig config; ///< Конфигурация клиента
    
    /**
     * @brief Парсит аргументы командной строки
     * @param argc Количество аргументов
     * @param argv Массив аргументов
     * @return true если парсинг успешен, false в случае ошибки
     */
    bool parseCommandLineArgs(int argc, char* argv[]);
    
    /**
     * @brief Читает конфигурационный файл
     * @return true если чтение успешно, false в случае ошибки
     * @details Читает логин и пароль из указанного файла
     */
    bool readConfigFile();
    
public:
    /**
     * @brief Основной метод запуска клиента
     * @param argc Количество аргументов командной строки
     * @param argv Массив аргументов командной строки
     * @return true если выполнение успешно, false в случае ошибки
     * @details Выполняет все шаги работы клиента:
     *          1. Парсинг аргументов
     *          2. Чтение конфигурации
     *          3. Обработка данных
     *          4. Установка соединения
     *          5. Аутентификация
     *          6. Отправка данных
     *          7. Сохранение результатов
     */
    bool run(int argc, char* argv[]);
};

#endif // CLIENT_H