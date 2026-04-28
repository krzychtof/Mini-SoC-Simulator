#pragma once
#include <systemc>
#include <tlm>
#include <tlm_utils/simple_target_socket.h>
#include <cstring>
#include <iostream>

// Timer Peripheral Module
// Returns the current simulation time when read
// Useful for timing measurements and benchmarking
namespace soc
{
    // SystemC module macro defining TimerPeripheral
    SC_MODULE(TimerPeripheral)
    {
        // TLM target socket for receiving transactions from the bus
        tlm_utils::simple_target_socket<TimerPeripheral> socket{"timer_socket"};

        // Constructor - registers transport methods
        SC_CTOR(TimerPeripheral)
        {
            // Register blocking transport method for normal transactions
            socket.register_b_transport(this, &TimerPeripheral::b_transport);
            // Register debug transport method for non-intrusive access
            socket.register_transport_dbg(this, &TimerPeripheral::transport_dbg);
        }

    private:
        // Blocking transport method - handles read/write transactions
        void b_transport(tlm::tlm_generic_payload & trans, sc_core::sc_time & delay)
        {
            // Check if this is a READ operation
            if (trans.get_command() == tlm::TLM_READ_COMMAND)
            {
                // Get current simulation time and convert to nanoseconds
                uint32_t now_ns = static_cast<uint32_t>(
                    sc_core::sc_time_stamp().to_seconds() * 1e9);
                // Copy time value to transaction data buffer (4 bytes)
                std::memcpy(trans.get_data_ptr(), &now_ns, 4);
                // Log the read operation
                std::cout << "[TIMER] Read = " << now_ns << " ns\n";
            }
            // Otherwise it's a WRITE operation
            else
            {
                // Timer is read-only, ignore writes
                std::cout << "[TIMER] Write ignored\n";
            }
            // Add minimal timing delay (1 nanosecond for timer access)
            delay += sc_core::sc_time(1, sc_core::SC_NS);
            // Set transaction status to OK
            trans.set_response_status(tlm::TLM_OK_RESPONSE);
        }

        // Debug transport method - for non-intrusive debugging access
        // Returns zero in debug mode (doesn't affect simulation time)
        unsigned int transport_dbg(tlm::tlm_generic_payload & trans)
        {
            // Check if this is a READ operation
            if (trans.get_command() == tlm::TLM_READ_COMMAND)
            {
                // Return zero for debug reads (time doesn't advance in debug mode)
                uint32_t zero = 0;
                std::memcpy(trans.get_data_ptr(), &zero, 4);
                // Return number of bytes transferred
                return 4;
            }
            // Write or invalid command
            return 0;
        }
    };

// End of soc namespace
} // namespace soc