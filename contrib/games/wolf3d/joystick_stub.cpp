/// JOYSTICK STUB FOR Wolfenstein 3D port to KolibriOS
/// Ported by maxcodehack and turbocat2001

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

/** @file SDL_joystick.h
 *  @note In order to use these functions, SDL_Init() must have been called
 *        with the SDL_INIT_JOYSTICK flag.  This causes SDL to scan the system
 *        for joysticks, and load appropriate drivers.
 */

/** The joystick structure used to identify an SDL joystick */
struct _SDL_Joystick;
typedef struct _SDL_Joystick SDL_Joystick;

/* Function prototypes */
/**
 * Count the number of joysticks attached to the system
 */
  int    SDL_NumJoysticks(void){};

/**
 * Get the implementation dependent name of a joystick.
 *
 * This can be called before any joysticks are opened.
 * If no name can be found, this function returns NULL.
 */
  const char *    SDL_JoystickName(int device_index){};

/**
 * Open a joystick for use.
 *
 * @param[in] device_index
 * The index passed as an argument refers to
 * the N'th joystick on the system.  This index is the value which will
 * identify this joystick in future joystick events.
 *
 * @return This function returns a joystick identifier, or NULL if an error occurred.
 */
  SDL_Joystick *    SDL_JoystickOpen(int device_index){};

/**
 * Returns 1 if the joystick has been opened, or 0 if it has not.
 */
  int    SDL_JoystickOpened(int device_index){};

/**
 * Get the device index of an opened joystick.
 */
  int    SDL_JoystickIndex(SDL_Joystick *joystick){};

/**
 * Get the number of general axis controls on a joystick
 */
  int    SDL_JoystickNumAxes(SDL_Joystick *joystick){};

/**
 * Get the number of trackballs on a joystick
 *
 * Joystick trackballs have only relative motion events associated
 * with them and their state cannot be polled.
 */
  int    SDL_JoystickNumBalls(SDL_Joystick *joystick){};

/**
 * Get the number of POV hats on a joystick
 */
  int    SDL_JoystickNumHats(SDL_Joystick *joystick){};

/**
 * Get the number of buttons on a joystick
 */
  int    SDL_JoystickNumButtons(SDL_Joystick *joystick){};

/**
 * Update the current state of the open joysticks.
 *
 * This is called automatically by the event loop if any joystick
 * events are enabled.
 */
  void    SDL_JoystickUpdate(void){};

/**
 * Enable/disable joystick event polling.
 *
 * If joystick events are disabled, you must call SDL_JoystickUpdate()
 * yourself and check the state of the joystick when you want joystick
 * information.
 *
 * @param[in] state The state can be one of SDL_QUERY, SDL_ENABLE or SDL_IGNORE.
 */
  int    SDL_JoystickEventState(int state){};

/**
 * Get the current state of an axis control on a joystick
 *
 * @param[in] axis The axis indices start at index 0.
 *
 * @return The state is a value ranging from -32768 to 32767.
 */
  int    SDL_JoystickGetAxis(SDL_Joystick *joystick, int axis){};

/**
 *  @name Hat Positions
 *  The return value of SDL_JoystickGetHat() is one of the following positions:
 */
/*@{*/
#define SDL_HAT_CENTERED	0x00
#define SDL_HAT_UP		0x01
#define SDL_HAT_RIGHT		0x02
#define SDL_HAT_DOWN		0x04
#define SDL_HAT_LEFT		0x08
#define SDL_HAT_RIGHTUP		(SDL_HAT_RIGHT|SDL_HAT_UP)
#define SDL_HAT_RIGHTDOWN	(SDL_HAT_RIGHT|SDL_HAT_DOWN)
#define SDL_HAT_LEFTUP		(SDL_HAT_LEFT|SDL_HAT_UP)
#define SDL_HAT_LEFTDOWN	(SDL_HAT_LEFT|SDL_HAT_DOWN)
/*@}*/

/** 
 *  Get the current state of a POV hat on a joystick
 *
 *  @param[in] hat The hat indices start at index 0.
 */
  int    SDL_JoystickGetHat(SDL_Joystick *joystick, int hat){};

/**
 * Get the ball axis change since the last poll
 *
 * @param[in] ball The ball indices start at index 0.
 *
 * @return This returns 0, or -1 if you passed it invalid parameters.
 */
  int    SDL_JoystickGetBall(SDL_Joystick *joystick, int ball, int *dx, int *dy){};

/**
 * Get the current state of a button on a joystick
 *
 * @param[in] button The button indices start at index 0.
 */
  int    SDL_JoystickGetButton(SDL_Joystick *joystick, int button){};

/**
 * Close a joystick previously opened with SDL_JoystickOpen()
 */
  void    SDL_JoystickClose(SDL_Joystick *joystick){};

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif
