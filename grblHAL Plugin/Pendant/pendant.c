/*
   TODO:    HOMING AT STARTUP ???
            Strange behavior with o or O - characters ???

 */

#include "grbl/hal.h"
// #include "grbl/system.h"
// #include <stdio.h>
// #include <stdlib.h>
// #include <ctype.h>
#include <string.h>
#include "pendant.h"

static io_stream_t pendant_serial;
static on_report_options_ptr on_report_options;
static on_execute_realtime_ptr on_execute_realtime;
static on_state_change_ptr on_state_change;

#define INBUF_SIZE 50
#define OUTBUF_SIZE 100
#define STATEBUF_SIZE 16
#define MAX_CMD_SIZE 32
#define MAX_TOKEN_STRING MAX_CMD_SIZE

#define fps 10                               // position and state updates per second
#define slow_sending_fps fps / 2            // => 2 continous sends per second
#define loopTicks 1000 / fps
#define pendant_debug_in 0                 // debug inputs and outputs
#define pendant_debug_in_raw 0
#define pendant_debug_out 0


static bool state_has_changed = true;       // state and position change flags
static bool position_has_changed = true;


///////////////////////////////////////////////////////////////////////////////////////////

// parse json strings
static bool pendant_json_lookup(const char* cSource, const char* cToken, char cReply[]){
        char *ptr;
        ptr = strstr(cSource, cToken);
        if (!ptr) { return false; }
        ptr = strstr(ptr, ":");
        if (!ptr) { return false; }

        ptr++;
        int quotMarkDepth = 0;
        char lastQuotMark = '\0';

        for(; (*ptr == ' ' || *ptr == '"' || *ptr == '\'' || *ptr == '\t') && !quotMarkDepth; ptr++) {
                if(*ptr == '"' || *ptr == '\'') {
                        lastQuotMark = *ptr;
                        ++quotMarkDepth;
                }
        }
        for (int i=0, x=0; i<MAX_TOKEN_STRING+1; i++, ptr++) {
                switch(*ptr) {
                case '\0':
                case '}':
                case ',':
                        i = MAX_TOKEN_STRING+1;
                        break;
                case '\'':
                case '"':
                        if(lastQuotMark == '\0') {
                                cReply[x ] = *ptr;
                                x++;
                        }
                        else{
                                if(*ptr == lastQuotMark) {
                                        lastQuotMark = lastQuotMark == '"' ? '\'' : '"';
                                        --quotMarkDepth;
                                        if(quotMarkDepth) {
                                                cReply[x ] = *ptr;
                                                x++;
                                        }
                                        else{
                                                i = MAX_TOKEN_STRING+1;
                                        }
                                }
                                else{
                                        lastQuotMark = *ptr;
                                        ++quotMarkDepth;
                                }
                        }
                        break;
                default:
                        cReply[x ] = *ptr;
                        x++;
                }
                cReply[x] = '\0';
        }
        return true;
}


///////////////////////////////////////////////////////////////////////////////////

static void pendant_parse_and_send_cmd(const char buffer[INBUF_SIZE]) {
        char command[MAX_CMD_SIZE];
        // TEST: {"gcode":"G0X0Y0Z0A0"}
        if (pendant_json_lookup(buffer, "gcode", command)) {
                if (pendant_debug_in) {
                        hal.stream.write("GCODE:"); hal.stream.write(command); hal.stream.write(ASCII_EOL);
                }
                grbl.enqueue_gcode(command);
        }
        // TEST: {"cmd":"START"}
        else if (pendant_json_lookup(buffer, "cmd", command)) {
                if (pendant_debug_in) {hal.stream.write("CMD:"); hal.stream.write(command); hal.stream.write(ASCII_EOL);}

                if (strcmp(command, "START") == 0)
                {
                        grbl.enqueue_realtime_command(CMD_CYCLE_START);
                }
                else if (strcmp(command, "STOP") == 0)
                {
                        grbl.enqueue_realtime_command(CMD_JOG_CANCEL);
                        grbl.enqueue_realtime_command(CMD_STOP);
                }
                else if (strcmp(command, "UNLOCK") == 0)
                {
                        system_execute_line("$X");
                }
        }
        // TEST: {"msg":"test"}
        else if (pendant_json_lookup(buffer, "msg", command)) {
                hal.stream.write(command); {hal.stream.write(ASCII_EOL);}
        }
}


static void pendant_update (sys_state_t state)
{
        static uint32_t ms = 0;
        static int send_counter = 0;

        // check, if it is time to update. Always update on state change.
        if((hal.get_elapsed_ticks() >= ms) || state_has_changed) {
                ms = hal.get_elapsed_ticks() + loopTicks; //ms

                // if state has changed, update State-String
                static char string_state[STATEBUF_SIZE];
                if (state_has_changed) {
                        switch (state) {
                        case STATE_IDLE:
                                strcpy(string_state, "Idle"); break;
                        case STATE_CYCLE:
                                strcpy(string_state, "Run"); break;
                        case STATE_HOLD:
                                strcpy(string_state, "Hold"); break;
                        case STATE_JOG:
                                strcpy(string_state, "Jog"); break;
                        case STATE_HOMING:
                                strcpy(string_state, "Home"); break;
                        case STATE_ESTOP:
                                strcpy(string_state, "EndStop"); break;
                        case STATE_ALARM:
                                strcpy(string_state, "Alarm"); break;
                        case STATE_CHECK_MODE:
                                strcpy(string_state, "Check"); break;
                        case STATE_SAFETY_DOOR:
                                strcpy(string_state, "Door"); break;
                        case STATE_SLEEP:
                                strcpy(string_state, "Sleep"); break;
                        case STATE_TOOL_CHANGE:
                                strcpy(string_state, "Tool"); break;
                        default:
                                strcpy(string_state, "N/A"); break;
                        }
                }


                // get new position and compare to old position
                static int32_t int_pos[N_AXIS];
                // static int32_t int_old_pos[N_AXIS];
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
                /*
                   if (memcmp(int_pos, int_old_pos, sizeof(int_pos)) != 0) {
                        system_convert_array_steps_to_mpos(float_pos, int_pos);
                        for (int i = 0; i < N_AXIS; i++) {
                                wco[i] = gc_get_offset(i);
                                float_pos[i] -= wco[i];
                                int_old_pos[i] = int_pos[i];
                        }
                        position_has_changed = true;
                   }
                 */
                // prepare JSON tring for Sending
                static char wifi_out_buffer[OUTBUF_SIZE];
                if (position_has_changed || state_has_changed) {
                        if (N_AXIS == 3) { sprintf(wifi_out_buffer, "{\"state\":\"%s\",\"wx\":%.3f,\"wy\":%.3f,\"wz\":%.3f}"ASCII_EOL, string_state, float_pos[0], float_pos[1], float_pos[2]); }
                        else if (N_AXIS == 4) { sprintf(wifi_out_buffer, "{\"state\":\"%s\",\"wx\":%.3f,\"wy\":%.3f,\"wz\":%.3f,\"wa\":%.3f}"ASCII_EOL, string_state, float_pos[0], float_pos[1], float_pos[2], float_pos[3]); }
                        else if (N_AXIS == 5) { sprintf(wifi_out_buffer, "{\"state\":\"%s\",\"wx\":%.3f,\"wy\":%.3f,\"wz\":%.3f,\"wa\":%.3f,\"wb\":%.3f}"ASCII_EOL, string_state, float_pos[0], float_pos[1], float_pos[2], float_pos[3], float_pos[4]); }
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

//report if state has changed
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
