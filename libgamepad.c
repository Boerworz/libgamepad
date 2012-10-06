#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>

#include <ApplicationServices/ApplicationServices.h>
#include "hidapi.h"

#include "libgamepad.h"

void gp_invoke_callback__(int controller, gp_event_type t, gp_state state, float x, float y);
int gp_read__();
int callback_index__(int controller, gp_event_type t);

static hid_device *handle__;

// Callbacks are stored in this array
// index 0-25 are controller 1 callbacks
// index 26-51 are controller 2 callbacks
static gp_callback callbacks__[(GP_RIGHT_JOYSTICK + 1) * 2];


const char *gp_event_name(gp_event_type t) {
	switch(t) {
		case GP_UP:
			return "GP_UP";
			break;
		case GP_RIGHT:
			return "GP_RIGHT";
			break;
		case GP_DOWN:
			return "GP_DOWN";
			break;
		case GP_LEFT:
			return "GP_LEFT";
			break;
		case GP_SPECIAL_UP:
			return "GP_SPECIAL_UP";
			break;
		case GP_SPECIAL_UP_RIGHT:
			return "GP_SPECIAL_UP_RIGHT";
			break;
		case GP_SPECIAL_RIGHT:
			return "GP_SPECIAL_RIGHT";
			break;
		case GP_SPECIAL_DOWN_RIGHT:
			return "GP_SPECIAL_DOWN_RIGHT";
			break;
		case GP_SPECIAL_DOWN:
			return "GP_SPECIAL_DOWN";
			break;
		case GP_SPECIAL_DOWN_LEFT:
			return "GP_SPECIAL_DOWN_LEFT";
			break;
		case GP_SPECIAL_LEFT:
			return "GP_SPECIAL_LEFT";
			break;
		case GP_SPECIAL_UP_LEFT:
			return "GP_SPECIAL_UP_LEFT";
			break;
		case GP_1:
			return "GP_1";
			break;
		case GP_2:
			return "GP_2";
			break;
		case GP_3:
			return "GP_3";
			break;
		case GP_4:
			return "GP_4";
			break;
		case GP_R1:
			return "GP_R1";
			break;
		case GP_R2:
			return "GP_R2";
			break;
		case GP_R3:
			return "GP_R3";
			break;
		case GP_L1:
			return "GP_L1";
			break;
		case GP_L2:
			return "GP_L2";
			break;
		case GP_L3:
			return "GP_L3";
			break;
		case GP_SELECT:
			return "GP_SELECT";
			break;
		case GP_START:
			return "GP_START";
			break;
		case GP_LEFT_JOYSTICK:
			return "GP_LEFT_JOYSTICK";
			break;
		case GP_RIGHT_JOYSTICK:
			return "GP_RIGHT_JOYSTICK";
			break;
		default:
			return "";
	}
	
}

const char *gp_state_name(gp_state s) {
	switch(s) {
		case GP_PRESS:
			return "GP_PRESS";
			break;
		case GP_RELEASE:
			return "GP_RELEASE";
			break;
		case GP_REPEAT:
			return "GP_REPEAT";
			break;
		default:
			return "";
	}
}

int callback_index__(int controller, gp_event_type t) {
	return 26 * (controller - 1) + t;
}

int gp_init() {
	if(hid_init() != 0) {
		fprintf(stderr, "Unable to initialize hidapi. Exiting.\n");
		return -1;
	}
	// Try to open the device
	handle__ = hid_open(0x0810, 0x0001, NULL);
	if(!handle__) {
		return -2;
	}

	return 0;
}

int gp_register(int controller, gp_event_type event_type, gp_callback callback) {
	if(!callback) return -1;
	if(event_type > GP_RIGHT_JOYSTICK) return -2;
	if(controller != 1 && controller != 2) return -3;

	int index = callback_index__(controller, event_type);
	callbacks__[index] = callback;
	return 0;
}

