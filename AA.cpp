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
        
    }
    
    // Метод для машины, подъезжающей с юга
    void arriveFromSouth(int car_id) {
        total_cars++;
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
        
        // Уведомляем все ожидающие потоки, что состояние изменилось
        cv.notify_all();
    }
};
