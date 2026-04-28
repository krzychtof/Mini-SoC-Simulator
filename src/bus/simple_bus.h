#pragma once
#include <systemc>
#include <tlm>
#include <tlm_utils/simple_target_socket.h>
#include <tlm_utils/simple_initiator_socket.h>
#include <iostream>

namespace soc
{
    // Simple Bus Module
    // Acts as an interconnect between CPU and peripherals
    // Performs address decoding and routes transactions to appropriate targets
    SC_MODULE(SimpleBus)
    {
        // Target socket - receives transactions from CPU
        tlm_utils::simple_target_socket<SimpleBus> cpu_socket{"bus_cpu"};
        // Initiator sockets - send transactions to peripherals
        tlm_utils::simple_initiator_socket<SimpleBus> rom_socket{"bus_rom"};
        tlm_utils::simple_initiator_socket<SimpleBus> ram_socket{"bus_ram"};
        tlm_utils::simple_initiator_socket<SimpleBus> timer_socket{"bus_timer"};
        tlm_utils::simple_initiator_socket<SimpleBus> gpio_socket{"bus_gpio"};

        // Constructor - registers transport methods
        SC_CTOR(SimpleBus)
        {
            // Register blocking transport for normal transactions
            cpu_socket.register_b_transport(this, &SimpleBus::b_transport);
            // Register debug transport for non-intrusive access
            cpu_socket.register_transport_dbg(this, &SimpleBus::transport_dbg);
        }

    private:
        // Memory region structure - defines address ranges for each peripheral
        struct Region
        {
            uint64_t base, size;  // Base address and size in bytes
            const char *name;     // Human-readable name for logging
        };
        // Memory map definition - maps global addresses to peripherals
        static constexpr Region REGIONS[4] = {
            {0x0000, 0x1000, "ROM"},    // 0x0000-0x0FFF: ROM (4KB)
            {0x1000, 0x1000, "RAM"},    // 0x1000-0x1FFF: RAM (4KB)
            {0x2000, 0x0004, "TIMER"},  // 0x2000-0x2003: Timer (4 bytes)
            {0x3000, 0x0004, "GPIO"},   // 0x3000-0x3003: GPIO (4 bytes)
        };

        // Address decoder - converts global address to local address and target index
        // Returns: target index (0-3) or -1 if address is not mapped
        int decode(uint64_t addr, uint64_t &local) const
        {
            // Check each memory region
            for (int i = 0; i < 4; ++i)
            {
                // Check if address falls within this region
                if (addr >= REGIONS[i].base && addr < REGIONS[i].base + REGIONS[i].size)
                {
                    // Calculate local address (offset from region base)
                    local = addr - REGIONS[i].base;
                    return i;  // Return region index
                }
            }
            return -1;  // Address not mapped to any region
        }

        // Get target socket by region index
        tlm_utils::simple_initiator_socket<SimpleBus> *target_socket(int idx)
        {
            switch (idx)
            {
            case 0:
                return &rom_socket;    // ROM
            case 1:
                return &ram_socket;    // RAM
            case 2:
                return &timer_socket;  // Timer
            case 3:
                return &gpio_socket;   // GPIO
            default:
                return nullptr;        // Invalid index
            }
        }

        // Blocking transport - handles transaction routing
        void b_transport(tlm::tlm_generic_payload & trans, sc_core::sc_time & delay)
        {
            uint64_t local = 0;
            // Decode global address to local address and target
            int idx = decode(trans.get_address(), local);
            // Check if address is valid
            if (idx < 0)
            {
                std::cerr << "[BUS] No mapping for 0x" << std::hex << trans.get_address() << "\n";
                trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
                return;
            }
            // Log the routing decision
            std::cout << "[BUS] 0x" << std::hex << trans.get_address()
                      << " -> " << REGIONS[idx].name << " local=0x" << local << "\n";
            // Update transaction with local address
            trans.set_address(local);
            // Forward transaction to appropriate target
            (*target_socket(idx))->b_transport(trans, delay);
        }

        // Debug transport - for non-intrusive debugging access
        unsigned int transport_dbg(tlm::tlm_generic_payload & trans)
        {
            uint64_t local = 0;
            // Decode address
            int idx = decode(trans.get_address(), local);
            if (idx < 0)
                return 0;  // Invalid address
            // Update to local address
            trans.set_address(local);
            // Forward to target
            return (*target_socket(idx))->transport_dbg(trans);
        }
    };

} // namespace soc