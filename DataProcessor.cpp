/**
 * @file DataProcessor.cpp
 * @brief Реализация класса DataProcessor
 * @author Ладыгин Пётр Александрович
 * @date 20.12.2025
 */

#include "DataProcessor.h"
#include "ErrorHandler.h"
#include <fstream>
#include <iostream>
#include <cstring>

/**
 * @brief Читает векторы из бинарного файла
 * @param filename Имя входного файла
 * @return true если чтение успешно, false в случае ошибки
 */
bool DataProcessor::readVectorsFromFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        ErrorHandler::exitWithError("Не удалось открыть файл: " + filename);
        return false;
    }

    // Чтение количества векторов (uint32_t, 4 байта)
    uint32_t numVectors;
    file.read(reinterpret_cast<char*>(&numVectors), sizeof(uint32_t));
    
    if (!file || file.gcount() != sizeof(uint32_t)) {
        ErrorHandler::exitWithError("Ошибка чтения количества векторов из файла: " + filename);
        return false;
    }
    
    vectors.clear();
    vectors.reserve(numVectors);

    for (uint32_t i = 0; i < numVectors; ++i) {
        // Чтение размера вектора
        uint32_t vectorSize;
        file.read(reinterpret_cast<char*>(&vectorSize), sizeof(uint32_t));
        
        if (!file || file.gcount() != sizeof(uint32_t)) {
            ErrorHandler::exitWithError("Ошибка чтения размера вектора " + std::to_string(i+1));
            return false;
        }

        std::vector<uint32_t> vec;
        vec.resize(vectorSize);
        
        // Чтение значений вектора
        if (vectorSize > 0) {
            file.read(reinterpret_cast<char*>(vec.data()), vectorSize * sizeof(uint32_t));
            
            if (!file || file.gcount() != static_cast<std::streamsize>(vectorSize * sizeof(uint32_t))) {
                ErrorHandler::exitWithError("Ошибка чтения значений вектора " + std::to_string(i+1));
                return false;
            }
        }
        
        vectors.push_back(vec);
        
        std::cout << "Отладка: Вектор " << i << ", размер " << vectorSize << std::endl;
    }

    file.close();
    return true;
}

/**
 * @brief Валидирует загруженные данные
 * @return true если данные валидны, false в случае ошибки
 */
bool DataProcessor::validateData() const {
    if (vectors.empty()) {
        ErrorHandler::logError("Нет векторов для обработки");
        return false;
    }
    
    for (const auto& vec : vectors) {
        if (vec.empty()) {
            ErrorHandler::logError("Обнаружен пустой вектор");
            return false;
        }
    }
    
    return true;
}

/**
 * @brief Сохраняет результаты обработки в файл
 * @param filename Имя выходного файла
 * @param results Вектор результатов для сохранения
 * @return true если сохранение успешно, false в случае ошибки
 */
bool DataProcessor::saveResults(const std::string& filename, const std::vector<uint32_t>& results) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        ErrorHandler::logError("Не удалось открыть файл для записи результатов: " + filename);
        return false;
    }
    
    // Количество результатов
    file << results.size();
    
    // Сами результаты
    for (size_t i = 0; i < results.size(); ++i) {
        file << " " << results[i];
    }
    
    file << std::endl;
    file.close();
    
    if (!file) {
        ErrorHandler::logError("Ошибка записи в файл: " + filename);
        return false;
    }
    
    return true;
}