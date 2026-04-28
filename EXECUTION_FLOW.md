# Execution Flow Visualization

This document provides a detailed visualization of how the simulator executes step-by-step.

## Complete Execution Timeline

```
TIME    MODULE   ACTION
════════════════════════════════════════════════════════════════════════

0 ns    [SIM]    Simulation starts
        [CPU]    Thread begins execution
        
        ┌─────────────────────────────────────────────────────┐
        │ TEST 1: Write to RAM                                │
        └─────────────────────────────────────────────────────┘
        
0 ns    [CPU]    write32(0x1000, 0xDEADBEEF)
        [BUS]    Decode 0x1000 → RAM (local 0x0)
        [RAM]    Write 4 bytes at offset 0x0
        [RAM]    Data: 0xDEADBEEF stored
10 ns   [CPU]    Transaction complete (10ns RAM delay)
        [CPU]    wait(10ns) - simulate processing
        
        ┌─────────────────────────────────────────────────────┐
        │ TEST 2: Read from RAM                               │
        └─────────────────────────────────────────────────────┘
        
20 ns   [CPU]    read32(0x1000)
        [BUS]    Decode 0x1000 → RAM (local 0x0)
        [RAM]    Read 4 bytes from offset 0x0
        [RAM]    Return: 0xDEADBEEF
30 ns   [CPU]    Verify: 0xDEADBEEF ✓
        
        ┌─────────────────────────────────────────────────────┐
        │ TEST 3: Read from ROM                               │
        └─────────────────────────────────────────────────────┘
        
30 ns   [CPU]    read32(0x0000)
        [BUS]    Decode 0x0000 → ROM (local 0x0)
        [ROM]    Read 4 bytes from offset 0x0
        [ROM]    Return: 0x03020100 (pattern data)
35 ns   [CPU]    Display: ROM[0x0000] = 0x03020100
        
        ┌─────────────────────────────────────────────────────┐
        │ TEST 4: Read Timer                                  │
        └─────────────────────────────────────────────────────┘
        
35 ns   [CPU]    read32(0x2000)
        [BUS]    Decode 0x2000 → TIMER (local 0x0)
        [TIMER]  Get simulation time: 35ns
        [TIMER]  Return: 35
36 ns   [CPU]    Display: Timer = 35 ns
        
        ┌─────────────────────────────────────────────────────┐
        │ TEST 5: Write to GPIO                               │
        └─────────────────────────────────────────────────────┘
        
36 ns   [CPU]    write32(0x3000, 0xFF)
        [BUS]    Decode 0x3000 → GPIO (local 0x0)
        [GPIO]   Set pin_state = 0xFF (all pins HIGH)
38 ns   [CPU]    Transaction complete (2ns GPIO delay)
        [CPU]    wait(5ns) - simulate processing
        
        ┌─────────────────────────────────────────────────────┐
        │ TEST 6: Read from GPIO                              │
        └─────────────────────────────────────────────────────┘
        
43 ns   [CPU]    read32(0x3000)
        [BUS]    Decode 0x3000 → GPIO (local 0x0)
        [GPIO]   Read pin_state
        [GPIO]   Return: 0xFF
45 ns   [CPU]    Verify: GPIO = 0xFF ✓
        
        ┌─────────────────────────────────────────────────────┐
        │ TEST 7: Burst Write to RAM (4 values)              │
        └─────────────────────────────────────────────────────┘
        
45 ns   [CPU]    write32(0x1010, 0x11111111)
        [BUS]    Decode 0x1010 → RAM (local 0x10)
        [RAM]    Write at offset 0x10
55 ns   [CPU]    wait(5ns)
        
60 ns   [CPU]    write32(0x1014, 0x22222222)
        [BUS]    Decode 0x1014 → RAM (local 0x14)
        [RAM]    Write at offset 0x14
70 ns   [CPU]    wait(5ns)
        
75 ns   [CPU]    write32(0x1018, 0x33333333)
        [BUS]    Decode 0x1018 → RAM (local 0x18)
        [RAM]    Write at offset 0x18
85 ns   [CPU]    wait(5ns)
        
90 ns   [CPU]    write32(0x101C, 0x44444444)
        [BUS]    Decode 0x101C → RAM (local 0x1C)
        [RAM]    Write at offset 0x1C
100 ns  [CPU]    Burst write complete
        
        ┌─────────────────────────────────────────────────────┐
        │ TEST 8: Burst Read from RAM (verify)               │
        └─────────────────────────────────────────────────────┘
        
100 ns  [CPU]    read32(0x1010)
        [RAM]    Return: 0x11111111 ✓
110 ns  
        
110 ns  [CPU]    read32(0x1014)
        [RAM]    Return: 0x22222222 ✓
120 ns  
        
120 ns  [CPU]    read32(0x1018)
        [RAM]    Return: 0x33333333 ✓
130 ns  
        
130 ns  [CPU]    read32(0x101C)
        [RAM]    Return: 0x44444444 ✓
140 ns  
        
140 ns  [CPU]    All tests passed!
        [CPU]    sc_stop() - end simulation
        [SIM]    Simulation complete

════════════════════════════════════════════════════════════════════════
```

