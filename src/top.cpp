#include "cpu/cpu_initiator.h"
#include "memory/ram_target.h"
#include "memory/rom_target.h"
#include "peripheral/timer_peripheral.h"
#include "peripheral/gpio_peripheral.h"
#include "bus/simple_bus.h"
#include <systemc>
#include <iostream>

// Main entry point for SystemC simulation
int sc_main(int /*argc*/, char * /*argv*/[])
{
    // Print banner
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════╗\n";
    std::cout << "║   Mini SoC Simulator v1.0.0      ║\n";
    std::cout << "║   SystemC/TLM-2.0 | C++17        ║\n";
    std::cout << "╚══════════════════════════════════╝\n";

    // Instantiate all SoC components
    soc::CpuInitiator cpu("cpu");           // CPU that initiates transactions
    soc::SimpleBus bus("bus");              // Bus for address decoding and routing
    soc::RomTarget rom("rom");              // Read-only memory
    soc::RamTarget ram("ram");              // Random access memory
    soc::TimerPeripheral timer("timer");    // Timer peripheral
    soc::GpioPeripheral gpio("gpio");       // GPIO peripheral

    // Connect all components through TLM sockets
    // CPU connects to bus
    cpu.socket.bind(bus.cpu_socket);
    // Bus connects to all peripherals
    bus.rom_socket.bind(rom.socket);
    bus.ram_socket.bind(ram.socket);
    bus.timer_socket.bind(timer.socket);
    bus.gpio_socket.bind(gpio.socket);

    // Start SystemC simulation (runs until sc_stop() is called)
    sc_core::sc_start();

    // Print simulation end time
    std::cout << "[SIM] Finished at: " << sc_core::sc_time_stamp() << "\n";
    return 0;
}