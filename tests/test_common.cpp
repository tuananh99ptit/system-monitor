#include "utils.h"
#include "file_reader.h"
#include "logger.h"
#include <cassert>
#include <iostream>
#include <cmath>

void TestUtils() {
    // Test Trim
    assert(Utils::Trim("  hello  ") == "hello");
    assert(Utils::Trim("\tworld\n") == "world");

    // Test Split
    auto tokens = Utils::Split("cpu  1234  5678", ' ');
    assert(tokens.size() == 3);
    assert(tokens[0] == "cpu");
    assert(tokens[1] == "1234");

    // Test Conversion
    assert(std::abs(Utils::KBToGB(1048576) - 1.0) < 0.0001);

    std::cout << "[PASS] Utils Unit Test" << std::endl;
}

void TestFileReader() {
    std::string line = FileReader::ReadFirstLine("/proc/stat");
    assert(!line.empty());
    assert(line.rfind("cpu", 0) == 0);

    std::cout << "[PASS] FileReader Unit Test" << std::endl;
}

int main() {
    std::cout << "Running Common Module Unit Tests..." << std::endl;
    TestUtils();
    TestFileReader();
    std::cout << "[SUCCESS] All unit tests passed!" << std::endl;
    std::cout << "ALL TESTS PASSED!" << std::endl;
    return 0;
}