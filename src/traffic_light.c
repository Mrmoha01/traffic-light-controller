#include "traffic_light.h"

/* Move to a new state and restart the state timer. */
static void enter_state(TrafficLight *tl, TrafficState next)
{
    tl->state = next;
    tl->seconds_in_state = 0;

    /* The waiting pedestrian is being served now. */
    if (next == STATE_PEDESTRIAN) {
        tl->ped_request = false;
    }
}

/* Duration of a state in seconds (0 = no time limit). */
static uint32_t state_duration(TrafficState state)
{
    switch (state) {
    case STATE_RED:        return RED_TIME_S;
    case STATE_GREEN:      return GREEN_TIME_S;
    case STATE_YELLOW:     return YELLOW_TIME_S;
    case STATE_PEDESTRIAN: return PEDESTRIAN_TIME_S;
    case STATE_NIGHT:      return 0;
    default:               return 0;
    }
}

void traffic_light_init(TrafficLight *tl)
{
    tl->state = STATE_RED;          /* always start in the safe state */
    tl->seconds_in_state = 0;
    tl->ped_request = false;
}

void traffic_light_handle_event(TrafficLight *tl, TrafficEvent ev)
{
    /* Events that work the same way in (almost) every state. */
    switch (ev) {
    case EVENT_NIGHT_TOGGLE:
        if (tl->state == STATE_NIGHT) {
            enter_state(tl, STATE_RED);      /* leave night mode safely */
        } else {
            tl->ped_request = false;
            enter_state(tl, STATE_NIGHT);
        }
        return;

    case EVENT_BUTTON:
        /* Ignore the button at night, or if pedestrians are already walking. */
        if (tl->state != STATE_NIGHT && tl->state != STATE_PEDESTRIAN) {
            tl->ped_request = true;
        }
        return;

    case EVENT_TICK:
        tl->seconds_in_state++;
        break;                               /* handle timing below */

    case EVENT_NONE:
    default:
        return;
    }

    /* Timed transitions (only reached on EVENT_TICK). */
    switch (tl->state) {
    case STATE_RED:
        if (tl->ped_request) {
            enter_state(tl, STATE_PEDESTRIAN);
        } else if (tl->seconds_in_state >= state_duration(STATE_RED)) {
            enter_state(tl, STATE_GREEN);
        }
        break;

    case STATE_GREEN:
        if (tl->seconds_in_state >= state_duration(STATE_GREEN) ||
            (tl->ped_request && tl->seconds_in_state >= MIN_GREEN_S)) {
            enter_state(tl, STATE_YELLOW);
        }
        break;

    case STATE_YELLOW:
        if (tl->seconds_in_state >= state_duration(STATE_YELLOW)) {
            enter_state(tl, STATE_RED);
        }
        break;

    case STATE_PEDESTRIAN:
        if (tl->seconds_in_state >= state_duration(STATE_PEDESTRIAN)) {
            enter_state(tl, STATE_GREEN);
        }
        break;

    case STATE_NIGHT:
        break;                               /* stays here until toggled */

    default:
        enter_state(tl, STATE_RED);          /* corrupted state -> safe */
        break;
    }
}

Outputs traffic_light_get_outputs(const TrafficLight *tl)
{
    Outputs out = { false, false, false, false };

    switch (tl->state) {
    case STATE_RED:
        out.red = true;
        break;
    case STATE_GREEN:
        out.green = true;
        break;
    case STATE_YELLOW:
        out.yellow = true;
        break;
    case STATE_PEDESTRIAN:
        out.red = true;
        out.walk = true;
        break;
    case STATE_NIGHT:
        out.yellow = (tl->seconds_in_state % 2u) == 0u;   /* blink */
        break;
    default:
        out.red = true;
        break;
    }
    return out;
}

const char *traffic_light_state_name(TrafficState state)
{
    switch (state) {
    case STATE_RED:        return "RED";
    case STATE_GREEN:      return "GREEN";
    case STATE_YELLOW:     return "YELLOW";
    case STATE_PEDESTRIAN: return "PEDESTRIAN";
    case STATE_NIGHT:      return "NIGHT";
    default:               return "UNKNOWN";
    }
}

uint32_t traffic_light_seconds_left(const TrafficLight *tl)
{
    uint32_t total = state_duration(tl->state);

    if (total == 0u || tl->seconds_in_state >= total) {
        return 0u;
    }
    return total - tl->seconds_in_state;
}