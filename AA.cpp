#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <vector>
#include <random>
#include <atomic>

// Класс для моделирования узкого моста
class NarrowBridge {
private:
    // Мьютекс для синхронизации доступа к общим данным
    std::mutex mtx;
    // Условная переменная для координации между потоками
    std::condition_variable cv;
    
    // Счетчики машин на мосту
    int north_cars_on_bridge = 0;    // Машины с севера на мосту
    int south_cars_on_bridge = 0;    // Машины с юга на мосту
    // Счетчики ожидающих машин
    int north_cars_waiting = 0;      // Машины с севера, ожидающие проезда
    int south_cars_waiting = 0;      // Машины с юга, ожидающие проезда
    // Текущее разрешенное направление движения
    std::string current_direction = "НЕТ"; // "СЕВЕР", "ЮГ", "НЕТ"
    
    // Атомарные счетчики для статистики (не требуют мьютекса)
    std::atomic<int> successful_crossings{0}; // Успешные переезды
    std::atomic<int> total_cars{0};           // Общее количество машин

public:
    // Метод для машины, подъезжающей с севера
    void arriveFromNorth(int car_id) {
        total_cars++; // Увеличиваем общий счетчик машин
        
        {
            // Захватываем мьютекс для работы с общими данными
            std::unique_lock<std::mutex> lock(mtx);
            north_cars_waiting++; // Увеличиваем счетчик ожидающих с севера
            std::cout << "Машина " << car_id << " с СЕВЕРА подъехала к мосту. Ожидание..." << std::endl;
            
            // Ждем, пока можно будет проехать
            // Условие: на мосту нет машин с юга И направление разрешено для севера
            cv.wait(lock, [this]() {
                return south_cars_on_bridge == 0 && 
                       (current_direction == "СЕВЕР" || current_direction == "НЕТ");
            });
            
            // Условие выполнено - машина может ехать
            north_cars_waiting--;    // Уменьшаем счетчик ожидающих
            north_cars_on_bridge++;  // Увеличиваем счетчик на мосту
            current_direction = "СЕВЕР"; // Устанавливаем направление движения
            
            std::cout << "Машина " << car_id << " с СЕВЕРА начала переезд. На мосту: " 
                      << north_cars_on_bridge << " с севера, " << south_cars_on_bridge << " с юга" << std::endl;
        } // Мьютекс автоматически освобождается при выходе из блока
        
        // Имитация времени переезда (мьютекс не захвачен - другие машины могут подъезжать)
        std::this_thread::sleep_for(std::chrono::milliseconds(500 + car_id % 300));
        
        // Завершаем переезд
        leaveBridge(car_id, "СЕВЕР");
    }
    
    // Метод для машины, подъезжающей с юга (аналогично северу)
    void arriveFromSouth(int car_id) {
        total_cars++;
        {
            std::unique_lock<std::mutex> lock(mtx);
            south_cars_waiting++;
            std::cout << "Машина " << car_id << " с ЮГА подъехала к мосту. Ожидание..." << std::endl;
            
            // Ждем, пока можно будет проехать
            // Условие: на мосту нет машин с севера И направление разрешено для юга
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
    
    // Геттеры для получения статистики
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
    // Приватный метод для завершения переезда
    void leaveBridge(int car_id, const std::string& direction) {
        // Захватываем мьютекс для изменения общих данных
        std::unique_lock<std::mutex> lock(mtx);
        
        // Уменьшаем счетчик машин на мосту в зависимости от направления
        if (direction == "СЕВЕР") {
            north_cars_on_bridge--;
        } else {
            south_cars_on_bridge--;
        }
        
        // Увеличиваем счетчик успешных переездов
        successful_crossings++;
        
        // Если все машины уехали с моста в текущем направлении
        if (north_cars_on_bridge == 0 && south_cars_on_bridge == 0) {
            // Решаем, какое направление будет следующим
            if (direction == "СЕВЕР" && south_cars_waiting > 0) {
                // Если только что ехали с севера и есть ожидающие с юга - разрешаем юг
                current_direction = "ЮГ";
            } else if (direction == "ЮГ" && north_cars_waiting > 0) {
                // Если только что ехали с юга и есть ожидающие с севера - разрешаем север
                current_direction = "СЕВЕР";
            } else {
                // Если ожидающих нет - мост свободен
                current_direction = "НЕТ";
            }
        }
        
        // Выводим информацию о завершении переезда
        std::cout << "Машина " << car_id << " с " << (direction == "СЕВЕР" ? "СЕВЕРА" : "ЮГА") 
                  << " переехала мост. На мосту осталось: " << north_cars_on_bridge 
                  << " с севера, " << south_cars_on_bridge << " с юга" << std::endl;
        std::cout << "Текущее направление: " << current_direction 
                  << ", Ожидают: " << north_cars_waiting << " с севера, " 
                  << south_cars_waiting << " с юга" << std::endl;
        
        // Уведомляем все ожидающие потоки, что состояние изменилось
        cv.notify_all();
    }
};
