/**
 * @file Authenticator.cpp
 * @brief Реализация класса Authenticator
 * @author Ладыгин Пётр Александрович
 * @date 20.12.2025
 */

#include "Authenticator.h"
#include <openssl/md5.h>
#include <sstream>
#include <iomanip>
#include <random>
#include <algorithm>
#include <cstring>

/**
 * @brief Вычисляет MD5 хеш от конкатенации соли и пароля
 * @param salt Соль в строковом формате
 * @param password Пароль пользователя
 * @return MD5 хеш в верхнем регистре (hex)
 */
std::string Authenticator::computeHash(const std::string& salt, const std::string& password) {
    std::string combined = salt + password;
    unsigned char hash[MD5_DIGEST_LENGTH];
    
    MD5(reinterpret_cast<const unsigned char*>(combined.c_str()), combined.size(), hash);

    std::stringstream ss;
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') 
           << static_cast<int>(hash[i]);
    }
    
    // Конвертируем в верхний регистр согласно ТЗ
    std::string result = ss.str();
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    
    return result;
}

/**
 * @brief Преобразует 64-битную соль в hex строку
 * @param salt 64-битное беззнаковое целое
 * @return Hex строка (16 символов) в верхнем регистре
 */
std::string Authenticator::saltToHex(uint64_t salt) {
    std::stringstream ss;
    ss << std::hex << std::setw(16) << std::setfill('0') << salt;
    
    std::string result = ss.str();
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    
    return result;
}

/**
 * @brief Преобразует hex строку в 64-битную соль
 * @param hex Hex строка (ожидается 16 символов)
 * @return 64-битное беззнаковое целое
 */
uint64_t Authenticator::hexToSalt(const std::string& hex) {
    uint64_t salt;
    std::stringstream ss;
    ss << std::hex << hex;
    ss >> salt;
    return salt;
}