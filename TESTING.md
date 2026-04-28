# Testing Guide

## Overview

This project includes comprehensive unit tests using Google Test (GTest) framework to ensure all components work correctly.

## Test Structure

### Test Files
- `tests/test_main.cpp` - Main test file containing all unit tests

### Test Suites

#### 1. RamTest
Tests for RAM (Random Access Memory) component:
- **WriteRead** - Verifies that data written to RAM can be read back correctly
- **OutOfBounds** - Ensures proper error handling for out-of-bounds access

#### 2. RomTest
Tests for ROM (Read-Only Memory) component:
- **ReadPattern** - Validates ROM initialization pattern (0x00, 0x01, 0x02, 0x03...)
- **WriteForbidden** - Ensures write operations are blocked

#### 3. GpioTest
Tests for GPIO (General Purpose Input/Output) peripheral:
- **WriteRead** - Tests GPIO pin state management

#### 4. TimerTest
Tests for Timer peripheral:
- **ReadTime** - Verifies timer returns simulation time correctly

## Running Tests

### Build Tests
```bash
# Configure project
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# Build all targets including tests
cmake --build build --config Release
```

### Run Tests

#### Windows
```bash
build\Release\soc_tests.exe
```

#### Linux/macOS
```bash
./build/soc_tests
```

#### Using CTest
```bash
cd build
ctest -C Release --verbose
```

## Test Results

Expected output:
```
[==========] Running 6 tests from 4 test suites.
[----------] Global test environment set-up.
[----------] 2 tests from RamTest
[ RUN      ] RamTest.WriteRead
[       OK ] RamTest.WriteRead (0 ms)
[ RUN      ] RamTest.OutOfBounds
[       OK ] RamTest.OutOfBounds (0 ms)
[----------] 2 tests from RamTest (0 ms total)

[----------] 2 tests from RomTest
[ RUN      ] RomTest.ReadPattern
[       OK ] RomTest.ReadPattern (0 ms)
[ RUN      ] RomTest.WriteForbidden
[       OK ] RomTest.WriteForbidden (0 ms)
[----------] 2 tests from RomTest (0 ms total)

[----------] 1 test from GpioTest
[ RUN      ] GpioTest.WriteRead
[       OK ] GpioTest.WriteRead (0 ms)
[----------] 1 test from GpioTest (0 ms total)

[----------] 1 test from TimerTest
[ RUN      ] TimerTest.ReadTime
[       OK ] TimerTest.ReadTime (0 ms)
[----------] 1 test from TimerTest (0 ms total)

[----------] Global test environment tear-down
[==========] 6 tests from 4 test suites ran. (0 ms total)
[  PASSED  ] 6 tests.
```

## Test Implementation Details

### TestInitiator Module

Tests use a helper `TestInitiator` module to properly connect TLM sockets:

```cpp
SC_MODULE(TestInitiator) {
    tlm_utils::simple_initiator_socket<TestInitiator> socket;
    SC_CTOR(TestInitiator) : socket("socket") {}
};
```

### Test Fixture Pattern

Each test suite uses a fixture to set up and tear down components:

```cpp
class RamTest : public ::testing::Test {
protected:
    void SetUp() override {
        ram = new soc::RamTarget("ram");
        init = new TestInitiator("init");
        init->socket.bind(ram->socket);
    }
    void TearDown() override {
        delete ram;
        delete init;
    }
    soc::RamTarget* ram;
    TestInitiator* init;
};
```

### TLM Transaction Creation

Tests create TLM transactions to communicate with components:

```cpp
tlm::tlm_generic_payload trans;
sc_core::sc_time delay = sc_core::SC_ZERO_TIME;
uint32_t data = 0xABCD1234;

trans.set_command(tlm::TLM_WRITE_COMMAND);
trans.set_address(0x100);
trans.set_data_ptr(reinterpret_cast<uint8_t*>(&data));
trans.set_data_length(4);
trans.set_streaming_width(4);
trans.set_byte_enable_ptr(nullptr);
trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

init->socket->b_transport(trans, delay);

EXPECT_EQ(trans.get_response_status(), tlm::TLM_OK_RESPONSE);
```

## Adding New Tests

To add a new test:

1. Create a test fixture class
2. Implement SetUp() and TearDown() methods
3. Write test cases using TEST_F macro
4. Use EXPECT_* or ASSERT_* macros for assertions

Example:
```cpp
class NewComponentTest : public ::testing::Test {
protected:
    void SetUp() override {
        component = new soc::NewComponent("component");
        init = new TestInitiator("init");
        init->socket.bind(component->socket);
    }
    void TearDown() override {
        delete component;
        delete init;
    }
    soc::NewComponent* component;
    TestInitiator* init;
};

TEST_F(NewComponentTest, BasicOperation) {
    // Test implementation
    EXPECT_TRUE(true);
}
```

## Continuous Integration

Tests can be integrated into CI/CD pipelines:

```yaml
# Example GitHub Actions workflow
- name: Build and Test
  run: |
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build --config Release
    cd build
    ctest -C Release --output-on-failure
```

## Test Coverage

Current test coverage:
- RAM: Write, Read, Boundary checking
- ROM: Read, Write protection
- GPIO: State management
- Timer: Time reading

Future test additions:
- Bus address decoding
- CPU instruction execution
- End-to-end integration tests
- Performance benchmarks
