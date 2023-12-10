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
    "\talerter - a very noticeable alerter with timer\n"
    "\n"
    "OPTIONS:\n"
    "message [message]\n"
    "\tThe message to say\n"
    "\n"
    "sleep [integer] [hour(s)|minute(s)|second(s)]\n"
    "\t When should the program activate\n"
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

const char *time_unit_tostring(enum Time_Unit tu, bool is_plural) {
    switch (tu) {
    case SECOND: return is_plural ? "seconds" : "second";
    case MINUTE: return is_plural ? "minutes" : "minute";
    case HOUR:   return is_plural ? "hours"   : "hour";
    default:     return "";
    }
}

bool color_eq(Color *color1, Color *color2) {
    bool a = color1->a == color2->a;
    bool r = color1->r == color2->r;
    bool g = color1->g == color2->g;
    bool b = color1->b == color2->b;

    return a && r && b && g;
}

char *color_tostring(Color *color) {
    if (color_eq(&RED,    color)) return "red";
    if (color_eq(&GREEN,  color)) return "green";
    if (color_eq(&BLUE,   color)) return "blue";
    if (color_eq(&WHITE,  color)) return "white";
    if (color_eq(&BLACK,  color)) return "black";
    if (color_eq(&YELLOW, color)) return "yellow";  
    return "";
}

#define MESSAGE_SIZE 255

struct Alert {
    char message[MESSAGE_SIZE];
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
    alert.raw_time = 1;
    alert.wants_to_sleep = false;
    alert.raw_time_unit = SECOND;

    return alert;
}

void parse_args(int argc, char **argv, struct Alert *alert) {
    for (int arg_index = 1; arg_index < argc; ++arg_index) {
        if (TextIsEqual(argv[arg_index], "message")) {
            TextCopy(alert->message, argv[++arg_index]);
            
        } else if (TextIsEqual(argv[arg_index], "background")) {
            ++arg_index;
            
            if (arg_index >= argc)
                err_and_die("No color provided after 'background'\n");

            if      (TextIsEqual(argv[arg_index], "red"))    alert->background_color = RED;
            else if (TextIsEqual(argv[arg_index], "green"))  alert->background_color = GREEN;
            else if (TextIsEqual(argv[arg_index], "blue"))   alert->background_color = BLUE;
            else if (TextIsEqual(argv[arg_index], "white"))  alert->background_color = WHITE;
            else if (TextIsEqual(argv[arg_index], "black"))  alert->background_color = BLACK;
            else if (TextIsEqual(argv[arg_index], "yellow")) alert->background_color = YELLOW;
            else err_and_die(TextFormat("Invalid background color: %s\n", argv[arg_index]));
            
        } else if (TextIsEqual(argv[arg_index], "text")) {
            ++arg_index;
            
            if (arg_index >= argc)
                err_and_die("No color provided after 'text'\n");

            if      (TextIsEqual(argv[arg_index], "red"))    alert->text_color = RED;
            else if (TextIsEqual(argv[arg_index], "green"))  alert->text_color = GREEN;
            else if (TextIsEqual(argv[arg_index], "blue"))   alert->text_color = BLUE;
            else if (TextIsEqual(argv[arg_index], "white"))  alert->text_color = WHITE;
            else if (TextIsEqual(argv[arg_index], "black"))  alert->text_color = BLACK;
            else if (TextIsEqual(argv[arg_index], "yellow")) alert->text_color = YELLOW;
            else err_and_die(TextFormat("Invalid text color: %s\n", argv[arg_index]));
            
        } else if (TextIsEqual(argv[arg_index], "flash")) {
            alert->flash = true;
            
        } else if (TextIsEqual(argv[arg_index], "sleep")) {
            if (arg_index + 2 >= argc)
                err_and_die("malformed 'sleep' argument\n");

            ++arg_index;

            const int time = TextToInteger(argv[arg_index]);
            if (time <= 0)
                err_and_die(TextFormat("time should be a positive number but got: %d\n", time));
            alert->raw_time = time;
            alert->wants_to_sleep = true;
                                     
            ++arg_index;
            
                 if (TextIsEqual(argv[arg_index], "hour")   || TextIsEqual(argv[arg_index], "hours"))   alert->raw_time_unit = HOUR;
            else if (TextIsEqual(argv[arg_index], "minute") || TextIsEqual(argv[arg_index], "minutes")) alert->raw_time_unit = MINUTE;
            else if (TextIsEqual(argv[arg_index], "second") || TextIsEqual(argv[arg_index], "seconds")) alert->raw_time_unit = SECOND;
            else err_and_die(TextFormat("In for argument: You must choose between hour(s), minute(s), or second(s). Got: %s\n" , argv[arg_index]));
                 
        } else {
            err_and_die(TextFormat("did not expect: %s\n", argv[arg_index]));
        }
    }
}

