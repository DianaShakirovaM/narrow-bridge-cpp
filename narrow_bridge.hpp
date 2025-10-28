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
    void arriveFromNorth(int car_id);
    void arriveFromSouth(int car_id);
    
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
    void leaveBridge(int car_id, const std::string& direction);
};

#endif