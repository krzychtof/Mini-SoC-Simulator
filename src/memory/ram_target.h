#pragma once
#include <systemc>
#include <tlm>
#include <tlm_utils/simple_target_socket.h>
#include <array>
#include <cstring>
#include <iostream>
#include <stdexcept>

namespace soc
{
    // RAM size in bytes (4 kilobytes)
    constexpr std::size_t RAM_SIZE = 4096;

    // RAM Target Module
    // Simulates random access memory with read/write capabilities
    SC_MODULE(RamTarget)
    {
        // TLM target socket for receiving transactions
        tlm_utils::simple_target_socket<RamTarget> socket{"ram_socket"};

        // Constructor - initializes RAM and registers transport methods
        SC_CTOR(RamTarget)
        {
            // Clear all memory to zero
            mem_.fill(0);
            // Register blocking transport method
            socket.register_b_transport(this, &RamTarget::b_transport);
            // Register DMI (Direct Memory Interface) method for fast access
            socket.register_get_direct_mem_ptr(this, &RamTarget::get_direct_mem_ptr);
            // Register debug transport method
            socket.register_transport_dbg(this, &RamTarget::transport_dbg);
        }

        // Operator overload for direct memory access (for testing/debugging)
        uint8_t &operator[](std::size_t idx)
        {
            if (idx >= RAM_SIZE)
                throw std::out_of_range("RAM: out of range");
            return mem_[idx];
        }

    private:
        // Internal memory storage (4KB byte array)
        std::array<uint8_t, RAM_SIZE> mem_{};

        // Blocking transport - handles read/write transactions
        void b_transport(tlm::tlm_generic_payload & trans, sc_core::sc_time & delay)
        {
            // Extract transaction parameters
            const uint64_t addr = trans.get_address();
            uint8_t *const ptr = trans.get_data_ptr();
            const unsigned len = trans.get_data_length();
            
            // Check if access is within bounds
            if (addr + len > RAM_SIZE)
            {
                trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
                return;
            }
            
            // Handle WRITE command
            if (trans.get_command() == tlm::TLM_WRITE_COMMAND)
            {
                // Copy data from transaction buffer to memory
                std::memcpy(&mem_[addr], ptr, len);
                std::cout << "[RAM] Write " << len << "B @ 0x" << std::hex << addr << "\n";
            }
            // Handle READ command
            else
            {
                // Copy data from memory to transaction buffer
                std::memcpy(ptr, &mem_[addr], len);
                std::cout << "[RAM] Read  " << len << "B @ 0x" << std::hex << addr << "\n";
            }
            
            // Add realistic timing delay (10 nanoseconds for RAM access)
            delay += sc_core::sc_time(10, sc_core::SC_NS);
            // Set successful response
            trans.set_response_status(tlm::TLM_OK_RESPONSE);
        }

        // DMI (Direct Memory Interface) - allows direct memory access for performance
        // Returns pointer to internal memory for fast access without transactions
        bool get_direct_mem_ptr(tlm::tlm_generic_payload &, tlm::tlm_dmi & dmi)
        {
            // Provide pointer to memory array
            dmi.set_dmi_ptr(mem_.data());
            // Set valid address range
            dmi.set_start_address(0);
            dmi.set_end_address(RAM_SIZE - 1);
            // Allow both read and write access
            dmi.set_granted_access(tlm::tlm_dmi::DMI_ACCESS_READ_WRITE);
            // Set latency for DMI accesses
            dmi.set_read_latency(sc_core::sc_time(5, sc_core::SC_NS));
            dmi.set_write_latency(sc_core::sc_time(5, sc_core::SC_NS));
            return true;  // DMI granted
        }

        // Debug transport - for non-intrusive debugging access
        // Does not affect simulation time or trigger side effects
        unsigned int transport_dbg(tlm::tlm_generic_payload & trans)
        {
            const uint64_t addr = trans.get_address();
            const unsigned len = trans.get_data_length();
            // Check bounds
            if (addr + len > RAM_SIZE)
                return 0;
            // Handle write
            if (trans.get_command() == tlm::TLM_WRITE_COMMAND)
                std::memcpy(&mem_[addr], trans.get_data_ptr(), len);
            // Handle read
            else
                std::memcpy(trans.get_data_ptr(), &mem_[addr], len);
            return len;  // Return number of bytes transferred
        }
    };

} // namespace soc