void gen_desktop_file(struct Alert *alert, const char *name) {
    const char *filepath=
        TextFormat("/home/nathan/.local/share/applications/%s.desktop", name);
    
    FILE *fp = fopen(filepath, "w");
    if (!fp) err_and_die("Can't open file");

    const char *desktop_file =
        "[Desktop Entry]\n"   
        "Version=1.0\n"
        "Name=%s\n"
        "GenericName=alerter\n"
        "Type=Application\n"
        "Exec=alerter message \"%s\" sleep %d %s background %s text %s %s\n"
        "Terminal=false\n"
        "Keywords=time;timer;alarm\n";

    fprintf(fp,
            desktop_file,
            name,
            alert->message,
            alert->raw_time,
            time_unit_tostring(alert->raw_time_unit, false),
            color_tostring(&alert->background_color),
            color_tostring(&alert->text_color),
            alert->flash ? "flash" : "");

    fclose(fp);
}

#define TOGGLE(var, func) do { if ((func)) var = !(var); /* ooh scary! an unhygenic variable assignment! */ clear = true; } while(0)

bool alert_window_editor(struct Alert *alert) {
    InitWindow(800, 600, "Make alert");
    SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()));

    const int fontsize = 50;
    const int width = fontsize;
    const int height = fontsize;
    const int yHeight = fontsize+10;
    const int y = 10;
    const int x = 10;

    bool message_edit_mode = false;

    char save_as_message[MESSAGE_SIZE] = {0};
    bool save_as_edit_mode = false;

    const char *colors = "White;Black;Red;Green;Blue;Yellow";
    
    const int dropdown_width = MeasureText("Yellow", fontsize) + 15;

    const int max_width = MeasureText("Background:", fontsize) + 20;
    bool background_edit_mode = false;
    int background_color = 0;

    bool text_edit_mode = false;
    int text_color = 1;

    bool sleep_edit_mode = false;

    const char *units = "Minutes;Hours;Seconds";
    const int unit_width = MeasureText("Seconds", fontsize) + 25;
    int which_unit = 0;
    bool unit_edit_mode = false;

    bool should_run = false;
    bool should_save = false;

    bool clear = false;

    const Rectangle save_as_button            = { x,                           y+yHeight*7, 200,            height };
    const Rectangle save_as_textbox           = { x+210,                       y+yHeight*7, 400,            height };
    const Rectangle exit_button               = { x,                           y+yHeight*6, 100,            height };
    const Rectangle run_button                = { x,                           y+yHeight*5, 100,            height };
    const Rectangle sleep_value_box           = { max_width,                   y+yHeight*4, dropdown_width, height };
    const Rectangle time_unit_dropdown        = { max_width+dropdown_width+10, y+yHeight*4, unit_width,     height };
    const Rectangle text_color_dropdown       = { max_width,                   y+yHeight*3, dropdown_width, height };
    const Rectangle background_color_dropdown = { max_width,                   y+yHeight*2, dropdown_width, height };
    const Rectangle flash_checkbox            = { max_width,                   y+yHeight*1, width,          height };
    const Rectangle message_textbox           = { max_width,                   y+yHeight*0, 400,            height };

    GuiSetStyle(DEFAULT, TEXT_SIZE, fontsize);
    GuiSetStyle(DEFAULT, TEXT_SPACING, 3);

    BeginDrawing();
    ClearBackground(RAYWHITE);
    EndDrawing();

    while (!WindowShouldClose()) {
        BeginDrawing();
        {
            if (clear) { // ooh efficiency
                ClearBackground(RAYWHITE);
                clear = false;
            }

            DrawText("Sleep:",      x, y+yHeight*4, fontsize, BLACK);
            DrawText("Text:",       x, y+yHeight*3, fontsize, BLACK);
            DrawText("Background:", x, y+yHeight*2, fontsize, BLACK);
            DrawText("Flash:",      x, y+yHeight*1, fontsize, BLACK);
            DrawText("Message:",    x, y+yHeight*0, fontsize, BLACK);

            if (GuiButton(save_as_button, "Save as")) {
                should_save = true;
                break;
            }

            TOGGLE(save_as_edit_mode, GuiTextBox(save_as_textbox, save_as_message, MESSAGE_SIZE, save_as_edit_mode));
            
            if (GuiButton(exit_button, "Exit"))
                break;
            
            if (GuiButton(run_button, "Run")) {
                should_run = true;
                break;
            }

            TOGGLE(sleep_edit_mode,      GuiValueBox(sleep_value_box, "", &alert->raw_time, 1, 1000, sleep_edit_mode));
            TOGGLE(unit_edit_mode,       GuiDropdownBox(time_unit_dropdown, units, &which_unit, unit_edit_mode));
            TOGGLE(text_edit_mode,       GuiDropdownBox(text_color_dropdown, colors, &text_color, text_edit_mode));
            TOGGLE(background_edit_mode, GuiDropdownBox(background_color_dropdown, colors, &background_color, background_edit_mode));
            alert->flash = GuiCheckBox(flash_checkbox, "", alert->flash);
            TOGGLE(message_edit_mode,    GuiTextBox(message_textbox, alert->message, MESSAGE_SIZE, message_edit_mode));

        }
        EndDrawing();
    }

    switch (background_color) {
    case 0: alert->background_color = WHITE;  break;
    case 1: alert->background_color = BLACK;  break;
    case 2: alert->background_color = RED;    break;
    case 3: alert->background_color = GREEN;  break;
    case 4: alert->background_color = BLUE;   break;
    case 5: alert->background_color = YELLOW; break;
    }

    switch (text_color) {
    case 0: alert->text_color = WHITE;  break;
    case 1: alert->text_color = BLACK;  break;
    case 2: alert->text_color = RED;    break;
    case 3: alert->text_color = GREEN;  break;
    case 4: alert->text_color = BLUE;   break;
    case 5: alert->text_color = YELLOW; break;
    }

    switch (which_unit) {
    case 0: alert->raw_time_unit = MINUTE; break;
    case 1: alert->raw_time_unit = HOUR;   break;
    case 2: alert->raw_time_unit = SECOND; break;
    }

    CloseWindow();

    //                 is this an alright way to check if a string is empty?
    if (should_save && save_as_message[0] != '\0')
        gen_desktop_file(alert, save_as_message);

    if (save_as_message[0] == '\0') {
        fprintf(stderr, "yeah im not saving nothing\n");
    }

    return should_run;
}

