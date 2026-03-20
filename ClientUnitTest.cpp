// ClientUnitTest.cpp - модульное тестирование для клиента (без предупреждений)
#include <UnitTest++/UnitTest++.h>
#include <fstream>
#include <iostream>
#include <cstring>
#include <vector>

// Включаем заголовки проекта
#include "Client.h"
#include "DataProcessor.h"
#include "Authenticator.h"
#include "ErrorHandler.h"

// Вспомогательные функции для тестов
namespace TestUtils {
    // Создание тестового бинарного файла с векторами
    bool createTestVectorFile(const std::string& filename, 
                             const std::vector<std::vector<uint32_t>>& vectors) {
        std::ofstream file(filename, std::ios::binary);
        if (!file.is_open()) return false;
        
        uint32_t numVectors = static_cast<uint32_t>(vectors.size());
        file.write(reinterpret_cast<const char*>(&numVectors), sizeof(numVectors));
        
        for (const auto& vec : vectors) {
            uint32_t vecSize = static_cast<uint32_t>(vec.size());
            file.write(reinterpret_cast<const char*>(&vecSize), sizeof(vecSize));
            
            if (vecSize > 0) {
                file.write(reinterpret_cast<const char*>(vec.data()), 
                          vecSize * sizeof(uint32_t));
            }
        }
        
        file.close();
        return true;
    }
    
    // Создание текстового конфиг файла
    bool createConfigFile(const std::string& filename, 
                         const std::string& login, 
                         const std::string& password) {
        std::ofstream file(filename);
        if (!file.is_open()) return false;
        
        file << login << "\n";
        file << password << "\n";
        file.close();
        
        return true;
    }
    
    // Очистка тестовых файлов
    void cleanupTestFiles() {
        std::remove("test.bin");
        std::remove("test.txt");
        std::remove("config.txt");
        std::remove("output.txt");
        std::remove("empty.bin");
        std::remove("single.bin");
        std::remove("large.bin");
        std::remove("corrupted.bin");
        std::remove("many.bin");
        std::remove("many_results.txt");
        std::remove("integration.bin");
        std::remove("results.txt");
    }
}

// ==================== ТЕСТЫ ДЛЯ DataProcessor ====================

TEST(DataProcessor_ReadValidFile) {
    TestUtils::cleanupTestFiles();
    
    // Подготовка тестовых данных
    std::vector<std::vector<uint32_t>> testVectors = {
        {1, 2, 3},
        {4, 5, 6, 7},
        {8, 9}
    };
    
    // Создаем тестовый файл
    CHECK(TestUtils::createTestVectorFile("test.bin", testVectors));
    
    // Тестируем DataProcessor
    DataProcessor processor;
    
    // Чтение файла
    CHECK(processor.readVectorsFromFile("test.bin"));
    
    // Проверка количества векторов
    CHECK_EQUAL(3, processor.getVectorsCount());
    
    // Проверка содержимого
    const auto& vectors = processor.getVectors();
    CHECK_EQUAL(3, vectors[0].size());
    CHECK_EQUAL(1, vectors[0][0]);
    CHECK_EQUAL(3, vectors[0][2]);
    
    CHECK_EQUAL(4, vectors[1].size());
    CHECK_EQUAL(7, vectors[1][3]);
    
    TestUtils::cleanupTestFiles();
}

TEST(DataProcessor_ValidateCorrectData) {
    // Подготовка
    std::vector<std::vector<uint32_t>> testVectors = {{1, 2, 3}};
    CHECK(TestUtils::createTestVectorFile("test.bin", testVectors));
    
    DataProcessor processor;
    CHECK(processor.readVectorsFromFile("test.bin"));
    
    // Валидация должна пройти успешно
    CHECK(processor.validateData());
    
    TestUtils::cleanupTestFiles();
}

TEST(DataProcessor_SaveResults) {
    DataProcessor processor;
    
    // Тестовые результаты
    std::vector<uint32_t> results = {100, 200, 300};
    
    // Сохранение результатов
    CHECK(processor.saveResults("output.txt", results));
    
    // Проверка файла
    std::ifstream file("output.txt");
    CHECK(file.is_open());
    
    size_t count;
    file >> count;
    CHECK_EQUAL(3, count);
    
    uint32_t val1, val2, val3;
    file >> val1 >> val2 >> val3;
    CHECK_EQUAL(100, val1);
    CHECK_EQUAL(200, val2);
    CHECK_EQUAL(300, val3);
    
    file.close();
    
    TestUtils::cleanupTestFiles();
}

// ==================== ТЕСТЫ ДЛЯ Authenticator ====================

