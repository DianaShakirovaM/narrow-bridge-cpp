#ifndef NARROW_BRIDGE_HPP
#define NARROW_BRIDGE_HPP

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <vector>
#include <random>
#include <atomic>
#include <string>

// Класс для моделирования узкого моста
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
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500 + car_id % 300));
        leaveBridge(car_id, "СЕВЕР");
    }
    
    void arriveFromSouth(int car_id) {
        total_cars++;
        {
            std::unique_lock<std::mutex> lock(mtx);
            south_cars_waiting++;
            std::cout << "Машина " << car_id << " с ЮГА подъехала к мосту. Ожидание..." << std::endl;
            
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
        
        if (north_cars_on_bridge == 0 && south_cars_on_bridge == 0) {
            if (direction == "СЕВЕР" && south_cars_waiting > 0) {
                current_direction = "ЮГ";
            } else if (direction == "ЮГ" && north_cars_waiting > 0) {
                current_direction = "СЕВЕР";
            } else {
                current_direction = "НЕТ";
            }
        }
        
        std::cout << "Машина " << car_id << " с " << (direction == "СЕВЕР" ? "СЕВЕРА" : "ЮГА") 
                  << " переехала мост. На мосту осталось: " << north_cars_on_bridge 
                  << " с севера, " << south_cars_on_bridge << " с юга" << std::endl;
        std::cout << "Текущее направление: " << current_direction 
                  << ", Ожидают: " << north_cars_waiting << " с севера, " 
                  << south_cars_waiting << " с юга" << std::endl;
        
        cv.notify_all();
    }
};

#endif