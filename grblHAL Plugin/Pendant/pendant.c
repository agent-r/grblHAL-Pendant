/*
   TODO:    Strange behavior with o or O - characters ???
            Strange behavior when STOP
            Strange behavior
 */

#include "grbl/hal.h"
// #include "grbl/system.h"
// #include <stdio.h>
// #include <stdlib.h>
// #include <ctype.h>
#include <string.h>
#include "pendant.h"
#include "networking/cJSON.h"

static io_stream_t pendant_serial;
static on_report_options_ptr on_report_options;
static on_execute_realtime_ptr on_execute_realtime;
static on_state_change_ptr on_state_change;

#define INBUF_SIZE 50
#define OUTBUF_SIZE 100
#define STATEBUF_SIZE 16

#define fps 10                               // position and state updates per second
#define slow_sending_fps fps                 // => (fps / 2) => 2 sends per second
#define loopTicks 1000 / fps

#define pendant_debug_in 1                   // debug inputs and outputs
#define pendant_debug_in_raw 0
#define pendant_debug_out 0

static bool state_has_changed = true;       // state and position change flags
static bool position_has_changed = true;

const char StateStrings[12][STATEBUF_SIZE] = {
        "Idle",
        "Run",
        "Hold",
        "Jog",
        "Home",
        "EndStop",
        "Alarm",
        "Check",
        "Door",
        "Sleep",
        "Tool",
        "N/A"
};


///////////////////////////////////////////////////////////////////////////////////////////

static void pendant_parse_and_send_cmd(const char * const cmd_buffer) {

        const cJSON *js_cmd = NULL;
        const cJSON *cmd_json = cJSON_Parse(cmd_buffer);

        const char SysExecuteCommand[257];  // is this neccessary?

        if (cJSON_HasObjectItem(cmd_json,"cmd")) {
                js_cmd = cJSON_GetObjectItemCaseSensitive(cmd_json,"cmd");
                if (cJSON_IsString(js_cmd) && (js_cmd->valuestring != NULL))
                {
                        const char * str_cmd = js_cmd->valuestring;
                        if (pendant_debug_in) {hal.stream.write("CMD:"); hal.stream.write(str_cmd); hal.stream.write(ASCII_EOL);}

                        if (strcmp(str_cmd, "START") == 0)
                        {
                                grbl.enqueue_realtime_command(CMD_CYCLE_START);
                        }
                        else if (strcmp(str_cmd, "STOP") == 0)
                        {
                                // grbl.enqueue_realtime_command(CMD_JOG_CANCEL);
                                grbl.enqueue_realtime_command(CMD_STOP);
                                // strcpy(SysExecuteCommand, "");
                                // system_execute_line(SysExecuteCommand);     // must be at least "LINE_BUFFER_SIZE" long ???
                        }
                        else if (strcmp(str_cmd, "HOME") == 0)
                        {
                                strcpy(SysExecuteCommand, "$H");
                                system_execute_line(SysExecuteCommand);     // must be at least "LINE_BUFFER_SIZE" long ???
                                // system_execute_line("$H");
                        }
                        else if (strcmp(str_cmd, "UNLOCK") == 0)
                        {
                                // system_execute_line("$X");
                                strcpy(SysExecuteCommand, "$X");
                                system_execute_line(SysExecuteCommand);     // must be at least "LINE_BUFFER_SIZE" long ???
                        }
                }
        }

        if (cJSON_HasObjectItem(cmd_json,"gcode")) {
                js_cmd = cJSON_GetObjectItemCaseSensitive(cmd_json,"gcode");

                if (cJSON_IsString(js_cmd) && (js_cmd->valuestring != NULL))
                {
                        const char * str_gcode = js_cmd->valuestring;
                        if (pendant_debug_in) {
                                hal.stream.write("GCODE:"); hal.stream.write(str_gcode); hal.stream.write(ASCII_EOL);
                        }
                        grbl.enqueue_gcode(str_gcode);
                }
        }

        if (cJSON_HasObjectItem(cmd_json,"msg")) {
                js_cmd = cJSON_GetObjectItemCaseSensitive(cmd_json,"msg");

                if (cJSON_IsString(js_cmd) && (js_cmd->valuestring != NULL))
                {
                        const char * str_msg = js_cmd->valuestring;
                        hal.stream.write(str_msg); {hal.stream.write(ASCII_EOL);}
                }
        }
}


