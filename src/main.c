#include <raylib.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>



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
    int textsize = MeasureText(text, fontSize) / 2;
    int posX = GetScreenWidth() / 2.0f - textsize + offsetX * textsize;
    int posY = GetScreenHeight() / 2.0f - fontSize + offsetY * fontSize;
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
            return "Seconds";
        else
            return "Second";
    case MINUTE:
        if (is_plural)
            return "Minutes";
        else
            return "Minute";
    case HOUR:
        if (is_plural)
            return "Hours";
        else
            return "Hour";
    default:
        return "";
    }
}

struct Alert {
    char message[255];
    Color background_color;
    Color text_color;
    bool flash;
    int raw_time;
    bool wants_to_sleep;
    enum Time_Unit raw_time_unit;
};

struct Alert make_alert(void) {
    struct Alert alert = {0};
    TextCopy(alert.message, "Alert!");
    alert.background_color = RAYWHITE;
    alert.text_color = BLACK;
    alert.flash = false;
    alert.raw_time = 0;
    alert.wants_to_sleep = false;
    alert.raw_time_unit = SECOND;

    return alert;

}

struct Alert parse_args(int argc, char **argv) {
    struct Alert alert = make_alert();
    if (argc <= 1)
        return alert;

    for (int arg_index = 1; arg_index < argc; ++arg_index) {
        if (TextIsEqual(argv[arg_index], "message")) {
            TextCopy(alert.message, argv[++arg_index]);
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

bool make_alert_window(struct Alert *alert) {
    InitWindow(800, 600, "Make alert");
    SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()) / 6);

    char buffer[100] = "";
    int fontsize = 50;

    const char *message = "Message:";
    int message_width = MeasureText(message, fontsize);
    bool message_edit_mode = false;

    const char *flash = "Flash: ";
    int flash_width = MeasureText(flash, fontsize);

    const char *background_colors = "White;Black;Red;Green;Blue;Yellow";
    const char *text_colors = "Black;White;Red;Green;Blue;Yellow";
    
    int dropdown_width = MeasureText("Yellow", fontsize) + 15;

    const char *background = "Background:";
    int background_width = MeasureText(background, fontsize);
    bool background_edit_mode = false;
    int background_color = 0;

    const char *text = "Text:";
    int text_width = MeasureText(text, fontsize);
    bool text_edit_mode = false;
    int text_color = 0;

    const char *sleep = "Sleep:";
    int sleep_width = MeasureText(sleep, fontsize);
    int raw_time = 0;
    bool sleep_edit_mode = false;

    const char *units = "Minutes;Hours;Seconds";
    int unit_width = MeasureText("Seconds", fontsize) + 10;
    int which_unit = 0;
    bool unit_edit_mode = false;

    bool should_run = false;

    const char *run = "Run";
    int run_width = MeasureText(run, fontsize) + 10;

    const char *exit = "Exit";
    int exit_width = MeasureText(exit, fontsize) + 10;
    
    GuiSetStyle(DEFAULT, TEXT_SIZE, fontsize);
    GuiSetStyle(DEFAULT, TEXT_SPACING, 3);

    while (!WindowShouldClose()) {
        BeginDrawing(); {
            ClearBackground(RAYWHITE);

            GuiDrawText(message, (Rectangle){ .x=10, .y=50, .width=message_width, .height=fontsize }, 0, BLACK);
            if (GuiTextBox((Rectangle){ .x=10+background_width+10, .y=50, .width=400, .height=fontsize }, buffer, 100, message_edit_mode))
                message_edit_mode = !message_edit_mode;

            GuiDrawText(flash, (Rectangle){ .x=10, .y=50+fontsize+10, .width=flash_width, .height=fontsize }, 0, BLACK);
            alert->flash = GuiCheckBox((Rectangle){ .x=10+background_width+10, .y=50+fontsize+10, .width=fontsize, .height=fontsize }, "", alert->flash);

            if (GuiButton((Rectangle){ .x=10, .y=50+(fontsize+10)*5, .width=run_width, .height=fontsize }, run)) {
                should_run = true;
                break;
            }

            if (GuiButton((Rectangle){ .x=10, .y=50+(fontsize+10)*6, .width=exit_width, .height=fontsize }, exit)) {
                should_run = false;
                break;
            }

            GuiDrawText(sleep, (Rectangle){ .x=10, .y=50+(fontsize+10)*4, .width=sleep_width, .height=fontsize }, 0, BLACK);
            if (GuiValueBox((Rectangle){ .x=20+background_width, .y=50+(fontsize+10)*4, .width=sleep_width, .height=fontsize }, "", &raw_time, 0, 1000, sleep_edit_mode))
                sleep_edit_mode = !sleep_edit_mode;
            if (GuiDropdownBox((Rectangle){ .x=20+background_width+(sleep_width+10), .y=50+(fontsize+10)*4, .width=unit_width, .height=fontsize }, units, &which_unit, unit_edit_mode))
                unit_edit_mode = !unit_edit_mode;

            GuiDrawText(text, (Rectangle){ .x=10, .y=50+(fontsize+10)*3, .width=text_width, .height=fontsize }, 0, BLACK);
            if (GuiDropdownBox((Rectangle){ .x=20+background_width, .y=50+(fontsize+10)*3, .width=dropdown_width, .height=fontsize }, text_colors, &text_color, text_edit_mode))
                text_edit_mode = !text_edit_mode;
            
            GuiDrawText(background, (Rectangle){ .x=10, .y=50+(fontsize+10)*2, .width=background_width, .height=fontsize }, 0, BLACK);
            if (GuiDropdownBox((Rectangle){ .x=10+background_width+10, .y=50+(fontsize+10)*2, .width=dropdown_width, .height=fontsize }, background_colors, &background_color, background_edit_mode))
                background_edit_mode = !background_edit_mode;
        } EndDrawing();
    }

    switch (background_color) {
    case 0:
        alert->background_color = WHITE;
        break;
    case 1:
        alert->background_color = BLACK;
        break;
    case 2:
        alert->background_color = RED;
        break;
    case 3:
        alert->background_color = GREEN;
        break;
    case 4:
        alert->background_color = BLUE;
        break;
    case 5:
        alert->background_color = YELLOW;
        break;
    }

    switch (text_color) {
    case 0:
        alert->text_color = BLACK;
        break;
    case 1:
        alert->text_color = WHITE;
        break;
    case 2:
        alert->text_color = RED;
        break;
    case 3:
        alert->text_color = GREEN;
        break;
    case 4:
        alert->text_color = BLUE;
        break;
    case 5:
        alert->text_color = YELLOW;
        break;
    }

    switch (which_unit) {
    case 0:
        alert->raw_time_unit = MINUTE;
        break;
    case 1:
        alert->raw_time_unit = HOUR;
        break;
    case 2:
        alert->raw_time_unit = SECOND;
    }

    alert->raw_time = raw_time;

    TextCopy(alert->message, buffer);

    CloseWindow();

    return should_run;
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

    if (argc <= 1)
        if (!make_alert_window(&alert))
            return 0;

    do {
        sleep(alert.raw_time * alert.raw_time_unit);

        alert_window(&alert);
    } while (alert.wants_to_sleep && alert.raw_time > 0);

    return 0;
}
