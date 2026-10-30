# Traffic Light Controller (C state machine)

A small, hardware-independent traffic light controller written in C. It shows a core firmware pattern: a clean, event-driven **finite state machine** with timed phases, a pedestrian button and a night mode. The logic is separated from the platform code, so it can run on a PC (console simulation) or be ported to a microcontroller.

## Features

- Timed phases: RED, GREEN, YELLOW
- Pedestrian button: requests a crossing, with a minimum green time before green is cut short
- Pedestrian phase: cars see RED, pedestrians see WALK
- Night mode: blinking yellow, button ignored, toggle on/off
- Safe start: always begins in RED, and an invalid state falls back to RED
- Unit tests that check the cycle, the pedestrian flow, night mode and light safety

## State machine

```
              timer                timer              timer
   +-----+ ----------> +-------+ ----------> +--------+ -------+
   | RED |             | GREEN |             | YELLOW |        |
   +-----+ <-----------+-------+             +--------+        |
     |  ^                                         ^             |
     |  +-------------------------------------------------------+
     |  (button pressed -> GREEN ends early after MIN_GREEN_S)
     |
     | pedestrian waiting
     v
 +------------+  timer
 | PEDESTRIAN | ----------> GREEN
 +------------+

 Any state except NIGHT --[night toggle]--> NIGHT
 NIGHT --[night toggle]--> RED
```

| State | Cars | Pedestrians | Leaves when |
|-------|------|-------------|-------------|
| RED | red | don't walk | red time is over (or a pedestrian is waiting -> PEDESTRIAN) |
| GREEN | green | don't walk | green time is over, or a button request after the minimum green |
| YELLOW | yellow | don't walk | yellow time is over |
| PEDESTRIAN | red | WALK | pedestrian time is over |
| NIGHT | blinking yellow | don't walk | night toggle |

Events: `EVENT_TICK` (one second passed), `EVENT_BUTTON`, `EVENT_NIGHT_TOGGLE`.

## Project structure

```
traffic-light-controller/
├── include/traffic_light.h      public interface (states, events, API)
├── src/traffic_light.c          state machine logic (no I/O, portable)
├── src/main.c                   console simulation (keyboard + timing)
├── tests/test_traffic_light.c   unit tests
└── README.md
```

## Build and run

Requires GCC (on Windows: MSYS2/MinGW-w64).

### Windows (PowerShell)

```
gcc -std=c11 -Wall -Wextra -Iinclude src/main.c src/traffic_light.c -o traffic_light.exe
.\traffic_light.exe
```

Tests:

```
gcc -std=c11 -Wall -Wextra -Iinclude tests/test_traffic_light.c src/traffic_light.c -o test_runner.exe
.\test_runner.exe
```

## Controls

| Key | Action |
|-----|--------|
| `b` | pedestrian button |
| `n` | night mode on/off |
| `q` | quit |

## Example output

```
[t=  0s] RED        | Cars: (R)(-)(-) | Pedestrians: DON'T WALK | Left:  6s | Waiting: no
  >> Pedestrian button pressed
[t=  1s] PEDESTRIAN | Cars: (R)(-)(-) | Pedestrians: WALK       | Left:  6s | Waiting: no
```

## Design notes

- **Separation of concerns:** `traffic_light.c` knows nothing about the keyboard, the screen or the clock. It only receives events and reports what the lights should show.
- **No dynamic memory, no globals:** all data lives in one `TrafficLight` struct, as is typical in firmware.
- **Configurable timing:** the phase lengths are `#define`s in `traffic_light.h`.
- **Porting to a microcontroller:** call `traffic_light_handle_event(&tl, EVENT_TICK)` from a 1-second timer interrupt or main-loop tick, send button presses as `EVENT_BUTTON`, and drive the GPIO pins from `traffic_light_get_outputs()`.

## Possible extensions

- Two crossing directions (a four-way junction)
- Button debouncing
- Configurable timings from a config file
- Logging over UART on real hardware
