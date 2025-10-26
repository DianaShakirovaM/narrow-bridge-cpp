#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <vector>
#include <random>
#include <atomic>

class NarrowBridge {
private:
    std::mutex mtx;
    std::condition_variable cv;
    
    int north_cars_on_bridge = 0;
    int south_cars_on_bridge = 0;
    int north_cars_waiting = 0;
    int south_cars_waiting = 0;
    std::string current_direction = "НЕТ";
    
    std::atomic<int> successful_crossings{0};
    std::atomic<int> total_cars{0};

public:
    void arriveFromNorth(int car_id) {
        total_cars++;
        {
            std::unique_lock<std::mutex> lock(mtx);
            north_cars_waiting++;
            std::cout << "Машина " << car_id << " с СЕВЕРА подъехала к мосту. Ожидание..." << std::endl;
            
            // Ждем, пока можно будет проехать
            cv.wait(lock, [this]() {
                return south_cars_on_bridge == 0 && 
                       (current_direction == "СЕВЕР" || current_direction == "НЕТ");
            });
            
            north_cars_waiting--;
            north_cars_on_bridge++;
            current_direction = "СЕВЕР";
            
            std::cout << "Машина " << car_id << " с СЕВЕРА начала переезд. На мосту: " 
                      << north_cars_on_bridge << " с севера, " << south_cars_on_bridge << " с юга" << std::endl;
        }
        
        // Имитация времени переезда
        std::this_thread::sleep_for(std::chrono::milliseconds(500 + car_id % 300));
        
        leaveBridge(car_id, "СЕВЕР");
    }
    
    void arriveFromSouth(int car_id) {
        total_cars++;
        {
            std::unique_lock<std::mutex> lock(mtx);
            south_cars_waiting++;
            std::cout << "Машина " << car_id << " с ЮГА подъехала к мосту. Ожидание..." << std::endl;
            
            // Ждем, пока можно будет проехать
            cv.wait(lock, [this]() {
                return north_cars_on_bridge == 0 && 
                       (current_direction == "ЮГ" || current_direction == "НЕТ");
            });
            
            south_cars_waiting--;
            south_cars_on_bridge++;
            current_direction = "ЮГ";
            
            std::cout << "Машина " << car_id << " с ЮГА начала переезд. На мосту: " 
                      << north_cars_on_bridge << " с севера, " << south_cars_on_bridge << " с юга" << std::endl;
        }
        
        // Имитация времени переезда
        std::this_thread::sleep_for(std::chrono::milliseconds(500 + car_id % 300));
        
        leaveBridge(car_id, "ЮГ");
    }
    
    int getSuccessfulCrossings() const {
        return successful_crossings.load();
    }
    
    int getTotalCars() const {
        return total_cars.load();
    }
    
    bool allCarsCrossedSuccessfully() const {
        return successful_crossings == total_cars;
    }

private:
    void leaveBridge(int car_id, const std::string& direction) {
        std::unique_lock<std::mutex> lock(mtx);
        
        if (direction == "СЕВЕР") {
            north_cars_on_bridge--;
        } else {
            south_cars_on_bridge--;
        }
        
        successful_crossings++;
        
        // Если все машины уехали с моста в текущем направлении
        if (north_cars_on_bridge == 0 && south_cars_on_bridge == 0) {
            // Решаем, какое направление будет следующим
            if (direction == "СЕВЕР" && south_cars_waiting > 0) {
                current_direction = "ЮГ";
            } else if (direction == "ЮГ" && north_cars_waiting > 0) {
                current_direction = "СЕВЕР";
            } else {
                current_direction = "НЕТ";
            }
        }
        
        std::cout << "Машина " << car_id << " с " << (direction == "NORTH" ? "СЕВЕРА" : "ЮГА") 
                  << " переехала мост. На мосту осталось: " << north_cars_on_bridge 
                  << " с севера, " << south_cars_on_bridge << " с юга" << std::endl;
        std::cout << "Текущее направление: " << current_direction 
                  << ", Ожидают: " << north_cars_waiting << " с севера, " 
                  << south_cars_waiting << " с юга" << std::endl;
        
        // Уведомляем все ожидающие потоки
        cv.notify_all();
    }
};

void simulateTraffic(NarrowBridge& bridge, int num_cars) {
    std::vector<std::thread> cars;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dir_dist(0, 1);
    
    for (int i = 1; i <= num_cars; ++i) {
        if (dir_dist(gen) == 0) {
            cars.emplace_back(&NarrowBridge::arriveFromNorth, &bridge, i);
        } else {
            cars.emplace_back(&NarrowBridge::arriveFromSouth, &bridge, i);
        }
        
        // Уменьшаем задержку между появлением машин
        std::this_thread::sleep_for(std::chrono::milliseconds(100 + gen() % 200));
    }
    
    for (auto& car : cars) {
        if (car.joinable()) {
            car.join();
        }
    }
}

int getNumberOfCars() {
    std::string input;
    int num_cars;
    
    while (true) {
        std::cout << "Введите количество машин (1-50): ";
        std::getline(std::cin, input);
        
        // Простая проверка, что все символы - цифры
        bool is_valid = true;
        for (char c : input) {
            if (!std::isdigit(c)) {
                is_valid = false;
                break;
            }
        }
        
        if (!is_valid || input.empty()) {
            std::cout << "Ошибка: введите целое положительное число!" << std::endl;
            continue;
        }
        
        num_cars = std::stoi(input);
        
        if (num_cars < 1) {
            std::cout << "Ошибка: число должно быть не менее 1!" << std::endl;
        } else if (num_cars > 50) {
            std::cout << "Ошибка: слишком много машин! Максимум 50." << std::endl;
        } else {
            return num_cars;
        }
    }
}

int main() {
    NarrowBridge bridge;
    int num_cars = getNumberOfCars();    
    
    std::cout << "=== МОДЕЛИРОВАНИЕ УЗКОГО МОСТА ===" << std::endl;
    std::cout << "Количество машин: " << num_cars << std::endl;
    std::cout << "=================================" << std::endl;
    
    auto start_time = std::chrono::steady_clock::now();
    
    try {
        simulateTraffic(bridge, num_cars);
        
        auto end_time = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        std::cout << "=================================" << std::endl;
        
        int successful = bridge.getSuccessfulCrossings();
        int total = bridge.getTotalCars();
        
        std::cout << "Время выполнения: " << duration.count() << " мс" << std::endl;
        
        if (successful == total && total == num_cars) {
            std::cout << "УСПЕХ: Все " << successful << " машин успешно переехали мост!" << std::endl;
            std::cout << "Моделирование завершено без ошибок" << std::endl;
        } else {
            std::cout << "ОШИБКА: Переехало только " << successful << " из " << total << " машин" << std::endl;
            std::cout << "Ожидалось: " << num_cars << " машин" << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cout << "=================================" << std::endl;
        std::cout << "КРИТИЧЕСКАЯ ОШИБКА: " << e.what() << std::endl;
    }
    
    return 0;
}