TEST(Authenticator_ComputeHash) {
    std::string salt = "0123456789ABCDEF";
    std::string password = "test123";
    
    std::string hash = Authenticator::computeHash(salt, password);
    
    // Проверка формата хеша
    CHECK_EQUAL(32, hash.length());  // MD5 хеш = 32 hex символа
    
    // Проверка что все символы в верхнем регистре
    for (size_t i = 0; i < hash.length(); ++i) {
        char c = hash[i];
        CHECK(std::isxdigit(c));
        CHECK(std::toupper(c) == c);
    }
}

TEST(Authenticator_SaltConversion) {
    uint64_t salt = 0xDEADBEEFCAFEBABE;
    
    // Конвертация в hex
    std::string hex = Authenticator::saltToHex(salt);
    CHECK_EQUAL(16, hex.length());
    CHECK_EQUAL("DEADBEEFCAFEBABE", hex);
    
    // Обратная конвертация
    uint64_t convertedBack = Authenticator::hexToSalt(hex);
    CHECK_EQUAL(salt, convertedBack);
}

TEST(Authenticator_HexToSaltCaseInsensitive) {
    // Проверка работы с нижним регистром
    std::string lowerHex = "deadbeefcafebabe";
    uint64_t salt = Authenticator::hexToSalt(lowerHex);
    CHECK_EQUAL(0xDEADBEEFCAFEBABE, salt);
}

// ==================== ТЕСТЫ ДЛЯ ClientConfig ====================

TEST(ClientConfig_DefaultConstructor) {
    ClientConfig config;
    
    // Проверка значений по умолчанию
    CHECK_EQUAL(33333, config.serverPort);
    CHECK_EQUAL("~/.config/vclient.conf", config.configFileName);
    
    // Остальные поля должны быть пустыми
    CHECK(config.serverAddress.empty());
    CHECK(config.inputFileName.empty());
    CHECK(config.outputFileName.empty());
    CHECK(config.login.empty());
    CHECK(config.password.empty());
}

// ==================== ИНТЕГРАЦИОННЫЕ ТЕСТЫ ====================

TEST(Integration_FileReadWriteCycle) {
    TestUtils::cleanupTestFiles();
    
    // Подготовка тестовых данных
    std::vector<std::vector<uint32_t>> originalVectors = {
        {100, 200, 300},
        {400, 500},
        {600, 700, 800, 900}
    };
    
    // Запись в файл
    CHECK(TestUtils::createTestVectorFile("integration.bin", originalVectors));
    
    // Чтение через DataProcessor
    DataProcessor processor;
    CHECK(processor.readVectorsFromFile("integration.bin"));
    
    // Проверка прочитанных данных
    CHECK_EQUAL(3, processor.getVectorsCount());
    
    const auto& vectors = processor.getVectors();
    
    // Проверка первого вектора
    CHECK_EQUAL(3, vectors[0].size());
    CHECK_EQUAL(100, vectors[0][0]);
    CHECK_EQUAL(200, vectors[0][1]);
    CHECK_EQUAL(300, vectors[0][2]);
    
    // Проверка второго вектора
    CHECK_EQUAL(2, vectors[1].size());
    CHECK_EQUAL(400, vectors[1][0]);
    CHECK_EQUAL(500, vectors[1][1]);
    
    // Сохранение результатов
    std::vector<uint32_t> results = {42, 84, 126};
    CHECK(processor.saveResults("results.txt", results));
    
    // Проверка сохраненных результатов
    std::ifstream resultsFile("results.txt");
    CHECK(resultsFile.is_open());
    
    size_t count;
    resultsFile >> count;
    CHECK_EQUAL(3, count);
    
    uint32_t r1, r2, r3;
    resultsFile >> r1 >> r2 >> r3;
    CHECK_EQUAL(42, r1);
    CHECK_EQUAL(84, r2);
    CHECK_EQUAL(126, r3);
    
    resultsFile.close();
    
    TestUtils::cleanupTestFiles();
}

TEST(Integration_ConfigFileParsing) {
    TestUtils::cleanupTestFiles();
    
    // Создание тестового конфига
    CHECK(TestUtils::createConfigFile("config.txt", "testuser", "testpass"));
    
    // Чтение конфига вручную (имитация логики из Client::readConfigFile)
    std::ifstream configFile("config.txt");
    CHECK(configFile.is_open());
    
    std::string login, password;
    
    // Чтение логина
    std::string line;
    while (std::getline(configFile, line)) {
        // Удаление пробелов
        size_t start = line.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) continue;
        size_t end = line.find_last_not_of(" \t\r\n");
        line = line.substr(start, end - start + 1);
        
        if (line.empty() || line[0] == '#') continue;
        
        login = line;
        break;
    }
    
    CHECK_EQUAL("testuser", login);
    
    // Чтение пароля
    while (std::getline(configFile, line)) {
        size_t start = line.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) continue;
        size_t end = line.find_last_not_of(" \t\r\n");
        line = line.substr(start, end - start + 1);
        
        if (line.empty() || line[0] == '#') continue;
        
        password = line;
        break;
    }
    
    CHECK_EQUAL("testpass", password);
    
    configFile.close();
    
    TestUtils::cleanupTestFiles();
}

