# Example Simulation Output

This file shows the expected output when running the Mini SoC Simulator.

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
[TIMER] Read = 20 ns
[CPU] Timer       = 20 ns
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

[SIM] Finished at: 85 ns
```

## Output Explanation

### Initialization
- Banner shows simulator version and technology stack

### Test Sequence

1. **RAM Write Test** (0x1000)
   - CPU writes 0xDEADBEEF to RAM address 0x1000
   - Bus decodes global address 0x1000 to RAM local address 0x0

2. **RAM Read Test** (0x1000)
   - CPU reads back from RAM to verify write
   - Returns 0xDEADBEEF confirming successful write

3. **ROM Read Test** (0x0000)
   - CPU reads from ROM at address 0x0000
   - ROM contains pattern data (0x00, 0x01, 0x02, 0x03)
   - Returns 0x03020100 (little-endian format)

4. **Timer Read Test** (0x2000)
   - CPU reads current simulation time from Timer
   - Returns 20 ns (time elapsed since simulation start)

5. **GPIO Write Test** (0x3000)
   - CPU writes 0xFF to GPIO (sets all pins HIGH)
   - GPIO peripheral stores the pin state

6. **GPIO Read Test** (0x3000)
   - CPU reads back GPIO state
   - Returns 0xFF confirming all pins are HIGH

7. **Burst Write Test** (0x1010-0x101C)
   - CPU writes 4 sequential values to RAM:
     - 0x1010 → 0x11111111
     - 0x1014 → 0x22222222
     - 0x1018 → 0x33333333
     - 0x101C → 0x44444444

8. **Burst Read Test** (0x1010-0x101C)
   - CPU reads back all 4 values
   - Confirms all writes were successful

### Timing Analysis
- Total simulation time: 85 nanoseconds
- Includes:
  - CPU wait states (10ns + 5ns + multiple 5ns waits)
  - Memory access delays (ROM: 5ns, RAM: 10ns)
  - Peripheral access delays (Timer: 1ns, GPIO: 2ns)
  - Bus routing overhead (1ns per transaction)

### Address Decoding
All transactions show the bus converting global addresses to local addresses:
- Global 0x1000 → RAM local 0x0000
- Global 0x0000 → ROM local 0x0000
- Global 0x2000 → TIMER local 0x0000
- Global 0x3000 → GPIO local 0x0000
