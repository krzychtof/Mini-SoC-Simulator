// Unit tests for Mini SoC Simulator components
// Uses Google Test framework to verify functionality of RAM, ROM, GPIO, and Timer

#include <gtest/gtest.h>
#include <systemc>
#include <tlm_utils/simple_initiator_socket.h>
#include "memory/ram_target.h"
#include "memory/rom_target.h"
#include "peripheral/timer_peripheral.h"
#include "peripheral/gpio_peripheral.h"

// TestInitiator - Helper module for connecting to target components in tests
// Provides an initiator socket that can send TLM transactions to targets
SC_MODULE(TestInitiator) {
    tlm_utils::simple_initiator_socket<TestInitiator> socket;
    SC_CTOR(TestInitiator) : socket("socket") {}
};

// ============================================================================
// RAM Tests - Verify Random Access Memory functionality
// ============================================================================

// Test fixture for RAM component
// Sets up RAM target and test initiator, connects them via TLM sockets
class RamTest : public ::testing::Test {
protected:
    void SetUp() override {
        ram = new soc::RamTarget("ram");
        init = new TestInitiator("init");
        init->socket.bind(ram->socket);  // Connect initiator to RAM target
    }
    void TearDown() override {
        delete ram;
        delete init;
    }
    soc::RamTarget* ram;
    TestInitiator* init;
};

// Test: Write data to RAM and read it back to verify correctness
TEST_F(RamTest, WriteRead) {
    // Create TLM transaction for WRITE operation
    tlm::tlm_generic_payload trans;
    sc_core::sc_time delay = sc_core::SC_ZERO_TIME;
    uint32_t data = 0xABCD1234;
    
    // Configure write transaction
    trans.set_command(tlm::TLM_WRITE_COMMAND);
    trans.set_address(0x100);
    trans.set_data_ptr(reinterpret_cast<uint8_t*>(&data));
    trans.set_data_length(4);
    trans.set_streaming_width(4);
    trans.set_byte_enable_ptr(nullptr);
    trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
    
    // Execute write transaction
    init->socket->b_transport(trans, delay);
    EXPECT_EQ(trans.get_response_status(), tlm::TLM_OK_RESPONSE);
    
    // Configure read transaction to verify write
    uint32_t read_data = 0;
    trans.set_command(tlm::TLM_READ_COMMAND);
    trans.set_data_ptr(reinterpret_cast<uint8_t*>(&read_data));
    trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
    
    // Execute read transaction
    init->socket->b_transport(trans, delay);
    EXPECT_EQ(trans.get_response_status(), tlm::TLM_OK_RESPONSE);
    EXPECT_EQ(read_data, 0xABCD1234);  // Verify data matches
}

// Test: Attempt to access memory beyond RAM boundaries
// Should return ADDRESS_ERROR_RESPONSE
TEST_F(RamTest, OutOfBounds) {
    tlm::tlm_generic_payload trans;
    sc_core::sc_time delay = sc_core::SC_ZERO_TIME;
    uint32_t data = 0;
    
    // Try to read beyond RAM size (4KB = 0x1000)
    trans.set_command(tlm::TLM_READ_COMMAND);
    trans.set_address(0x2000);  // Beyond 4KB boundary
    trans.set_data_ptr(reinterpret_cast<uint8_t*>(&data));
    trans.set_data_length(4);
    trans.set_streaming_width(4);
    trans.set_byte_enable_ptr(nullptr);
    trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
    
    init->socket->b_transport(trans, delay);
    EXPECT_EQ(trans.get_response_status(), tlm::TLM_ADDRESS_ERROR_RESPONSE);
}

// ============================================================================
// ROM Tests - Verify Read-Only Memory functionality
// ============================================================================

// Test fixture for ROM component
class RomTest : public ::testing::Test {
protected:
    void SetUp() override {
        rom = new soc::RomTarget("rom");
        init = new TestInitiator("init");
        init->socket.bind(rom->socket);  // Connect initiator to ROM target
    }
    void TearDown() override {
        delete rom;
        delete init;
    }
    soc::RomTarget* rom;
    TestInitiator* init;
};

// Test: Read ROM initialization pattern
// ROM is initialized with pattern: 0x00, 0x01, 0x02, 0x03, ...
TEST_F(RomTest, ReadPattern) {
    tlm::tlm_generic_payload trans;
    sc_core::sc_time delay = sc_core::SC_ZERO_TIME;
    uint32_t data = 0;
    
    // Read first 4 bytes from ROM
    trans.set_command(tlm::TLM_READ_COMMAND);
    trans.set_address(0);
    trans.set_data_ptr(reinterpret_cast<uint8_t*>(&data));
    trans.set_data_length(4);
    trans.set_streaming_width(4);
    trans.set_byte_enable_ptr(nullptr);
    trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
    
    init->socket->b_transport(trans, delay);
    EXPECT_EQ(trans.get_response_status(), tlm::TLM_OK_RESPONSE);
    EXPECT_EQ(data, 0x03020100);  // Little-endian: bytes 0,1,2,3
}

