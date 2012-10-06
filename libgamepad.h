#include <dispatch/dispatch.h>

typedef enum { 
	GP_UP,
	GP_RIGHT,
	GP_DOWN,
	GP_LEFT,
	GP_SPECIAL_UP,    /* Directional */
	GP_SPECIAL_UP_RIGHT,
	GP_SPECIAL_RIGHT, /* buttons in  */
	GP_SPECIAL_DOWN_RIGHT,
	GP_SPECIAL_DOWN,  /* analog      */
	GP_SPECIAL_DOWN_LEFT,
	GP_SPECIAL_LEFT,  /* mode        */
	GP_SPECIAL_UP_LEFT,
	GP_1,
	GP_2,
	GP_3,
	GP_4,
	GP_R1,
	GP_R2,
	GP_R3,
	GP_L1,
	GP_L2,
	GP_L3,
	GP_SELECT,
	GP_START,
	GP_LEFT_JOYSTICK,
	GP_RIGHT_JOYSTICK
} gp_event_type;

typedef enum {
	GP_PRESS = 0,
	GP_RELEASE,
	GP_REPEAT
} gp_state;

// x_axis and y_axis are zero if the event is
// not a joystick event.
typedef void(*gp_callback)(int controller, gp_event_type, gp_state, float x_axis, float y_axis);

const char *gp_event_name(gp_event_type t);
const char *gp_state_name(gp_state s);

int gp_init();
int gp_register(int controller, gp_event_type t, gp_callback callback);
int gp_run();
