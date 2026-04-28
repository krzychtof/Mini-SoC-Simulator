#pragma once
#include <systemc>
#include <tlm>
#include <tlm_utils/simple_initiator_socket.h>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <iomanip>

namespace soc
{
    // CPU Initiator Module
    // Simulates a simple CPU that initiates read/write transactions to memory and peripherals
    SC_MODULE(CpuInitiator)
    {
        // TLM initiator socket for sending transactions to the bus
        tlm_utils::simple_initiator_socket<CpuInitiator> socket{"cpu_socket"};

        // Constructor - registers the run method as a SystemC thread
        SC_CTOR(CpuInitiator) { SC_THREAD(run); }

    private:
        // Write 32-bit data to specified address
        // Creates a TLM transaction and sends it through the socket
        void write32(uint64_t addr, uint32_t data)
        {
            // Create TLM generic payload for the transaction
            tlm::tlm_generic_payload trans;
            // Initialize delay to zero (will be updated by target)
            sc_core::sc_time delay = sc_core::SC_ZERO_TIME;
            // Set transaction type to WRITE
            trans.set_command(tlm::TLM_WRITE_COMMAND);
            // Set target address
            trans.set_address(addr);
            // Set pointer to data buffer
            trans.set_data_ptr(reinterpret_cast<uint8_t *>(&data));
            // Set data length (4 bytes for 32-bit)
            trans.set_data_length(4);
            // Set streaming width (same as data length for simple transfers)
            trans.set_streaming_width(4);
            // No byte enables (all bytes are valid)
            trans.set_byte_enable_ptr(nullptr);
            // DMI (Direct Memory Interface) not used
            trans.set_dmi_allowed(false);
            // Initial response status
            trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
            // Send transaction through socket (blocking call)
            socket->b_transport(trans, delay);
            // Check if transaction was successful
            if (trans.get_response_status() != tlm::TLM_OK_RESPONSE)
                SC_REPORT_ERROR("CPU", "Write FAILED");
        }

        // Read 32-bit data from specified address
        // Creates a TLM transaction and returns the read value
        uint32_t read32(uint64_t addr)
        {
            // Buffer to store read data
            uint32_t data = 0;
            // Create TLM generic payload for the transaction
            tlm::tlm_generic_payload trans;
            // Initialize delay to zero
            sc_core::sc_time delay = sc_core::SC_ZERO_TIME;
            // Set transaction type to READ
            trans.set_command(tlm::TLM_READ_COMMAND);
            // Set source address
            trans.set_address(addr);
            // Set pointer to data buffer (where data will be written)
            trans.set_data_ptr(reinterpret_cast<uint8_t *>(&data));
            // Set data length (4 bytes for 32-bit)
            trans.set_data_length(4);
            // Set streaming width
            trans.set_streaming_width(4);
            // No byte enables
            trans.set_byte_enable_ptr(nullptr);
            // DMI not used
            trans.set_dmi_allowed(false);
            // Initial response status
            trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
            // Send transaction through socket (blocking call)
            socket->b_transport(trans, delay);
            // Check if transaction was successful
            if (trans.get_response_status() != tlm::TLM_OK_RESPONSE)
                SC_REPORT_ERROR("CPU", "Read FAILED");
            // Return the read data
            return data;
        }

        // Main CPU execution thread
        // Simulates a simple program that tests all SoC components
        void run()
        {
            std::cout << "\n[CPU] ===== START =====\n";

            // Test 1: Write to RAM
            write32(0x1000, 0xDEADBEEF);
            // Wait 10 nanoseconds (simulate processing time)
            sc_core::wait(10, sc_core::SC_NS);

            // Test 2: Read back from RAM to verify write
            uint32_t v = read32(0x1000);
            std::cout << "[CPU] RAM[0x1000] = 0x" << std::hex << v << "\n";

            // Test 3: Read from ROM (read-only memory)
            uint32_t r = read32(0x0000);
            std::cout << "[CPU] ROM[0x0000] = 0x" << std::hex << r << "\n";

            // Test 4: Read current simulation time from Timer peripheral
            uint32_t t = read32(0x2000);
            std::cout << "[CPU] Timer       = " << std::dec << t << " ns\n";

            // Test 5: Write to GPIO peripheral (set all pins HIGH)
            write32(0x3000, 0xFF);
            sc_core::wait(5, sc_core::SC_NS);

            // Test 6: Read back GPIO state
            uint32_t gpio = read32(0x3000);
            std::cout << "[CPU] GPIO pins   = 0x" << std::hex << gpio << "\n";

            // Test 7: Write sequence to RAM (burst write)
            for (uint32_t i = 0; i < 4; ++i)
            {
                write32(0x1010 + i * 4, (i + 1) * 0x11111111);
                sc_core::wait(5, sc_core::SC_NS);
            }

            // Test 8: Read back the sequence (burst read)
            for (uint32_t i = 0; i < 4; ++i)
            {
                uint32_t val = read32(0x1010 + i * 4);
                std::cout << "[CPU] RAM[0x" << std::hex << (0x1010 + i * 4)
                          << "] = 0x" << val << "\n";
            }

            std::cout << "[CPU] ===== DONE =====\n\n";
            // Stop the simulation
            sc_core::sc_stop();
        }
    };

} // namespace soc