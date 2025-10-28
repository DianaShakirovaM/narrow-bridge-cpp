#include "narrow_bridge_test.hpp"
#include <chrono>
#include <atomic>

// Тест 1: Базовая функциональность - одиночные машины
class SingleCarTest : public TestBase {
public:
    void run_all_tests() override {
        std::cout << "\n=== ТЕСТ 1: ОДИНОЧНЫЕ МАШИНЫ ===" << std::endl;
        
        // Тест 1.1: Одна машина с севера
        test_single_north_car();
        
        // Тест 1.2: Одна машина с юга
        test_single_south_car();
    }

private:
    void test_single_north_car() {
        NarrowBridge bridge;
        std::atomic<bool> test_completed{false};
        
        std::thread car([&]() {
            bridge.arriveFromNorth(1);
            test_completed = true;
        });
        
        // Ждем завершения с таймаутом
        auto start = std::chrono::steady_clock::now();
        while (!test_completed && 
               std::chrono::steady_clock::now() - start < std::chrono::seconds(5)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        test_assert(test_completed, "Одиночная машина с севера успешно переехала");
        test_assert(bridge.getSuccessfulCrossings() == 1, "Счетчик успешных переездов = 1");
        test_assert(bridge.getTotalCars() == 1, "Общий счетчик машин = 1");
        
        if (car.joinable()) car.join();
    }
    
    void test_single_south_car() {
        NarrowBridge bridge;
        std::atomic<bool> test_completed{false};
        
        std::thread car([&]() {
            bridge.arriveFromSouth(1);
            test_completed = true;
        });
        
        auto start = std::chrono::steady_clock::now();
        while (!test_completed && 
               std::chrono::steady_clock::now() - start < std::chrono::seconds(5)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        test_assert(test_completed, "Одиночная машина с юга успешно переехала");
        test_assert(bridge.getSuccessfulCrossings() == 1, "Счетчик успешных переездов = 1");
        test_assert(bridge.getTotalCars() == 1, "Общий счетчик машин = 1");
        
        if (car.joinable()) car.join();
    }
};

// Тест 2: Множественные машины в одном направлении
class SameDirectionTest : public TestBase {
public:
    void run_all_tests() override {
        std::cout << "\n=== ТЕСТ 2: МНОЖЕСТВЕННЫЕ МАШИНЫ В ОДНОМ НАПРАВЛЕНИИ ===" << std::endl;
        
        test_multiple_north_cars();
        test_multiple_south_cars();
    }

private:
    void test_multiple_north_cars() {
        NarrowBridge bridge;
        const int num_cars = 3;
        std::atomic<int> completed_count{0};
        std::vector<std::thread> cars;
        
        // Создаем несколько машин с севера
        for (int i = 1; i <= num_cars; ++i) {
            cars.emplace_back([&, i]() {
                bridge.arriveFromNorth(i);
                completed_count++;
            });
        }
        
        // Ждем завершения всех машин
        auto start = std::chrono::steady_clock::now();
        while (completed_count < num_cars && 
               std::chrono::steady_clock::now() - start < std::chrono::seconds(10)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        test_assert(completed_count == num_cars, "Все 3 машины с севера успешно переехали");
        test_assert(bridge.getSuccessfulCrossings() == num_cars, 
                   "Счетчик успешных переездов = " + std::to_string(num_cars));
        test_assert(bridge.getTotalCars() == num_cars, 
                   "Общий счетчик машин = " + std::to_string(num_cars));
        
        for (auto& car : cars) {
            if (car.joinable()) car.join();
        }
    }
    
    void test_multiple_south_cars() {
        NarrowBridge bridge;
        const int num_cars = 3;
        std::atomic<int> completed_count{0};
        std::vector<std::thread> cars;
        
        for (int i = 1; i <= num_cars; ++i) {
            cars.emplace_back([&, i]() {
                bridge.arriveFromSouth(i);
                completed_count++;
            });
        }
        
        auto start = std::chrono::steady_clock::now();
        while (completed_count < num_cars && 
               std::chrono::steady_clock::now() - start < std::chrono::seconds(10)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        test_assert(completed_count == num_cars, "Все 3 машины с юга успешно переехали");
        test_assert(bridge.getSuccessfulCrossings() == num_cars, 
                   "Счетчик успешных переездов = " + std::to_string(num_cars));
        test_assert(bridge.getTotalCars() == num_cars, 
                   "Общий счетчик машин = " + std::to_string(num_cars));
        
        for (auto& car : cars) {
            if (car.joinable()) car.join();
        }
    }
};

// Тест 3: Чередование направлений
class AlternatingDirectionsTest : public TestBase {
public:
    void run_all_tests() override {
        std::cout << "\n=== ТЕСТ 3: ЧЕРЕДОВАНИЕ НАПРАВЛЕНИЙ ===" << std::endl;
        
        test_alternating_directions();
    }

private:
    void test_alternating_directions() {
        NarrowBridge bridge;
        std::atomic<int> north_completed{0};
        std::atomic<int> south_completed{0};
        
        // Сначала машина с севера
        std::thread north_car([&]() {
            bridge.arriveFromNorth(1);
            north_completed++;
        });
        
        // Даем время северной машине начать движение
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // Затем машина с юга (должна ждать)
        std::thread south_car([&]() {
            bridge.arriveFromSouth(2);
            south_completed++;
        });
        
        // Ждем завершения
        auto start = std::chrono::steady_clock::now();
        while ((north_completed < 1 || south_completed < 1) && 
               std::chrono::steady_clock::now() - start < std::chrono::seconds(10)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        test_assert(north_completed == 1, "Машина с севера завершила переезд");
        test_assert(south_completed == 1, "Машина с юга завершила переезд");
        test_assert(bridge.getSuccessfulCrossings() == 2, "Обе машины успешно переехали");
        test_assert(bridge.getTotalCars() == 2, "Общий счетчик машин = 2");
        
        if (north_car.joinable()) north_car.join();
        if (south_car.joinable()) south_car.join();
    }
};

// Тест 4: Статистика и граничные условия
class StatisticsTest : public TestBase {
public:
    void run_all_tests() override {
        std::cout << "\n=== ТЕСТ 4: СТАТИСТИКА И ГРАНИЧНЫЕ УСЛОВИЯ ===" << std::endl;
        
        test_empty_bridge_statistics();
        test_all_cars_crossed_method();
    }

private:
    void test_empty_bridge_statistics() {
        NarrowBridge bridge;
        
        // Проверяем начальное состояние
        test_assert(bridge.getSuccessfulCrossings() == 0, "Начальный счетчик успешных переездов = 0");
        test_assert(bridge.getTotalCars() == 0, "Начальный общий счетчик машин = 0");
        test_assert(bridge.allCarsCrossedSuccessfully() == true, "allCarsCrossedSuccessfully = true для пустого моста");
    }
    
    void test_all_cars_crossed_method() {
        NarrowBridge bridge;
        
        // Запускаем одну машину
        std::thread car([&]() {
            bridge.arriveFromNorth(1);
        });
        
        // Даем время на выполнение
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        // Проверяем метод allCarsCrossedSuccessfully
        test_assert(bridge.allCarsCrossedSuccessfully() == true, 
                   "allCarsCrossedSuccessfully возвращает true после завершения всех переездов");
        
        if (car.joinable()) car.join();
    }
};

// Тест 5: Стресс-тест с большим количеством машин
class StressTest : public TestBase {
public:
    void run_all_tests() override {
        std::cout << "\n=== ТЕСТ 5: СТРЕСС-ТЕСТ ===" << std::endl;
        
        test_multiple_cars_both_directions();
    }

private:
    void test_multiple_cars_both_directions() {
        NarrowBridge bridge;
        const int num_cars = 10;
        std::atomic<int> completed_count{0};
        std::vector<std::thread> cars;
        
        // Создаем машины в случайном порядке
        for (int i = 1; i <= num_cars; ++i) {
            if (i % 2 == 0) {
                cars.emplace_back([&, i]() {
                    bridge.arriveFromNorth(i);
                    completed_count++;
                });
            } else {
                cars.emplace_back([&, i]() {
                    bridge.arriveFromSouth(i);
                    completed_count++;
                });
            }
            // Небольшая задержка между созданием машин
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        
        // Ждем завершения всех машин
        auto start = std::chrono::steady_clock::now();
        while (completed_count < num_cars && 
               std::chrono::steady_clock::now() - start < std::chrono::seconds(30)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        test_assert(completed_count == num_cars, 
                   "Все " + std::to_string(num_cars) + " машин успешно переехали");
        test_assert(bridge.getSuccessfulCrossings() == num_cars, 
                   "Счетчик успешных переездов = " + std::to_string(num_cars));
        test_assert(bridge.getTotalCars() == num_cars, 
                   "Общий счетчик машин = " + std::to_string(num_cars));
        test_assert(bridge.allCarsCrossedSuccessfully() == true, 
                   "allCarsCrossedSuccessfully возвращает true после стресс-теста");
        
        for (auto& car : cars) {
            if (car.joinable()) car.join();
        }
    }
};

// Главная функция запуска всех тестов
int main() {
    std::cout << "ЗАПУСК ТЕСТИРОВАНИЯ КЛАССА NarrowBridge" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // Создаем и запускаем все тесты
    SingleCarTest test1;
    SameDirectionTest test2;
    AlternatingDirectionsTest test3;
    StatisticsTest test4;
    StressTest test5;
    
    test1.run_all_tests();
    test2.run_all_tests();
    test3.run_all_tests();
    test4.run_all_tests();
    test5.run_all_tests();
    
    // Выводим общую статистику
    std::cout << "\n=== ОБЩАЯ СТАТИСТИКА ТЕСТИРОВАНИЯ ===" << std::endl;
    test1.print_summary();
    
    // Проверяем общий успех
    if (test1.passed_tests == test1.total_tests) {
        std::cout << "\n🎉 ВСЕ ТЕСТЫ ПРОЙДЕНЫ УСПЕШНО!" << std::endl;
        return 0;
    } else {
        std::cout << "\n💥 ОБНАРУЖЕНЫ ПРОВАЛЕННЫЕ ТЕСТЫ!" << std::endl;
        return 1;
    }
}