CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -I.
LDFLAGS = -lssl -lcrypto -lpthread
TEST_LDFLAGS = -lUnitTest++ $(LDFLAGS)

CLIENT_SRCS = main.cpp Client.cpp DataProcessor.cpp ErrorHandler.cpp Authenticator.cpp ServerConnection.cpp
TEST_SRCS = UnitTest.cpp
CLIENT_EXEC = client
TEST_EXEC = client_tests

all: $(CLIENT_EXEC)

# Сборка клиента
$(CLIENT_EXEC): $(CLIENT_SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Сборка тестов
$(TEST_EXEC): $(TEST_SRCS) Client.cpp DataProcessor.cpp ErrorHandler.cpp Authenticator.cpp ServerConnection.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(TEST_LDFLAGS)

# Очистка
clean:
	rm -f $(CLIENT_EXEC) $(TEST_EXEC) *.o

# Тестирование
test: $(TEST_EXEC)

.PHONY: all clean test