// ==================== ТЕСТЫ ГРАНИЧНЫХ СЛУЧАЕВ ====================

TEST(Boundary_EmptyVectorFile) {
    TestUtils::cleanupTestFiles();
    
    // Файл с нулевым количеством векторов
    std::vector<std::vector<uint32_t>> emptyVectors;
    CHECK(TestUtils::createTestVectorFile("empty.bin", emptyVectors));
    
    DataProcessor processor;
    CHECK(processor.readVectorsFromFile("empty.bin"));
    CHECK_EQUAL(0, processor.getVectorsCount());
    
    // Валидация должна вернуть false
    CHECK(!processor.validateData());
    
    TestUtils::cleanupTestFiles();
}

TEST(Boundary_SingleElement) {
    TestUtils::cleanupTestFiles();
    
    // Вектор с одним элементом
    std::vector<std::vector<uint32_t>> singleVector = {{42}};
    CHECK(TestUtils::createTestVectorFile("single.bin", singleVector));
    
    DataProcessor processor;
    CHECK(processor.readVectorsFromFile("single.bin"));
    
    CHECK_EQUAL(1, processor.getVectorsCount());
    CHECK_EQUAL(1, processor.getVectors()[0].size());
    CHECK_EQUAL(42, processor.getVectors()[0][0]);
    
    TestUtils::cleanupTestFiles();
}

TEST(Boundary_LargeNumbers) {
    TestUtils::cleanupTestFiles();
    
    // Большие числа
    std::vector<std::vector<uint32_t>> largeNumbers = {
        {0xFFFFFFFF, 0, 0x12345678}
    };
    
    CHECK(TestUtils::createTestVectorFile("large.bin", largeNumbers));
    
    DataProcessor processor;
    CHECK(processor.readVectorsFromFile("large.bin"));
    
    const auto& vectors = processor.getVectors();
    CHECK_EQUAL(0xFFFFFFFF, vectors[0][0]);
    CHECK_EQUAL(0, vectors[0][1]);
    CHECK_EQUAL(0x12345678, vectors[0][2]);
    
    TestUtils::cleanupTestFiles();
}

// ==================== ТЕСТЫ ПРОИЗВОДИТЕЛЬНОСТИ ====================

TEST(Performance_MultipleVectors) {
    TestUtils::cleanupTestFiles();
    
    // Создаем 100 векторов
    std::vector<std::vector<uint32_t>> manyVectors;
    for (int i = 0; i < 100; i++) {
        std::vector<uint32_t> vec = {
            static_cast<uint32_t>(i), 
            static_cast<uint32_t>(i*10), 
            static_cast<uint32_t>(i*100)
        };
        manyVectors.push_back(vec);
    }
    
    CHECK(TestUtils::createTestVectorFile("many.bin", manyVectors));
    
    DataProcessor processor;
    
    // Измеряем время чтения (косвенно)
    CHECK(processor.readVectorsFromFile("many.bin"));
    CHECK_EQUAL(100, processor.getVectorsCount());
    
    // Сохранение результатов
    std::vector<uint32_t> results;
    for (int i = 0; i < 100; i++) {
        results.push_back(static_cast<uint32_t>(i * 1000));
    }
    
    CHECK(processor.saveResults("many_results.txt", results));
    
    TestUtils::cleanupTestFiles();
}

// ==================== ОСНОВНАЯ ФУНКЦИЯ ====================

int main() {
    // Убраны неиспользуемые параметры argc и argv
    std::cout << "Запуск модульных тестов для Client\n";
    std::cout << "===================================\n";
    
    // Очистка перед запуском
    TestUtils::cleanupTestFiles();
    
    // Запуск тестов
    int result = UnitTest::RunAllTests();
    
    // Очистка после тестов
    TestUtils::cleanupTestFiles();
    
    std::cout << "\n===================================\n";
    if (result == 0) {
        std::cout << "Все тесты пройдены успешно!\n";
    } else {
        std::cout << "Обнаружены ошибки в тестах\n";
    }
    
    return result;
}