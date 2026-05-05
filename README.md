# Mini SoC Simulator (prototype)

A lightweight System-on-Chip (SoC) simulator built with **SystemC** and **TLM-2.0** for learning and demonstrating hardware/software interaction modeling.

> **⚠️ DISCLAIMER**: This is a **non-commercial educational prototype** created for learning purposes only. It is a free learning tool developed as a fun experiment with AI assistance. Not intended for production use.

## 🎯 Project Overview

This project simulates a complete SoC architecture including:
- CPU with transaction-based memory access
- Simple bus with address decoding
- ROM (Read-Only Memory)
- RAM (Random Access Memory)
- Timer peripheral
- GPIO peripheral (General Purpose Input/Output)

## 🏗️ Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                      SYSTEM ON CHIP                         │
│                                                             │
│  ┌──────────────────────────────────────────────────────┐  │
│  │                    CPU (Initiator)                    │  │
│  │  - Executes program instructions                     │  │
│  │  - Sends READ/WRITE transactions                     │  │
│  │  - Has "cpu_socket" (initiator socket)               │  │
│  └────────────────────┬─────────────────────────────────┘  │
│                       │                                     │
│                       │ TLM-2.0 transactions                │
│                       ▼                                     │
│  ┌──────────────────────────────────────────────────────┐  │
│  │                   BUS (SimpleBus)                     │  │
│  │  Address decoder - routes traffic to correct module  │  │
│  │                                                       │  │
│  │  Memory Map:                                          │  │
│  │  0x0000-0x0FFF → ROM   (4KB, read-only)             │  │
│  │  0x1000-0x1FFF → RAM   (4KB, read/write)            │  │
│  │  0x2000-0x2003 → TIMER (4B, time counter)           │  │
│  │  0x3000-0x3003 → GPIO  (4B, I/O pins)               │  │
│  └───┬──────────┬──────────┬──────────┬─────────────────┘  │
│      │          │          │          │                     │
│  ┌───▼────┐ ┌──▼────┐ ┌───▼──────┐ ┌─▼─────┐             │
│  │  ROM   │ │  RAM  │ │  TIMER   │ │  GPIO │             │
│  │ Target │ │Target │ │  Target  │ │Target │             │
│  │        │ │       │ │          │ │       │             │
│  │ 4096B  │ │ 4096B │ │ Returns  │ │ 32bit │             │
│  │ Read   │ │ R/W   │ │ sim time │ │ pins  │             │
│  │ only   │ │ memory│ │          │ │       │             │
│  └────────┘ └───────┘ └──────────┘ └───────┘             │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

## 🔄 Transaction Flow

```
CPU wants to write to RAM at address 0x1000:

CPU: write32(0x1000, 0xDEADBEEF)
     │
     ├─► [TLM WRITE, addr=0x1000, data=0xDEADBEEF]
     │
     ▼
BUS: Decodes 0x1000 → RAM (local addr = 0x0000)
     │
     ├─► [TLM WRITE, addr=0x0000, data=0xDEADBEEF]
     │
     ▼
RAM: Writes 0xDEADBEEF at address 0x0000
     └─► [OK] + 10ns delay
```

## 📁 Project Structure

```
Mini-SoC-Simulator/
├── src/
│   ├── bus/
│   │   └── simple_bus.h          # Address decoder and router
│   ├── cpu/
│   │   └── cpu_initiator.h       # CPU with test program
│   ├── memory/
│   │   ├── ram_target.h          # RAM implementation (4KB)
│   │   └── rom_target.h          # ROM implementation (4KB)
│   ├── peripheral/
│   │   ├── timer_peripheral.h    # Timer returning simulation time
│   │   └── gpio_peripheral.h     # GPIO for pin control (32 pins)
│   └── top.cpp                   # Top-level integration
├── tests/
│   └── test_main.cpp             # Unit tests (GTest)
├── CMakeLists.txt                # Build configuration
├── vcpkg.json                    # Dependencies manifest
├── README.md                     # This file
├── TESTING.md                    # Testing guide
├── EXAMPLE_OUTPUT.md             # Example simulation output
├── EXECUTION_FLOW.md             # Execution flow diagram
└── .gitignore                    # Git ignore rules
```

## 🚀 Building and Running

### Prerequisites
- C++17 compatible compiler (MSVC 2022, GCC 9+, Clang 10+)
- CMake 3.21+
- vcpkg (for dependency management)
- SystemC library (installed via vcpkg)
- GTest library (installed via vcpkg)