## Memory State After Execution

```
ROM (0x0000-0x0FFF) - Read Only
┌──────────┬──────────────────────────────────┐
│ Address  │ Content                          │
├──────────┼──────────────────────────────────┤
│ 0x0000   │ 0x00 0x01 0x02 0x03 (pattern)   │
│ 0x0004   │ 0x04 0x05 0x06 0x07             │
│ ...      │ ...                              │
└──────────┴──────────────────────────────────┘

RAM (0x1000-0x1FFF) - Read/Write
┌──────────┬──────────────────────────────────┐
│ Address  │ Content                          │
├──────────┼──────────────────────────────────┤
│ 0x1000   │ 0xDEADBEEF                       │
│ 0x1004   │ 0x00000000                       │
│ ...      │ ...                              │
│ 0x1010   │ 0x11111111                       │
│ 0x1014   │ 0x22222222                       │
│ 0x1018   │ 0x33333333                       │
│ 0x101C   │ 0x44444444                       │
│ ...      │ ...                              │
└──────────┴──────────────────────────────────┘

TIMER (0x2000-0x2003) - Read Only
┌──────────┬──────────────────────────────────┐
│ Address  │ Content                          │
├──────────┼──────────────────────────────────┤
│ 0x2000   │ Current simulation time (35 ns)  │
└──────────┴──────────────────────────────────┘

GPIO (0x3000-0x3003) - Read/Write
┌──────────┬──────────────────────────────────┐
│ Address  │ Content                          │
├──────────┼──────────────────────────────────┤
│ 0x3000   │ 0x000000FF (all pins HIGH)       │
└──────────┴──────────────────────────────────┘
```

## Transaction Breakdown

### Write Transaction Example
```
CPU → BUS → RAM

1. CPU creates TLM payload:
   - Command: WRITE
   - Address: 0x1000 (global)
   - Data: 0xDEADBEEF
   - Length: 4 bytes

2. BUS receives transaction:
   - Decodes address 0x1000
   - Maps to RAM region
   - Converts to local address 0x0000
   - Forwards to RAM

3. RAM processes:
   - Writes data to memory[0x0000]
   - Adds 10ns delay
   - Returns OK status

4. CPU receives response:
   - Checks status (OK)
   - Continues execution
```

### Read Transaction Example
```
CPU → BUS → TIMER → BUS → CPU

1. CPU creates TLM payload:
   - Command: READ
   - Address: 0x2000 (global)
   - Data buffer: empty
   - Length: 4 bytes

2. BUS receives transaction:
   - Decodes address 0x2000
   - Maps to TIMER region
   - Converts to local address 0x0000
   - Forwards to TIMER

3. TIMER processes:
   - Gets current simulation time (35ns)
   - Writes to data buffer
   - Adds 1ns delay
   - Returns OK status

4. CPU receives response:
   - Reads data from buffer: 35
   - Displays result
```

## Timing Analysis

```
Component Access Times:
┌──────────┬──────────┬─────────────────────┐
│ Module   │ Latency  │ Notes               │
├──────────┼──────────┼─────────────────────┤
│ ROM      │ 5 ns     │ Fast read-only      │
│ RAM      │ 10 ns    │ Read/write memory   │
│ TIMER    │ 1 ns     │ Register access     │
│ GPIO     │ 2 ns     │ Register access     │
│ BUS      │ ~0 ns    │ Routing only        │
└──────────┴──────────┴─────────────────────┘

Total Simulation Time: 35 ns
- Actual operations: ~25 ns
- CPU wait states: ~10 ns
```

## Key Observations

1. **Address Translation**: Bus correctly translates global addresses to local addresses
2. **Memory Persistence**: Data written to RAM persists across reads
3. **Timing Accuracy**: Each component adds realistic delays
4. **Transaction Ordering**: All operations execute in correct sequence
5. **GPIO State**: Peripheral maintains state between accesses

This demonstrates a fully functional SoC simulator with realistic hardware behavior!