static void pendant_update (sys_state_t state)
{
        static uint32_t ms = 0;
        static int send_counter = 0;
        static byte state_number;

        // check, if it is time to update. Always update on state change.
        if((hal.get_elapsed_ticks() >= ms) || state_has_changed) {
                ms = hal.get_elapsed_ticks() + loopTicks; //ms

                // if state has changed, update State-Number
                if (state_has_changed) {
                        switch (state) {
                        case STATE_IDLE:
                                state_number = 0; break;
                        case STATE_CYCLE:
                                state_number = 1; break;
                        case STATE_HOLD:
                                state_number = 2; break;
                        case STATE_JOG:
                                state_number = 3; break;
                        case STATE_HOMING:
                                state_number = 4; break;
                        case STATE_ESTOP:
                                state_number = 5; break;
                        case STATE_ALARM:
                                state_number = 6; break;
                        case STATE_CHECK_MODE:
                                state_number = 7; break;
                        case STATE_SAFETY_DOOR:
                                state_number = 8; break;
                        case STATE_SLEEP:
                                state_number = 9; break;
                        case STATE_TOOL_CHANGE:
                                state_number = 10; break;
                        default:
                                state_number = 11; break;
                        }
                }

                // get new position and compare to old position
                static int32_t int_pos[N_AXIS];
                static float float_pos[N_AXIS];
                static float float_pos_old[N_AXIS];
                static float wco[N_AXIS];

                memcpy(int_pos, sys.position, sizeof(sys.position));
                system_convert_array_steps_to_mpos(float_pos, int_pos);
                for (int i = 0; i < N_AXIS; i++) {
                        wco[i] = gc_get_offset(i);
                        float_pos[i] -= wco[i];
                }

                if (memcmp(float_pos, float_pos_old, sizeof(float_pos)) != 0) {
                        position_has_changed = true;
                        memcpy(float_pos_old, float_pos, sizeof(float_pos));
                }

                // prepare JSON String for Sending
                static char wifi_out_buffer[OUTBUF_SIZE];
                if (position_has_changed || state_has_changed) {
                        if (N_AXIS == 3) { sprintf(wifi_out_buffer, "{\"state\":\"%s\",\"wx\":%.3f,\"wy\":%.3f,\"wz\":%.3f}", StateStrings[state_number], float_pos[0], float_pos[1], float_pos[2]); }
                        else if (N_AXIS == 4) { sprintf(wifi_out_buffer, "{\"state\":\"%s\",\"wx\":%.3f,\"wy\":%.3f,\"wz\":%.3f,\"wa\":%.3f}", StateStrings[state_number], float_pos[0], float_pos[1], float_pos[2], float_pos[3]); }
                        // else if (N_AXIS == 5) { sprintf(wifi_out_buffer, "{\"state\":\"%s\",\"wx\":%.3f,\"wy\":%.3f,\"wz\":%.3f,\"wa\":%.3f,\"wb\":%.3f}"ASCII_EOL, string_state, float_pos[0], float_pos[1], float_pos[2], float_pos[3], float_pos[4]); }
                }

                // send JSON string only on state change or once per second
                if (state_has_changed || position_has_changed || (send_counter == 0)) {
                        pendant_serial.write(wifi_out_buffer);
                        if (pendant_debug_out) {hal.stream.write(wifi_out_buffer);}
                        position_has_changed = false;
                        state_has_changed = false;
                }

                // increase once-per-second counter or reset counter if second is full
                send_counter++;
                if (send_counter >= slow_sending_fps) { send_counter = 0; }


                /////////////   RECEIVE COMMANDS   //////////////

                static int i = 0;
                static char in_buffer[INBUF_SIZE];

                while(pendant_serial.get_rx_buffer_count() > 0)
                {
                        char in = pendant_serial.read();

                        if (pendant_debug_in_raw) {
                                char * str_in = "-";
                                str_in[0] = in;
                                hal.stream.write(str_in); hal.stream.write(ASCII_EOL);
                        }

                        if (in == '{') {
                                i = 0;
                                in_buffer[i] = in;
                        }
                        else if (in == '}') {
                                i++;
                                in_buffer[i] = in;
                                in_buffer[i+1] = '\0';
                                i = 0;
                                if (in_buffer[0] == '{') {
                                        pendant_parse_and_send_cmd(in_buffer);
                                }
                        }
                        else {
                                if (i < (sizeof(in_buffer)-3)) {
                                        i++;
                                        in_buffer[i] = in;
                                }
                        }
                }
        }
        on_execute_realtime(state);
}

// report if state has changed (some states show up for only very short time - needed for probing-alarms)
static void state_changed(sys_state_t state) {
        state_has_changed = true;
        if(on_state_change) { on_state_change(state); }
}

// Say hello, pendant!
static void report_options (bool newopt)
{
        on_report_options(newopt);
        if(!newopt) { hal.stream.write("[PLUGIN:PENDANT]" ASCII_EOL); }
}

// initialize pendant
bool pendant_init (const io_stream_t *stream)
{
        // initialize serial stream
        memcpy(&pendant_serial, serialInit(115200), sizeof(io_stream_t));

        // Add pendant_update function to grblHAL foreground process
        on_execute_realtime = grbl.on_execute_realtime;
        grbl.on_execute_realtime = pendant_update;

        // Add state change interrupt
        on_state_change = grbl.on_state_change;
        grbl.on_state_change = state_changed;

        // Add report
        on_report_options = grbl.on_report_options;
        grbl.on_report_options = report_options;

        return true;
}