### vcpkg Setup

The project uses vcpkg manifest mode (`vcpkg.json`). Dependencies are automatically installed:
- `systemc` - SystemC library for hardware modeling
- `gtest` - Google Test framework for unit testing

If you have `VCPKG_ROOT` environment variable set, CMake will automatically use it. Otherwise, specify the toolchain file manually.

### Build from Scratch (Clean Build)

#### Windows (MSVC)

```bash
# Step 1: Remove old build directory (if exists)
rmdir /s /q build

# Step 2: Configure with CMake (using vcpkg toolchain)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake -G "Visual Studio 17 2022"

# Step 3: Build the project
cmake --build build --config Release

# Step 4: Run main simulator
cd build\Release && soc_sim.exe

# Step 5: Run unit tests
cd build\Release && soc_tests.exe
```

**Note:** Adjust `C:/vcpkg/` to your actual vcpkg installation path.

#### Linux/macOS

```bash
# Step 1: Remove old build directory (if exists)
rm -rf build

# Step 2: Configure with CMake
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake

# Step 3: Build the project
cmake --build build

# Step 4: Run main simulator
./build/soc_sim

# Step 5: Run unit tests
./build/soc_tests
```

### Quick Build (If vcpkg is already configured)

If `VCPKG_ROOT` environment variable is set in CMakeLists.txt:

```bash
# Windows
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# Linux/macOS
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### Troubleshooting

**Problem:** CMake can't find SystemC
```
Could not find a package configuration file provided by "SystemCLanguage"
```

**Solution:** Specify vcpkg toolchain explicitly:
```bash
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
```

**Problem:** Wrong generator (MinGW vs MSVC)
```
Error: generator : Visual Studio 17 2022
Does not match the generator used previously: MinGW Makefiles
```

**Solution:** Remove build directory and reconfigure:
```bash
rmdir /s /q build  # Windows
rm -rf build       # Linux/macOS
```

## 📊 Example Output

### Main Simulator Output

```
╔══════════════════════════════════╗
║   Mini SoC Simulator v1.0.0      ║
║   SystemC/TLM-2.0 | C++17        ║
╚══════════════════════════════════╝

[CPU] ===== START =====
[BUS] 0x1000 -> RAM local=0x0
[RAM] Write 4B @ 0x0
[BUS] 0x1000 -> RAM local=0x0
[RAM] Read  4B @ 0x0
[CPU] RAM[0x1000] = 0xdeadbeef
[BUS] 0x0 -> ROM local=0x0
[ROM] Read  4B @ 0x0
[CPU] ROM[0x0000] = 0x3020100
[BUS] 0x2000 -> TIMER local=0x0
[TIMER] Read = 10 ns
[CPU] Timer       = 10 ns
[BUS] 0x3000 -> GPIO local=0x0
[GPIO] Write pin_state = 0xff
[BUS] 0x3000 -> GPIO local=0x0
[GPIO] Read pin_state = 0xff
[CPU] GPIO pins   = 0xff
[BUS] 0x1010 -> RAM local=0x10
[RAM] Write 4B @ 0x10
[BUS] 0x1014 -> RAM local=0x14
[RAM] Write 4B @ 0x14
[BUS] 0x1018 -> RAM local=0x18
[RAM] Write 4B @ 0x18
[BUS] 0x101c -> RAM local=0x1c
[RAM] Write 4B @ 0x1c
[BUS] 0x1010 -> RAM local=0x10
[RAM] Read  4B @ 0x10
[CPU] RAM[0x1010] = 0x11111111
[BUS] 0x1014 -> RAM local=0x14
[RAM] Read  4B @ 0x14
[CPU] RAM[0x1014] = 0x22222222
[BUS] 0x1018 -> RAM local=0x18
[RAM] Read  4B @ 0x18
[CPU] RAM[0x1018] = 0x33333333
[BUS] 0x101c -> RAM local=0x1c
[RAM] Read  4B @ 0x1c
[CPU] RAM[0x101c] = 0x44444444
[CPU] ===== DONE =====