// Test: Attempt to write to ROM (should be forbidden)
// ROM is read-only, writes should return COMMAND_ERROR_RESPONSE
TEST_F(RomTest, WriteForbidden) {
    tlm::tlm_generic_payload trans;
    sc_core::sc_time delay = sc_core::SC_ZERO_TIME;
    uint32_t data = 0xDEADBEEF;
    
    // Try to write to ROM (should fail)
    trans.set_command(tlm::TLM_WRITE_COMMAND);
    trans.set_address(0);
    trans.set_data_ptr(reinterpret_cast<uint8_t*>(&data));
    trans.set_data_length(4);
    trans.set_streaming_width(4);
    trans.set_byte_enable_ptr(nullptr);
    trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
    
    init->socket->b_transport(trans, delay);
    EXPECT_EQ(trans.get_response_status(), tlm::TLM_COMMAND_ERROR_RESPONSE);
}

// ============================================================================
// GPIO Tests - Verify General Purpose Input/Output functionality
// ============================================================================

// Test fixture for GPIO peripheral
class GpioTest : public ::testing::Test {
protected:
    void SetUp() override {
        gpio = new soc::GpioPeripheral("gpio");
        init = new TestInitiator("init");
        init->socket.bind(gpio->socket);  // Connect initiator to GPIO target
    }
    void TearDown() override {
        delete gpio;
        delete init;
    }
    soc::GpioPeripheral* gpio;
    TestInitiator* init;
};

// Test: Write GPIO pin state and read it back
// GPIO maintains 32-bit pin state register
TEST_F(GpioTest, WriteRead) {
    tlm::tlm_generic_payload trans;
    sc_core::sc_time delay = sc_core::SC_ZERO_TIME;
    uint32_t data = 0xFF;  // Set all pins HIGH
    
    // Write to GPIO register
    trans.set_command(tlm::TLM_WRITE_COMMAND);
    trans.set_address(0);
    trans.set_data_ptr(reinterpret_cast<uint8_t*>(&data));
    trans.set_data_length(4);
    trans.set_streaming_width(4);
    trans.set_byte_enable_ptr(nullptr);
    trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
    
    init->socket->b_transport(trans, delay);
    EXPECT_EQ(trans.get_response_status(), tlm::TLM_OK_RESPONSE);
    
    // Read back GPIO state to verify
    uint32_t read_data = 0;
    trans.set_command(tlm::TLM_READ_COMMAND);
    trans.set_data_ptr(reinterpret_cast<uint8_t*>(&read_data));
    trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
    
    init->socket->b_transport(trans, delay);
    EXPECT_EQ(trans.get_response_status(), tlm::TLM_OK_RESPONSE);
    EXPECT_EQ(read_data, 0xFF);  // Verify pins are HIGH
}

// ============================================================================
// Timer Tests - Verify Timer peripheral functionality
// ============================================================================

// Test fixture for Timer peripheral
class TimerTest : public ::testing::Test {
protected:
    void SetUp() override {
        timer = new soc::TimerPeripheral("timer");
        init = new TestInitiator("init");
        init->socket.bind(timer->socket);  // Connect initiator to Timer target
    }
    void TearDown() override {
        delete timer;
        delete init;
    }
    soc::TimerPeripheral* timer;
    TestInitiator* init;
};

// Test: Read current simulation time from Timer
// Timer returns simulation time in nanoseconds
TEST_F(TimerTest, ReadTime) {
    tlm::tlm_generic_payload trans;
    sc_core::sc_time delay = sc_core::SC_ZERO_TIME;
    uint32_t data = 0;
    
    // Read timer value
    trans.set_command(tlm::TLM_READ_COMMAND);
    trans.set_address(0);
    trans.set_data_ptr(reinterpret_cast<uint8_t*>(&data));
    trans.set_data_length(4);
    trans.set_streaming_width(4);
    trans.set_byte_enable_ptr(nullptr);
    trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
    
    init->socket->b_transport(trans, delay);
    EXPECT_EQ(trans.get_response_status(), tlm::TLM_OK_RESPONSE);
    // Note: Timer value depends on simulation time, so we just verify OK response
}
