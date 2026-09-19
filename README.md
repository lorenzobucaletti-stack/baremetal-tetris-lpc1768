# Bare-Metal Embedded Tetris on ARM Cortex-M3 (NXP LPC1768)

[![Target: NXP LPC1768](https://img.shields.io/badge/MCU-NXP%20LPC1768%20(ARM%20Cortex--M3)-007ACC.svg)](https://www.nxp.com/)
[![IDE: Keil uVision5](https://img.shields.io/badge/IDE-Keil%20µVision5-blue.svg)](https://www.keil.com/)
[![Language: C](https://img.shields.io/badge/Language-C%20%2F%20Assembly-orange.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

Bare-metal implementation of the classic Tetris game developed for the **LandTiger development board** powered by the **NXP LPC1768 (ARM Cortex-M3)** microcontroller[cite: 30]. The system runs directly on bare silicon without an RTOS, leveraging hardware timers, nested vectored interrupt controllers (NVIC), memory-mapped registers, direct-drive TFT display routines, onboard ADC conversion, and real-time audio synthesis[cite: 30].

---

## Technical Highlights & Hardware Architecture

- **Bare-Metal Event-Driven Design:** Operates without operating system overhead; all timing constraints, state transitions, and user inputs are mediated via hardware ISRs and polling loops[cite: 30].
- **Core Peripheral Drivers:**
  - **Hardware Timers (`timer/`):** Configured with match registers to govern the gravitational fall speed (1 block/s baseline, 2 blocks/s during soft drop) and provide reference clock ticks[cite: 30].
  - **Repetitive Interrupt Timer (`RIT/`):** Periodic sampling engine for mechanical input debouncing (directional joystick and push buttons)[cite: 30].
  - **External Interrupts (`button_EXINT/`):** Edge-triggered asynchronous interrupts (EINT) assigned to physical buttons (KEY1 for Pause/Play toggle, KEY2 for instantaneous Hard Drop)[cite: 30].
  - **Colour TFT Display (`GLCD/`):** Direct low-level frame buffer interface for the onboard 320x240 LCD[cite: 30]. Employs differential redraw algorithms to update only moving, rotated, or cleared blocks, preventing screen flickering[cite: 30].
  - **Touch Digitizer (`TouchPanel/`):** Low-level driver routines configured for resistive touch screen detection.
- **Advanced Extensions (Part 2):**
  - **Analog-to-Digital Converter (`IRQ_adc.c`, `lib_adc.c`, `adc.h`):** Interrupt-driven ADC peripheral management sampling onboard potentiometer voltage levels.
  - **Sound & Audio Engine (`music.c`, `music.h`):** Real-time generation of the classic 8-bit Tetris theme and sound cues for game events (line clears, placement, game over).

---

## Game Specifications & Mechanics

| Feature / Module | Technical Description |
| :--- | :--- |
| **Grid Dimensions** | $20 \times 10$ block playing grid on the left screen area; live status panel (Score, High Score, Lines) on the right[cite: 30]. |
| **Drop Speed (Gravity)** | 1 block/second nominal drop speed driven by hardware timer match interrupts[cite: 30]. |
| **Input Controls** | **Joystick Left/Right:** Translation. **Joystick Up:** Clockwise $90^\circ$ rotation[cite: 30]. |
| **Soft & Hard Drop** | **Joystick Down:** Doubles drop frequency to 2 blocks/s[cite: 30]. **KEY2:** Instant vertical hard drop[cite: 30]. |
| **Line Clearing** | Real-time row full-state checks, line elimination, and downward memory cascading[cite: 30]. |
| **Audio & ADC** | Onboard sound engine for music playback with potentiometer-driven analog controls. |
| **Scoring Engine** | Placement: +10 pts; Single line: +110 pts; Multi-line: +$100 \times n$ pts; Tetris (4 lines): +610 pts[cite: 30]. |
| **State Machine** | States: `PAUSED`, `RUNNING`, `GAME_OVER`. Preserves high scores across sessions[cite: 30]. |

---

## Repository Structure

```text
├── docs/
│   └── extrapoints1_tetris.pdf         # Functional & architectural specifications
│
├── src/
│   ├── part1_basic_gameplay/           # Part 1: Core mechanics, timer gravity & display
│   │   ├── CMSIS_core/                 # ARM Cortex-M3 core architecture definitions
│   │   ├── GLCD/                       # Graphic LCD low-level display drivers
│   │   ├── RIT/                        # Repetitive Interrupt Timer for input debouncing
│   │   ├── TouchPanel/                 # Resistive touch controller driver interface
│   │   ├── button_EXINT/               # External interrupt handlers (KEY1, KEY2)
│   │   ├── timer/                      # Hardware timer configuration for gravity & ticks
│   │   │
│   │   ├── sample.c                    # System setup & main initialization loop
│   │   ├── sample.uvprojx              # Keil µVision5 project file
│   │   ├── startup_LPC17xx.s           # Vector table, stack/heap setup & reset handler
│   │   ├── system_LPC17xx.c            # PLL & system clock configuration
│   │   ├── tetris.c                    # Game logic, collision engine & scoring routines
│   │   └── tetris.h                    # Game definitions, tetromino matrices & constants
│   │
│   └── part2_advanced_features/        # Part 2: ADC integration, sound engine & gameplay
│       ├── GLCD/                       # Graphic LCD drivers
│       ├── RIT/                        # Input debouncing timer
│       ├── TouchPanel/                 # Touch screen interface
│       ├── button_EXINT/               # Button interrupt routines
│       ├── timer/                      # Hardware timers & audio modulation ticks
│       │
│       ├── IRQ_adc.c                   # ADC conversion interrupt service routine
│       ├── lib_adc.c                   # Low-level ADC initialization & channel setup
│       ├── adc.h                       # ADC register definitions & control prototypes
│       ├── music.c                     # Sound synthesis routines & music sequence player
│       ├── music.h                     # Note frequency definitions & melody arrays
│       ├── sample.c                    # Extended main loop with ADC & audio tasks
│       ├── sample.uvprojx              # Keil µVision5 project file
│       ├── startup_LPC17xx.s           # Startup code & vector table
│       ├── system_LPC17xx.c            # Clock distribution configuration
│       ├── tetris.c                    # Extended game logic & audio trigger hooks
│       └── tetris.h                    # Tetris macros and structures
│
├── .gitignore
├── LICENSE
└── README.md
