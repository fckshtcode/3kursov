/**
 * @file DataProcessor.h
 * @brief Заголовочный файл класса DataProcessor
 * @author Ладыгин Пётр Александрович
 * @date 20.12.2025
 */

#ifndef DATAPROCESSOR_H
#define DATAPROCESSOR_H

#include <string>
#include <vector>
#include <cstdint>

/**
 * @class DataProcessor
 * @brief Класс для обработки векторных данных
 * 
 * Обеспечивает чтение векторов из бинарного файла,
 * валидацию данных и сохранение результатов обработки.
 */
class DataProcessor {
private:
    std::vector<std::vector<uint32_t>> vectors; ///< Вектора для обработки
    
public:
    /**
     * @brief Читает векторы из бинарного файла
     * @param filename Имя входного файла
     * @return true если чтение успешно, false в случае ошибки
     * @details Формат файла:
     *          - uint32_t: количество векторов
     *          - Для каждого вектора:
     *            - uint32_t: размер вектора
     *            - uint32_t[]: значения вектора
     */
    bool readVectorsFromFile(const std::string& filename);
    
    /**
     * @brief Валидирует загруженные данные
     * @return true если данные валидны, false в случае ошибки
     * @details Проверяет наличие векторов и отсутствие пустых векторов
     */
    bool validateData() const;
    
    /**
     * @brief Сохраняет результаты обработки в файл
     * @param filename Имя выходного файла
     * @param results Вектор результатов для сохранения
     * @return true если сохранение успешно, false в случае ошибки
     * @details Формат файла: количество результатов, затем значения через пробел
     */
    bool saveResults(const std::string& filename, const std::vector<uint32_t>& results) const;
    
    /**
     * @brief Возвращает ссылку на вектора
     * @return Константная ссылка на вектор векторов
     */
    const std::vector<std::vector<uint32_t>>& getVectors() const { return vectors; }
    
    /**
     * @brief Возвращает количество векторов
     * @return Количество загруженных векторов
     */
    size_t getVectorsCount() const { return vectors.size(); }
};

#endif // DATAPROCESSOR_H