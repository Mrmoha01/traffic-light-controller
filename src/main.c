/*
 * Traffic light controller - console simulation.
 *
 * Keys:  b = pedestrian button   n = night mode on/off   q = quit
 */

#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h>
#include <stdbool.h>
#include "traffic_light.h"

/* ---- Platform layer: keyboard + sleep ---------------------------- */
#ifdef _WIN32
#include <conio.h>
#include <windows.h>

static void platform_init(void) {}

static int read_key(void)
{
    if (_kbhit()) {
        return _getch();
    }
    return -1;
}

static void sleep_ms(unsigned ms) { Sleep(ms); }

#else
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>
#include <time.h>
#include <stdlib.h>

static struct termios saved_term;
static bool term_saved = false;

static void restore_terminal(void)
{
    if (term_saved) {
        tcsetattr(STDIN_FILENO, TCSANOW, &saved_term);
    }
}

static void platform_init(void)
{
    struct termios raw;

    if (tcgetattr(STDIN_FILENO, &saved_term) == 0) {
        term_saved = true;
        raw = saved_term;
        raw.c_lflag &= (tcflag_t)~(ICANON | ECHO);   /* no Enter needed */
        tcsetattr(STDIN_FILENO, TCSANOW, &raw);
        atexit(restore_terminal);
    }
}

static int read_key(void)
{
    fd_set set;
    struct timeval tv = { 0, 0 };
    unsigned char c;

    FD_ZERO(&set);
    FD_SET(STDIN_FILENO, &set);
    if (select(STDIN_FILENO + 1, &set, NULL, NULL, &tv) > 0) {
        if (read(STDIN_FILENO, &c, 1) == 1) {
            return c;
        }
    }
    return -1;
}

static void sleep_ms(unsigned ms)
{
    struct timespec ts;
    ts.tv_sec = ms / 1000u;
    ts.tv_nsec = (long)(ms % 1000u) * 1000000L;
    nanosleep(&ts, NULL);
}
#endif

/* ---- Display ----------------------------------------------------- */
static void print_status(const TrafficLight *tl, unsigned total_seconds)
{
    Outputs out = traffic_light_get_outputs(tl);

    printf("[t=%3us] %-10s | Cars: (%c)(%c)(%c) | Pedestrians: %-10s | "
           "Left: %2us | Waiting: %s\n",
           total_seconds,
           traffic_light_state_name(tl->state),
           out.red    ? 'R' : '-',
           out.yellow ? 'Y' : '-',
           out.green  ? 'G' : '-',
           out.walk   ? "WALK" : "DON'T WALK",
           (unsigned)traffic_light_seconds_left(tl),
           tl->ped_request ? "yes" : "no");
    fflush(stdout);
}

/* ---- Main -------------------------------------------------------- */
int main(void)
{
    TrafficLight tl;
    unsigned total_seconds = 0;
    bool running = true;

    platform_init();
    traffic_light_init(&tl);

    printf("Traffic light controller simulation\n");
    printf("Keys: b = pedestrian button, n = night mode on/off, q = quit\n\n");
    print_status(&tl, total_seconds);

    while (running) {
        /* Wait one second, but check the keyboard every 50 ms. */
        for (int i = 0; i < 20 && running; i++) {
            int key = read_key();

            if (key == 'b' || key == 'B') {
                traffic_light_handle_event(&tl, EVENT_BUTTON);
                printf("  >> Pedestrian button pressed\n");
            } else if (key == 'n' || key == 'N') {
                traffic_light_handle_event(&tl, EVENT_NIGHT_TOGGLE);
                printf("  >> Night mode toggled\n");
            } else if (key == 'q' || key == 'Q') {
                running = false;
            }
            sleep_ms(50);
        }

        if (running) {
            traffic_light_handle_event(&tl, EVENT_TICK);
            total_seconds++;
            print_status(&tl, total_seconds);
        }
    }

    printf("\nSimulation stopped.\n");
    return 0;
}