void gp_check_numbered__(unsigned char *buf, unsigned char *prev_buf, int controller) {
	int buttons[4] = {
		0x10, 0x20, 0x40, 0x80
	};

	gp_event_type event_types[4] = {
		GP_1, GP_2, GP_3, GP_4
	};

	int val = buf[5];
	int prev_val = prev_buf[5];

	for(int i = 0; i < 4; i++) {
		gp_state state = -1;
		int v = buttons[i];
		if(v & prev_val && v & val)
			state = GP_REPEAT;
		else if(v & val)
			state = GP_PRESS;
		else if(v & prev_val)
			state = GP_RELEASE;

		if(state != -1)
			gp_invoke_callback__(controller, event_types[i], state, 0, 0);
	}
}

void gp_check_special_directional__(unsigned char *buf, unsigned char *prev_buf, int controller) {
	// Directional buttons in analog mode
	int val = buf[5];
	int prev_val = prev_buf[5];

	int valid_val = val >= GP_SPECIAL_UP - 4 && val <= GP_SPECIAL_UP_LEFT - 4;
	int valid_prev = prev_val >= GP_SPECIAL_UP - 4 && prev_val <= GP_SPECIAL_UP_LEFT - 4;
	if(valid_val || valid_prev) {
		for(int v = GP_SPECIAL_UP; v <= GP_SPECIAL_UP_LEFT; v++) {
			int translated_val = v - 4;
			gp_state state = -1;
			if(translated_val == prev_val && translated_val == val)
				state = GP_REPEAT;
			else if(val == translated_val)
				state = GP_PRESS;
			else if(prev_val == translated_val)
				state = GP_RELEASE;

			if(state != -1)
				gp_invoke_callback__(controller, v, state, 0, 0);
		}
	}
}

void gp_check_special__(unsigned char *buf, unsigned char *prev_buf, int controller) {
	int val = buf[6];
	int prev_val = prev_buf[6];

	if(!val && !prev_val) return;

	int buttons[8] = {
		0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80
	};

	gp_event_type event_types[8] = {
		GP_L2, GP_R2, GP_L1, GP_R1, GP_SELECT, GP_START, GP_L3, GP_R3
	};

	for(int i = 0; i < 8; i++) {
		gp_state state = -1;
		int v = buttons[i];
		if(v & prev_val && v & val)
			state = GP_REPEAT;
		else if(v & val)
			state = GP_PRESS;
		else if(v & prev_val)
			state = GP_RELEASE;

		if(state != -1)
			gp_invoke_callback__(controller, event_types[i], state, 0, 0);
	}
}

void gp_check_left_joystick__(unsigned char *buf, unsigned char *prev_buf, int controller) {
	// The ugly assignments are due to the fact that
	// when in analog mode, the center of the joystick
	// is 128, but when not in analog mode it's 127...
	float x_val = (buf[3] == 127 ? 0 : (buf[3] - 128.0)/128.0);
	float y_val = (buf[4] == 127 ? 0 : (buf[4] - 128.0)/128.0);
	float prev_x_val = (prev_buf[3] == 127 ? 0 : (prev_buf[3] - 128.0)/128.0);
	float prev_y_val = (prev_buf[4] == 127 ? 0 : (prev_buf[4] - 128.0)/128.0);

	if(!x_val && !y_val && !prev_x_val && !prev_y_val)
		return;

	//if(controller == 2)
	//	printf("%f\t%f\t%f\t%f\n", x_val, y_val, prev_x_val, prev_y_val);

	if((!x_val && !y_val) && (prev_x_val || prev_y_val)) {
		// Release
		gp_invoke_callback__(controller, GP_LEFT_JOYSTICK, GP_RELEASE, x_val, y_val);
		return;
	}

	if((x_val || y_val) && (!prev_x_val && !prev_y_val)) {
		// Press
		gp_invoke_callback__(controller, GP_LEFT_JOYSTICK, GP_PRESS, x_val, y_val);
		return;
	}

	if((x_val || y_val) && (prev_x_val || prev_y_val)) {
		// Repeated
		gp_invoke_callback__(controller, GP_LEFT_JOYSTICK, GP_REPEAT, x_val, y_val);
		return;
	}
}