void pre_timer_window(struct Alert *alert) {
    InitWindow(200, 100, "About to start ...");
    int fps = GetMonitorRefreshRate(GetCurrentMonitor()) / 6;
    SetTargetFPS(fps);

    int frame = 0;
    
    while (!WindowShouldClose()) {
        BeginDrawing(); {
            if (frame >= fps)
                break;
            
            ClearBackground(alert->background_color);
            DrawTextCentered("Starting timer ...", 0, 0, 20, alert->text_color);

            ++frame;
        } EndDrawing();
    }
    
    CloseWindow();
}

void alert_window(struct Alert *alert) {
    InitWindow(0, 0, "ALERT!");

    int fps = GetMonitorRefreshRate(GetCurrentMonitor()) / 6;
    SetTargetFPS(fps);

    SetWindowState(FLAG_WINDOW_TOPMOST);
    
    int frame = 1;

    alert->wants_to_sleep = false;

    while (!WindowShouldClose()) {
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
        frame %= fps; // overflow bad
    }

    CloseWindow();
}

int main(int argc, char **argv) {
    SetTraceLogLevel(LOG_NONE);
    struct Alert alert = make_alert();

    bool should_run = true;
    if (argc > 1) {
        parse_args(argc, argv, &alert);
        pre_timer_window(&alert);
    } else {
        should_run = alert_window_editor(&alert);
    }

    if (!should_run)
        return 0;

    do {
        sleep(alert.raw_time * alert.raw_time_unit);
        alert_window(&alert);
    } while (alert.wants_to_sleep);

    return 0;
}
