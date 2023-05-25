#include <raylib.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

const char *helpmsg =
    "NAME\n"
    "\talerter - a very noticible alerter with timer\n"
    "\n"
    "OPTIONS:\n"
    "message [message]\n"
    "\tThe message to say\n"
    "\n"
    "sleep [integer] [hour(s)|minute(s)|second(s)]\n"
    "\t When should the program activate and if it should repeat\n"
    "\n"
    "background [RED|GREEN|BLUE|WHITE|BLACK|YELLOW]\n"
    "\t Chooses a color for the background\n"
    "\n"
    "text [RED|GREEN|BLUE|WHITE|BLACK|YELLOW]\n"
    "\t Chooses a color for the text\n"
    "\n"
    "flash\n"
    "\t If written, the screen will flash\n"
    "\n"
    "EXAMPLES: \n"
    "\talerter message \"Tea is ready\" sleep 2 minutes\n"
    "\talerter message \"Take a break\" background red text blue flash sleep 1 hour\n";

void DrawTextCentered(const char * text, float offsetX, float offsetY, int fontSize, Color color) {
    int monitor = GetCurrentMonitor();
    int textsize = MeasureText(text, fontSize) / 2;
    int posX = GetMonitorWidth(monitor) / 2.0f - textsize + offsetX * textsize;
    int posY = GetMonitorHeight(monitor) / 2.0f - fontSize + offsetY * fontSize;
    DrawText(text, posX, posY, fontSize, color);
}

void err_and_die(const char *msg) {
    fprintf(stderr, "ERROR: %s\n%s", msg, helpmsg);
    exit(1);
}

enum Time_Unit {
    SECOND = 1,
    MINUTE = 60,
    HOUR = 3600,
};

char *time_unit_tostring(enum Time_Unit tu, bool is_plural) {
    switch (tu) {
    case SECOND:
        if (is_plural)
            return "SECONDS";
        else
            return "SECOND";
    case MINUTE:
        if (is_plural)
            return "MINUTES";
        else
            return "MINUTE";
    case HOUR:
        if (is_plural)
            return "HOURS";
        else
            return "HOUR";
    default:
        return "";
    }
}

struct Alert {
    char *message;
    Color background_color;
    Color text_color;
    bool flash;
    int raw_time;
    bool wants_to_sleep;
    enum Time_Unit raw_time_unit;
};

struct Alert make_alert(void) {
    return (struct Alert) {
        .message = "Alert!",
        .background_color = RAYWHITE,
        .text_color = BLACK,
        .flash = false,
        .raw_time = 0,
        .wants_to_sleep = false,
        .raw_time_unit = SECOND
    };
}

struct Alert parse_args(int argc, char **argv) {
    if (argc <= 1)
        err_and_die("");
    
    struct Alert alert = make_alert();

