#ifndef TRAFFIC_LIGHT_H
#define TRAFFIC_LIGHT_H

#include <stdbool.h>
#include <stdint.h>

/* ---- Timing (in seconds) ---------------------------------------- */
#define RED_TIME_S        6u   /* cars stopped before green          */
#define GREEN_TIME_S     10u   /* normal green phase                 */
#define YELLOW_TIME_S     3u   /* warning phase                      */
#define PEDESTRIAN_TIME_S 6u   /* pedestrians may cross              */
#define MIN_GREEN_S       3u   /* green must last this long before a
                                  pedestrian request can cut it short */

/* ---- States ------------------------------------------------------ */
typedef enum {
    STATE_RED,
    STATE_GREEN,
    STATE_YELLOW,
    STATE_PEDESTRIAN,   /* cars red, pedestrians walk */
    STATE_NIGHT         /* blinking yellow            */
} TrafficState;

/* ---- Events ------------------------------------------------------ */
typedef enum {
    EVENT_NONE,
    EVENT_TICK,           /* one second has passed        */
    EVENT_BUTTON,         /* pedestrian pressed the button */
    EVENT_NIGHT_TOGGLE    /* night mode on/off            */
} TrafficEvent;

/* ---- Controller data -------------------------------------------- */
typedef struct {
    TrafficState state;
    uint32_t     seconds_in_state;
    bool         ped_request;   /* a pedestrian is waiting */
} TrafficLight;

/* What the physical lights should show right now. */
typedef struct {
    bool red;
    bool yellow;
    bool green;
    bool walk;      /* pedestrian "WALK" signal */
} Outputs;

void         traffic_light_init(TrafficLight *tl);
void         traffic_light_handle_event(TrafficLight *tl, TrafficEvent ev);
Outputs      traffic_light_get_outputs(const TrafficLight *tl);
const char  *traffic_light_state_name(TrafficState state);
uint32_t     traffic_light_seconds_left(const TrafficLight *tl);

#endif /* TRAFFIC_LIGHT_H */