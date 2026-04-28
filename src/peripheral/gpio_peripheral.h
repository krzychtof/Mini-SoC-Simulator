#pragma once
#include <systemc>
#include <tlm>
#include <tlm_utils/simple_target_socket.h>
#include <cstring>
#include <iostream>

namespace soc
{
    // GPIO Peripheral - General Purpose Input/Output
    // Simulates 32 digital pins that can be read or written
    SC_MODULE(GpioPeripheral)
    {
        // TLM target socket for receiving transactions from the bus
        tlm_utils::simple_target_socket<GpioPeripheral> socket{"gpio_socket"};

        // Constructor - initializes the GPIO peripheral
        SC_CTOR(GpioPeripheral)
        {
            // Register the blocking transport method for handling transactions
            socket.register_b_transport(this, &GpioPeripheral::b_transport);
            // Register the debug transport method for non-intrusive access
            socket.register_transport_dbg(this, &GpioPeripheral::transport_dbg);
            // Initialize all pins to 0 (LOW state)
            pin_state_ = 0;
        }

    private:
        // Internal state: 32-bit register representing pin states
        // Each bit represents one GPIO pin (0 = LOW, 1 = HIGH)
        uint32_t pin_state_;

        // Blocking transport method - handles read/write transactions
        // This is called when CPU wants to access GPIO registers
        void b_transport(tlm::tlm_generic_payload & trans, sc_core::sc_time & delay)
        {
            // Check if this is a READ operation
            if (trans.get_command() == tlm::TLM_READ_COMMAND)
            {
                // Copy current pin state to the transaction data buffer
                std::memcpy(trans.get_data_ptr(), &pin_state_, 4);
                // Log the read operation
                std::cout << "[GPIO] Read pin_state = 0x" << std::hex << pin_state_ << "\n";
            }
            // Otherwise it's a WRITE operation
            else
            {
                // Copy data from transaction buffer to pin state register
                std::memcpy(&pin_state_, trans.get_data_ptr(), 4);
                // Log the write operation
                std::cout << "[GPIO] Write pin_state = 0x" << std::hex << pin_state_ << "\n";
            }
            
            // Add realistic timing delay (2 nanoseconds for GPIO access)
            delay += sc_core::sc_time(2, sc_core::SC_NS);
            // Set transaction status to OK
            trans.set_response_status(tlm::TLM_OK_RESPONSE);
        }

        // Debug transport method - for non-intrusive debugging access
        // Does not affect simulation time or trigger side effects
        unsigned int transport_dbg(tlm::tlm_generic_payload & trans)
        {
            // Check if this is a READ operation
            if (trans.get_command() == tlm::TLM_READ_COMMAND)
            {
                // Copy current pin state to transaction buffer
                std::memcpy(trans.get_data_ptr(), &pin_state_, 4);
                // Return number of bytes transferred
                return 4;
            }
            // For WRITE in debug mode
            else if (trans.get_command() == tlm::TLM_WRITE_COMMAND)
            {
                // Update pin state from transaction buffer
                std::memcpy(&pin_state_, trans.get_data_ptr(), 4);
                // Return number of bytes transferred
                return 4;
            }
            // Invalid command
            return 0;
        }
    };

} // namespace soc
