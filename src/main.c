#include <raylib.h>
#include <stdio.h>
#include <time.h>

#define SWAP(a,b,T) \
    do {                                        \
        T temp = a;                             \
        a = b;                                  \
        b = temp;                               \
    } while (0)                                 \

void sleep(int seconds) {
    if (seconds <= 0)
        return;
    
    clock_t now = clock();
    int seconds_elapsed = 0;
    do {
        seconds_elapsed = (int)((clock() - now) / CLOCKS_PER_SEC);
    } while (seconds_elapsed < seconds);
}

const char * helpmsg =
    "Name\n"
    "\talerter - a very noticible alerter on a timer\n"
    "\n"
    "Options:\n"
    "message [message]\n"
    "\tThe message to say\n"
    "\n"
    "for [integer] [hour(s)|minute(s)|second(s)]\n"
    "\t When should the program activate and if it should repeat\n"
    "\n"
    "background [RED|GREEN|BLUE|WHITE|BLACK|YELLOW]\n"
    "\t Chooses a color for the background\n"
    "\n"
    "text [RED|GREEN|BLUE|WHITE|BLACK|YELLOW]\n"
    "\t Chooses a color for the text\n"
    "flash"
    "\t If written, the screen will flash\n"
    "\n"
    "EXAMPLES: "
    "\n"
    "alerter message \"Tea is ready\" for 2 minutes and stop"
    "\n"
    "alerter message \"Take a break\" for 1 hour and repeat"
    "\n";

void DrawTextCentered_X(const char * text, int posY, int fontSize, Color color) {
    int monitor = GetCurrentMonitor();
    int textsize = MeasureText(text, fontSize) / 2;    
    int posX = GetMonitorWidth(monitor) / 2.0f - textsize;
    DrawText(text, posX, posY, fontSize, color);
}

void DrawTextCentered(const char * text, int fontSize, Color color) {
    int monitor = GetCurrentMonitor();
    int textsize = MeasureText(text, fontSize) / 2;
    int posX = GetMonitorWidth(monitor) / 2.0f - textsize;
    int posY = GetMonitorHeight(monitor) / 2.0f - textsize;
    DrawText(text, posX, posY, fontSize, color);
}

int main(int argc, char **argv) {
    SetTraceLogLevel(LOG_NONE);

    // begin arg parsing

    if (argc <= 1) {
        puts(helpmsg);
        return 1;
    }

    const char *message = "ALERT!";
    Color background_color = RAYWHITE;
    Color text_color = BLACK;
    int flash = 0;
    int timer_length_seconds = 0;
    int given_time = 0;
    const char * time_unit = "";

    for (int arg_index = 1; arg_index < argc; ++arg_index) {
        if (TextIsEqual(argv[arg_index], "message")) {
            message = argv[++arg_index];
        } else if (TextIsEqual(argv[arg_index], "background")) {
            if (arg_index + 1 >= argc) {
                fprintf(stderr, "No color provided after 'background'\n");
                return 1;
            }

            ++arg_index;

            if (TextIsEqual(argv[arg_index], "red"))
                background_color = RED;
            else if (TextIsEqual(argv[arg_index], "green"))
                background_color = GREEN;
            else if (TextIsEqual(argv[arg_index], "blue"))
                background_color = BLUE;
            else if (TextIsEqual(argv[arg_index], "white"))
                background_color = WHITE;
            else if (TextIsEqual(argv[arg_index], "black"))
                background_color = BLACK;
            else if (TextIsEqual(argv[arg_index], "yellow"))
                background_color = YELLOW;
            else {
                fprintf(stderr, "Invalid background color: %s\n", argv[arg_index]);
                return 1;
            }
        } else if (TextIsEqual(argv[arg_index], "text")) {
            if (arg_index + 1 >= argc) {
                fprintf(stderr, "No color provided after 'text'\n");
                return 1;
            }

            ++arg_index;

            if (TextIsEqual(argv[arg_index], "red"))
                text_color = RED;
            else if (TextIsEqual(argv[arg_index], "green"))
                text_color = GREEN;
            else if (TextIsEqual(argv[arg_index], "blue"))
                text_color = BLUE;
            else if (TextIsEqual(argv[arg_index], "white"))
                text_color = WHITE;
            else if (TextIsEqual(argv[arg_index], "black"))
                text_color = BLACK;
            else if (TextIsEqual(argv[arg_index], "yellow"))
                text_color = YELLOW;
            else {
                fprintf(stderr, "Invalid text color: %s\n", argv[arg_index]);
                return 1;
            }
        } else if (TextIsEqual(argv[arg_index], "flash")) {
            flash = 1;
        } else if (TextIsEqual(argv[arg_index], "for")) {
            if (arg_index + 2 >= argc) {
                fprintf(stderr, "malformed 'for' argument\n");
                return 1;
            }

            // error checking how????
            given_time = TextToInteger(argv[++arg_index]);

            if (given_time <= 0) {
                fprintf(stderr, "time should be a positive number but got: %d\n", given_time);
                return 1;
            }
                                     
            ++arg_index;
            
            int multiplier = 1;
            time_unit = argv[arg_index];
            
            if (TextIsEqual(argv[arg_index], "hour") ||
                TextIsEqual(argv[arg_index], "hours"))
                multiplier = 60 * 60;
            else if (TextIsEqual(argv[arg_index], "minute") ||
                     TextIsEqual(argv[arg_index], "minutes"))
                multiplier = 60;
            else if (TextIsEqual(argv[arg_index], "second") ||
                     TextIsEqual(argv[arg_index], "seconds"))
                multiplier = 1;
            else {
                fprintf(stderr, "In for argument: You must choose between hour(s), minute(s), or second(s). Got: %s\n" , argv[arg_index]);
                return 1;
            }

            timer_length_seconds = given_time * multiplier;
        } else {
            fprintf(stderr, "did not expect: %s\n", argv[arg_index]);
            return 1;
        }
    }

    // end arg parsing

    int should_sleep = 0;
    do {
        sleep(timer_length_seconds);

        InitWindow(0, 0, "ALERT!");

        int fps = GetMonitorRefreshRate(GetCurrentMonitor()) / 6;
        SetTargetFPS(fps);

        SetWindowState(FLAG_WINDOW_MAXIMIZED);
    
        int fontsize = 50;
        int frame = 1;

        int height = GetScreenHeight();

        while(!WindowShouldClose()) {
            BeginDrawing(); {
                if (flash)
                    if (frame % fps == 0 || frame % (fps/2) == 0)
                        SWAP(background_color, text_color, Color);
            
                ClearBackground(background_color);            
                DrawTextCentered(message, fontsize, text_color);
                if (timer_length_seconds > 0) {
                    DrawTextCentered_X(TextFormat("Press Y to Repeat for %d %s or Press Q to Quit",
                                                  given_time, time_unit), height / 1.5, 30, text_color);
            
                    if (IsKeyPressed(KEY_Y)) {
                        should_sleep = 1;
                        break;
                    }
                } else {
                    DrawTextCentered_X("Press Q to Quit", height / 1.5, 30, text_color);
                }
            
                if (IsKeyPressed(KEY_Q)) {
                    should_sleep = 0;
                    break;
                }
            } EndDrawing();

            ++frame;
        }

        CloseWindow();
    } while (should_sleep && timer_length_seconds);

    return 0;
}