    for (int arg_index = 1; arg_index < argc; ++arg_index) {
        if (TextIsEqual(argv[arg_index], "message")) {
            alert.message = argv[++arg_index];
        } else if (TextIsEqual(argv[arg_index], "background")) {
            if (arg_index + 1 >= argc)
                err_and_die("No color provided after 'background'\n");

            ++arg_index;

            if (TextIsEqual(argv[arg_index], "red"))
                alert.background_color = RED;
            else if (TextIsEqual(argv[arg_index], "green"))
                alert.background_color = GREEN;
            else if (TextIsEqual(argv[arg_index], "blue"))
                alert.background_color = BLUE;
            else if (TextIsEqual(argv[arg_index], "white"))
                alert.background_color = WHITE;
            else if (TextIsEqual(argv[arg_index], "black"))
                alert.background_color = BLACK;
            else if (TextIsEqual(argv[arg_index], "yellow"))
                alert.background_color = YELLOW;
            else {
                err_and_die(TextFormat("Invalid background color: %s\n", argv[arg_index]));
            }
        } else if (TextIsEqual(argv[arg_index], "text")) {
            if (arg_index + 1 >= argc)
                err_and_die("No color provided after 'text'\n");

            ++arg_index;

            if (TextIsEqual(argv[arg_index], "red"))
                alert.text_color = RED;
            else if (TextIsEqual(argv[arg_index], "green"))
                alert.text_color = GREEN;
            else if (TextIsEqual(argv[arg_index], "blue"))
                alert.text_color = BLUE;
            else if (TextIsEqual(argv[arg_index], "white"))
                alert.text_color = WHITE;
            else if (TextIsEqual(argv[arg_index], "black"))
                alert.text_color = BLACK;
            else if (TextIsEqual(argv[arg_index], "yellow"))
                alert.text_color = YELLOW;
            else {
                err_and_die(TextFormat("Invalid text color: %s\n", argv[arg_index]));
            }
        } else if (TextIsEqual(argv[arg_index], "flash")) {
            alert.flash = true;
        } else if (TextIsEqual(argv[arg_index], "sleep")) {
            if (arg_index + 2 >= argc) {
                err_and_die("malformed 'sleep' argument\n");
            }

            int time = TextToInteger(argv[++arg_index]);
            if (time <= 0)
                err_and_die(TextFormat("time should be a positive number but got: %d\n", time));
            alert.raw_time = time;
            alert.wants_to_sleep = true;
                                     
            ++arg_index;
            
            if (TextIsEqual(argv[arg_index], "hour") ||
                TextIsEqual(argv[arg_index], "hours"))
                alert.raw_time_unit = HOUR;
            else if (TextIsEqual(argv[arg_index], "minute") ||
                     TextIsEqual(argv[arg_index], "minutes"))
                alert.raw_time_unit = MINUTE;
            else if (TextIsEqual(argv[arg_index], "second") ||
                     TextIsEqual(argv[arg_index], "seconds"))
                alert.raw_time_unit = SECOND;
            else {
                err_and_die(TextFormat("In for argument: You must choose between hour(s), minute(s), or second(s). Got: %s\n" , argv[arg_index]));
            }
        } else {
            err_and_die(TextFormat("did not expect: %s\n", argv[arg_index]));
        }
    }

    return alert;
}

void alert_window(struct Alert *alert) {
    InitWindow(0, 0, "ALERT!");

    int fps = GetMonitorRefreshRate(GetCurrentMonitor()) / 6;
    SetTargetFPS(fps);

    SetWindowState(FLAG_WINDOW_MAXIMIZED);
    
    int frame = 1;

    while(!WindowShouldClose()) {
        BeginDrawing(); {
            if (alert->flash)
                if (frame % fps == 0 || frame % (fps/2) == 0) {
                    Color temp = alert->background_color;
                    alert->background_color = alert->text_color;
                    alert->text_color = temp;
                }
            
            ClearBackground(alert->background_color);            
            DrawTextCentered(alert->message, 0, 0, 75, alert->text_color);
            DrawTextCentered("Press F to toggle flashing", 0, 4, 30, alert->text_color);
            if (alert->raw_time > 0) {
                DrawTextCentered(TextFormat("Press SPACE to ignore alert for %d %s or Press Q to Quit",
                                            alert->raw_time, time_unit_tostring(alert->raw_time_unit, alert->raw_time > 1)),
                                 0, 6, 30, alert->text_color);
            
                if (IsKeyPressed(KEY_SPACE)) {
                    alert->wants_to_sleep = true;
                    break;
                }
            } else {
                DrawTextCentered("Press Q to Quit", 0, 6, 30, alert->text_color);
            }

            if (IsKeyPressed(KEY_F))
                alert->flash = !alert->flash;
            
            if (IsKeyPressed(KEY_Q)) {
                alert->wants_to_sleep = false;
                break;
            }
        } EndDrawing();

        ++frame;
    }

    CloseWindow();
    
    return;
}

int main(int argc, char **argv) {
    SetTraceLogLevel(LOG_NONE);
    struct Alert alert = parse_args(argc, argv);

    do {
        sleep(alert.raw_time * alert.raw_time_unit);

        alert_window(&alert);
    } while (alert.wants_to_sleep && alert.raw_time > 0);

    return 0;
}
