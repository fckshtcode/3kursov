/**
 * @file Authenticator.h
 * @brief Заголовочный файл класса Authenticator
 * @author Ладыгин Пётр Александрович
 * @date 20.12.2025
 */

#ifndef AUTHENTICATOR_H
#define AUTHENTICATOR_H

#include <string>
#include <cstdint>

/**
 * @class Authenticator
 * @brief Класс для аутентификации и работы с хешами
 * 
 * Предоставляет методы для вычисления MD5 хешей, 
 * преобразования соли между различными форматами.
 * Все методы являются статическими.
 */
class Authenticator {
public:
    /**
     * @brief Вычисляет MD5 хеш от конкатенации соли и пароля
     * @param salt Соль в строковом формате
     * @param password Пароль пользователя
     * @return MD5 хеш в верхнем регистре (hex)
     * @details Результат преобразуется в верхний регистр согласно ТЗ
     */
    static std::string computeHash(const std::string& salt, const std::string& password);
    
    /**
     * @brief Преобразует 64-битную соль в hex строку
     * @param salt 64-битное беззнаковое целое
     * @return Hex строка (16 символов) в верхнем регистре
     */
    static std::string saltToHex(uint64_t salt);
    
    /**
     * @brief Преобразует hex строку в 64-битную соль
     * @param hex Hex строка (ожидается 16 символов)
     * @return 64-битное беззнаковое целое
     */
    static uint64_t hexToSalt(const std::string& hex);
};

#endif // AUTHENTICATOR_H