Info: /OSCI/SystemC: Simulation stopped by user.
[SIM] Finished at: 35 ns
```

### Unit Tests Output

```
[==========] Running 6 tests from 4 test suites.
[----------] 2 tests from RamTest
[ RUN      ] RamTest.WriteRead
[       OK ] RamTest.WriteRead (0 ms)
[ RUN      ] RamTest.OutOfBounds
[       OK ] RamTest.OutOfBounds (0 ms)
[----------] 2 tests from RomTest
[ RUN      ] RomTest.ReadPattern
[       OK ] RomTest.ReadPattern (0 ms)
[ RUN      ] RomTest.WriteForbidden
[       OK ] RomTest.WriteForbidden (0 ms)
[----------] 1 test from GpioTest
[ RUN      ] GpioTest.WriteRead
[       OK ] GpioTest.WriteRead (0 ms)
[----------] 1 test from TimerTest
[ RUN      ] TimerTest.ReadTime
[       OK ] TimerTest.ReadTime (0 ms)
[==========] 6 tests from 4 test suites ran. (0 ms total)
[  PASSED  ] 6 tests.
```

## 🧪 Testing

### Unit Tests

The project includes comprehensive unit tests using Google Test framework:

#### Test Coverage

1. **RamTest**
   - `WriteRead` - Verifies write and read operations
   - `OutOfBounds` - Tests address boundary checking

2. **RomTest**
   - `ReadPattern` - Validates ROM initialization pattern
   - `WriteForbidden` - Ensures write protection

3. **GpioTest**
   - `WriteRead` - Tests GPIO pin state management

4. **TimerTest**
   - `ReadTime` - Verifies timer functionality

#### Running Tests

```bash
# Build and run tests
cmake --build build --config Release
cd build\Release && soc_tests.exe  # Windows
./build/soc_tests                   # Linux/macOS

# Run with CTest
cd build
ctest -C Release --verbose
```

See [TESTING.md](TESTING.md) for detailed testing documentation.

## 🔑 Key Concepts Demonstrated

### TLM-2.0 (Transaction Level Modeling)
- **Initiator Socket**: Sends transactions (CPU)
- **Target Socket**: Receives transactions (ROM, RAM, Timer, GPIO)
- **Generic Payload**: Contains command, address, data, length
- **Blocking Transport**: Synchronous communication method

### Memory-Mapped I/O
All peripherals are accessed through memory addresses:
- Reading from 0x2000 returns current simulation time
- Writing to 0x3000 controls GPIO pins
- Standard memory operations work for all devices

### Hardware/Software Interaction
- CPU executes software (C++ code simulating instructions)
- Hardware responds with realistic timing delays
- Bus arbitrates access to shared resources

## 🛠️ Technical Details

### Address Decoding
```cpp
Memory Map:
0x0000-0x0FFF (4KB)  → ROM
0x1000-0x1FFF (4KB)  → RAM
0x2000-0x2003 (4B)   → TIMER
0x3000-0x3003 (4B)   → GPIO
```

### Timing Model
- ROM read: 5ns
- RAM read: 10ns
- RAM write: 10ns
- Timer read: 1ns
- GPIO read/write: 2ns

## 📚 What I Learned

### SystemC/TLM
- How to model hardware at transaction level
- Difference between blocking and non-blocking transport
- Simulation time management with sc_time

### SoC Architecture
- Memory mapping and address decoding
- Bus arbitration concepts
- Peripheral integration patterns

### Hardware/Software Co-design
- How CPU communicates with peripherals
- Memory-mapped I/O advantages
- Timing constraints in embedded systems

## 🎯 Why This Project?

This project was created to:
1. **Learn by doing** - Understanding SystemC through practical implementation
2. **Demonstrate skills** - Show ability to work with hardware modeling
3. **Explore SoC concepts** - Gain insight into system-on-chip architecture
4. **Best practices** - Modern C++17, unit testing, documentation
5. **Leverage AI tools** - Use modern development assistance effectively

## 🔮 Future Enhancements

### Planned Features
- [ ] Interrupt controller for event-driven behavior
- [ ] DMA (Direct Memory Access) controller
- [ ] UART peripheral for serial communication
- [ ] Cache memory between CPU and RAM
- [ ] Performance profiling and optimization
- [ ] Non-blocking transport (nb_transport)
- [ ] DMI (Direct Memory Interface) optimization
- [ ] Instruction set simulator
- [ ] Multi-core CPU support

### Advanced Topics
- [ ] Power modeling and estimation
- [ ] Clock domain crossing
- [ ] AXI/AHB bus protocol
- [ ] Virtual platform integration
- [ ] Co-simulation with RTL

## 📝 License

MIT License - Feel free to use for learning purposes

## 🤝 Contributing

This is a learning project, but suggestions and improvements are welcome!

---

**Built with curiosity and AI assistance** 🚀

> This project was created with the support of publicly available AI tools.
