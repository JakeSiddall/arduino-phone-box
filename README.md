# Arduino Phone Box

A weight-sensing speaker control system designed to encourage phone-free focus time. When a phone is placed in the box, speakers stay ON. When the phone is removed, the speakers automatically turn OFF.

## How It Works

The system uses an HX711 load cell to continuously monitor weight. When the weight drops below a threshold (indicating the phone was removed), a MOSFET circuit cuts power to the speakers.

Key features:
- Rolling average of readings to filter sensor noise
- Consecutive reading requirement to prevent false triggers
- Automatic baseline calibration when phone is placed

## Hardware

- Arduino (Uno, Nano, or similar)
- HX711 load cell amplifier (pins 4 & 5)
- Load cell / weight sensor
- MOSFET transistor circuit (pin 9) for speaker power control
- Speakers or audio amplifier

## Schematic

```
                                 ┌─────────────┐
                                 │   ARDUINO   │
                                 │  (Uno/Nano) │
                                 │             │
                                 │         GND ├────┬──────────┬────────┐
                                 │          5V ├──┐ │          │        │
                                 │             │  │ │          │        │
                                 │     Pin 4 ──┼──┼─┼───┐      │        │
                                 │     Pin 5 ──┼──┼─┼─┐ │      │        │
                                 │             │  │ │ │ │      │        │
                                 │     Pin 9 ──┼──┼─┼─┼─┼──────┼───┐    │
                                 └─────────────┘  │ │ │ │      │   │    │
                                                  │ │ │ │      │   │    │
       ┌──────────────────────────────────────────┘ │ │ │      │   │    │
       │  ┌─────────────────────────────────────────┘ │ │      │   │    │
       │  │                                           │ │      │   │    │
       │  │      ┌────────────────┐                   │ │      │   │    │
       │  │      │     HX711      │                   │ │      │   │    │
       │  │      │  Load Cell ADC │                   │ │      │   │    │
       │  │      │                │                   │ │      │   │    │
       │  └──────┤ GND        VCC ├───────────────────┘ │      │   │    │
       └─────────┤ VCC       DOUT ├─────────────────────┘      │   │    │
                 │            CLK ├────────────────────────────┘   │    │
                 │                │                                │    │
                 │  E+ E- A+ A-   │                                │    │
                 └───┬──┬──┬──┬───┘                                │    │
                     │  │  │  │                                    │    │
                 ┌───┴──┴──┴──┴───┐                                │    │
                 │   LOAD CELL    │                                │    │
                 │  (under box)   │                                │    │
                 │                │                                │    │
                 │  ┌──────────┐  │                                │    │
                 │  │  PHONE   │  │                                │    │
                 │  │   BOX    │  │                                │    │
                 │  └──────────┘  │                                │    │
                 └────────────────┘                                │    │
                                                                   │    │
                     ┌─────────────────────────────────────────────┘    │
                     │                                                  │
                     │    ┌─────────────┐      ┌─────────────────┐      │
                     │    │   MOSFET    │      │    SPEAKERS     │      │
                     │    │  (N-chan)   │      │   / AMPLIFIER   │      │
                     │    │             │      │                 │      │
                     └────┤ Gate        │      │  ┌───┐  ┌───┐   │      │
                          │       Drain ├──────┤  │ L │  │ R │   │      │
                 12V DC ──┤             │      │  └───┘  └───┘   │      │
                          │      Source ├──────┤ GND             │      │
                          └─────────────┘      └────────┬────────┘      │
                                                        │               │
                                                        └───────────────┘
                                                          (common GND)
```

### Wiring Summary

**HX711 → Arduino:**
| HX711 Pin | Arduino Pin |
|-----------|-------------|
| VCC       | 5V          |
| GND       | GND         |
| DOUT      | Pin 4       |
| CLK       | Pin 5       |

**MOSFET → Circuit:**
| MOSFET Pin | Connection |
|------------|------------|
| Gate       | Arduino Pin 9 |
| Drain      | Speaker/Amp power input |
| Source     | GND (common with Arduino) |

**Load Cell → HX711:**
| Wire Color | HX711 Pin |
|------------|-----------|
| Red        | E+        |
| Black      | E-        |
| White      | A-        |
| Green      | A+        |

*(Wire colors may vary by manufacturer)*

## Project Structure

```
Sketches/
├── Phocus_Control/     # Main application
├── HX711/              # Interactive calibration tool
├── Load_Cell_Test/     # Load cell testing
└── MOSFET_Test/        # Speaker control testing

libraries/
└── HX711/              # Required library for weight sensor

extras/                 # Optional libraries (not used in main sketch)
```

## Setup

1. Wire the HX711 to pins 4 (DOUT) and 5 (CLK)
2. Wire the MOSFET gate to pin 9
3. Upload `Sketches/HX711/HX711.ino` to calibrate:
   - Use `+`/`-` keys to adjust calibration factor
   - Note the factor when readings match known weights
4. Update `calibration_factor` in `Phocus_Control.ino`
5. Upload `Phocus_Control.ino`

## Configuration

In `Phocus_Control.ino`:
- `calibration_factor`: Tune for your specific load cell (-900 default)
- `timer`: Delay between readings in ms (250 default)
- `range`: Weight tolerance in grams (10 default)
- `history`: Consecutive readings required to trigger (3 default)

## Future Work

The following improvements could further reduce response latency and improve reliability:

### Use Exponential Moving Average
Replace the simple rolling average with an EMA for faster response to weight changes:
```cpp
float alpha = 0.4;  // Higher = faster response (0.0-1.0)
float ema = 0;

// In loop:
ema = alpha * units + (1 - alpha) * ema;
```

### Fixed Weight Threshold
Instead of tracking a moving "nominal" baseline, use a fixed threshold based on known phone weight:
```cpp
int phoneThreshold = 150;  // grams - adjust for your phone

if (average < phoneThreshold) {
  // Phone removed
} else {
  // Phone present
}
```

### Hysteresis for State Changes
Use separate thresholds for ON/OFF transitions to prevent rapid toggling:
```cpp
int onThreshold = 140;   // Turn ON when weight exceeds this
int offThreshold = 120;  // Turn OFF when weight drops below this
```

### Interrupt-Based Reading
The HX711 has a data-ready signal that could trigger readings only when new data is available, rather than polling with delays.

### Configurable Sensitivity
Add a potentiometer to adjust detection sensitivity without re-uploading code.
