#include <stdio.h>
#include "traffic_light.h"

static int failures = 0;

#define CHECK(cond, msg)                                         \
    do {                                                         \
        if (cond) { printf("  PASS: %s\n", msg); }               \
        else      { printf("  FAIL: %s\n", msg); failures++; }   \
    } while (0)

static void tick(TrafficLight *tl, unsigned n)
{
    for (unsigned i = 0; i < n; i++) {
        traffic_light_handle_event(tl, EVENT_TICK);
    }
}

static void test_normal_cycle(void)
{
    TrafficLight tl;
    printf("Normal cycle\n");
    traffic_light_init(&tl);

    CHECK(tl.state == STATE_RED, "starts in RED");
    tick(&tl, RED_TIME_S);
    CHECK(tl.state == STATE_GREEN, "RED -> GREEN after red time");
    tick(&tl, GREEN_TIME_S);
    CHECK(tl.state == STATE_YELLOW, "GREEN -> YELLOW after green time");
    tick(&tl, YELLOW_TIME_S);
    CHECK(tl.state == STATE_RED, "YELLOW -> RED after yellow time");
}

static void test_pedestrian(void)
{
    TrafficLight tl;
    printf("Pedestrian button\n");
    traffic_light_init(&tl);
    tick(&tl, RED_TIME_S);                       /* now GREEN */

    traffic_light_handle_event(&tl, EVENT_BUTTON);
    tick(&tl, MIN_GREEN_S - 1u);
    CHECK(tl.state == STATE_GREEN, "green is not cut shorter than minimum");
    tick(&tl, 1);
    CHECK(tl.state == STATE_YELLOW, "GREEN -> YELLOW early after request");
    tick(&tl, YELLOW_TIME_S);
    CHECK(tl.state == STATE_RED, "YELLOW -> RED");
    tick(&tl, 1);
    CHECK(tl.state == STATE_PEDESTRIAN, "RED -> PEDESTRIAN");
    CHECK(traffic_light_get_outputs(&tl).walk, "WALK signal is on");
    CHECK(traffic_light_get_outputs(&tl).red, "cars still see RED");
    tick(&tl, PEDESTRIAN_TIME_S);
    CHECK(tl.state == STATE_GREEN, "PEDESTRIAN -> GREEN");
    CHECK(!tl.ped_request, "request was cleared");
}

static void test_night_mode(void)
{
    TrafficLight tl;
    printf("Night mode\n");
    traffic_light_init(&tl);

    traffic_light_handle_event(&tl, EVENT_NIGHT_TOGGLE);
    CHECK(tl.state == STATE_NIGHT, "night mode turns on");
    traffic_light_handle_event(&tl, EVENT_BUTTON);
    CHECK(!tl.ped_request, "button ignored at night");
    CHECK(traffic_light_get_outputs(&tl).yellow, "yellow on at second 0");
    tick(&tl, 1);
    CHECK(!traffic_light_get_outputs(&tl).yellow, "yellow off at second 1 (blink)");
    tick(&tl, 100);
    CHECK(tl.state == STATE_NIGHT, "stays in night mode");
    traffic_light_handle_event(&tl, EVENT_NIGHT_TOGGLE);
    CHECK(tl.state == STATE_RED, "night mode off -> RED");
}

static void test_safety(void)
{
    TrafficLight tl;
    printf("Safety\n");
    traffic_light_init(&tl);

    for (unsigned i = 0; i < 200; i++) {
        Outputs o;
        traffic_light_handle_event(&tl, EVENT_TICK);
        if (i % 7 == 0) { traffic_light_handle_event(&tl, EVENT_BUTTON); }
        o = traffic_light_get_outputs(&tl);
        if (o.green && (o.red || o.yellow)) {
            CHECK(false, "green never shown together with red/yellow");
            return;
        }
        if (o.walk && !o.red) {
            CHECK(false, "WALK only when cars have red");
            return;
        }
    }
    CHECK(true, "no unsafe light combinations in 200 seconds");
}

int main(void)
{
    test_normal_cycle();
    test_pedestrian();
    test_night_mode();
    test_safety();

    if (failures == 0) {
        printf("\nAll tests passed.\n");
        return 0;
    }
    printf("\n%d test(s) FAILED.\n", failures);
    return 1;
}