void gp_check_right_joystick__(unsigned char *buf, unsigned char *prev_buf, int controller) {
	// The ugly assignments are due to the fact that
	// when in analog mode, the center of the joystick
	// is 128, but when not in analog mode it's 127...
	float x_val = (buf[2] == 127 ? 0 : (buf[2] - 128.0)/128.0);
	float y_val = (buf[1] == 127 ? 0 : (buf[1] - 128.0)/128.0);
	float prev_x_val = (prev_buf[2] == 127 ? 0 : (prev_buf[2] - 128.0)/128.0);
	float prev_y_val = (prev_buf[1] == 127 ? 0 : (prev_buf[1] - 128.0)/128.0);

	if(!x_val && !y_val && !prev_x_val && !prev_y_val)
		return;

	//if(controller == 2)
	//	printf("%f\t%f\t%f\t%f\n", x_val, y_val, prev_x_val, prev_y_val);

	if((!x_val && !y_val) && (prev_x_val || prev_y_val)) {
		// Release
		gp_invoke_callback__(controller, GP_RIGHT_JOYSTICK, GP_RELEASE, x_val, y_val);
		return;
	}

	if((x_val || y_val) && (!prev_x_val && !prev_y_val)) {
		// Press
		gp_invoke_callback__(controller, GP_RIGHT_JOYSTICK, GP_PRESS, x_val, y_val);
		return;
	}

	if((x_val || y_val) && (prev_x_val || prev_y_val)) {
		// Repeated
		gp_invoke_callback__(controller, GP_RIGHT_JOYSTICK, GP_REPEAT, x_val, y_val);
		return;
	}
}

void gp_invoke_callback__(int controller, gp_event_type t, gp_state state, float x, float y) {
	dispatch_async(dispatch_get_main_queue(), ^{
		int index = callback_index__(controller, t);
		gp_callback callback = callbacks__[index];
		if(callback) {
			callback(controller, t, state, x, y);
		}
	});
}

int gp_run() {
	dispatch_queue_t input_queue = dispatch_queue_create("input_queue", NULL);
	dispatch_async(input_queue, ^{
		gp_read__();				
	});
	return 0;
}

static unsigned char prev_ctrl1_buf[65] = {0};
static unsigned char prev_ctrl2_buf[65] = {0};

int gp_read__() {
	dispatch_time_t time = dispatch_time(DISPATCH_TIME_NOW, 1.0/160.0 * NSEC_PER_SEC);
	dispatch_after(time, dispatch_get_current_queue(), ^{ gp_read__(); });

	unsigned char temp_buf[65];
	unsigned char ctrl1_buf[65];
	unsigned char ctrl2_buf[65];
	int res = 1;

	res = hid_read(handle__, temp_buf, 65);
	if (res < 0) {
		printf("Unable to read()\n");
		return res;
	}

	unsigned char *buf = temp_buf[0] == 1 ? ctrl1_buf : ctrl2_buf;
	unsigned char *prev_buf = temp_buf[0] == 1 ? prev_ctrl1_buf : prev_ctrl2_buf;
	memcpy(buf, temp_buf, res);

	gp_check_numbered__(buf, prev_buf, buf[0]);
	gp_check_special_directional__(buf, prev_buf, buf[0]);
	gp_check_special__(buf, prev_buf, buf[0]);
	gp_check_left_joystick__(buf, prev_buf, buf[0]);
	gp_check_right_joystick__(buf, prev_buf, buf[0]);
			
#ifdef GP_DEBUG
	printf("Controller %d\n", buf[0]);
	for (int i = 1; i < res; i++) {
		printf("buf[%d]:\t%d\n", i, buf[i]);
	}
	printf("\n");
#endif

	memcpy(prev_buf, temp_buf, res);
	
	return res;
}
