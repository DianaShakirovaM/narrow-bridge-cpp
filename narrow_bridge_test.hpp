#ifndef NARROW_BRIDGE_TEST_HPP
#define NARROW_BRIDGE_TEST_HPP

#include "narrow_bridge.hpp"
#include <cassert>
#include <iostream>
#include <thread>
#include <vector>

// Базовый класс для тестов
class TestBase {
protected:
    int passed_tests = 0;
    int total_tests = 0;
    
    void test_assert(bool condition, const std::string& test_name) {
        total_tests++;
        if (condition) {
            passed_tests++;
            std::cout << "✓ PASS: " << test_name << std::endl;
        } else {
            std::cout << "✗ FAIL: " << test_name << std::endl;
        }
    }
    
public:
    virtual void run_all_tests() = 0;
    
    void print_summary() const {
        std::cout << "\n=== РЕЗУЛЬТАТЫ ТЕСТИРОВАНИЯ ===" << std::endl;
        std::cout << "Пройдено: " << passed_tests << "/" << total_tests << " тестов" << std::endl;
        if (passed_tests == total_tests) {
            std::cout << "✅ ВСЕ ТЕСТЫ ПРОЙДЕНЫ УСПЕШНО!" << std::endl;
        } else {
            std::cout << "❌ НЕКОТОРЫЕ ТЕСТЫ ПРОВАЛЕНЫ!" << std::endl;
        }
    }
};

#endif