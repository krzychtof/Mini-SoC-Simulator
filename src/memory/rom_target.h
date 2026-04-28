#pragma once
#include <systemc>
#include <tlm>
#include <tlm_utils/simple_target_socket.h>
#include <array>
#include <cstring>
#include <iostream>

namespace soc
{
    // ROM size in bytes (4 kilobytes)
    constexpr std::size_t ROM_SIZE = 4096;

    // ROM Target Module
    // Simulates read-only memory - writes are forbidden
    SC_MODULE(RomTarget)
    {
        // TLM target socket for receiving transactions
        tlm_utils::simple_target_socket<RomTarget> socket{"rom_socket"};

        // Constructor - initializes ROM with pattern data and registers transport methods
        SC_CTOR(RomTarget)
        {
            // Initialize ROM with pattern data (0x00, 0x01, 0x02, ... 0xFF, 0x00, ...)
            for (std::size_t i = 0; i < ROM_SIZE; ++i)
                mem_[i] = static_cast<uint8_t>(i & 0xFF);
            // Register blocking transport method
            socket.register_b_transport(this, &RomTarget::b_transport);
            // Register debug transport method
            socket.register_transport_dbg(this, &RomTarget::transport_dbg);
        }

    private:
        // Internal memory storage (4KB byte array)
        std::array<uint8_t, ROM_SIZE> mem_{};

        // Blocking transport - handles read transactions only
        void b_transport(tlm::tlm_generic_payload & trans, sc_core::sc_time & delay)
        {
            // Check if this is a WRITE command - ROM is read-only!
            if (trans.get_command() == tlm::TLM_WRITE_COMMAND)
            {
                std::cerr << "[ROM] Write FORBIDDEN!\n";
                trans.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
                return;
            }

            // Extract transaction parameters
            const uint64_t addr = trans.get_address();
            const unsigned len = trans.get_data_length();

            // Check if access is within bounds
            if (addr + len > ROM_SIZE)
            {
                trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
                return;
            }

            // Copy data from ROM to transaction buffer
            std::memcpy(trans.get_data_ptr(), &mem_[addr], len);
            std::cout << "[ROM] Read  " << len << "B @ 0x" << std::hex << addr << "\n";

            // Add realistic timing delay (5 nanoseconds for ROM access - faster than RAM)
            delay += sc_core::sc_time(5, sc_core::SC_NS);
            // Set successful response
            trans.set_response_status(tlm::TLM_OK_RESPONSE);
        }

        // Debug transport - for non-intrusive debugging access
        // Only allows reads, even in debug mode
        unsigned int transport_dbg(tlm::tlm_generic_payload & trans)
        {
            const uint64_t addr = trans.get_address();
            const unsigned len = trans.get_data_length();
            // Only allow reads within bounds
            if (trans.get_command() == tlm::TLM_READ_COMMAND && addr + len <= ROM_SIZE)
            {
                std::memcpy(trans.get_data_ptr(), &mem_[addr], len);
                return len; // Return number of bytes transferred
            }
            return 0; // Failed or write attempted
        }
    };

} // namespace soc