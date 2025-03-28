/* date = August 2nd 2023 9:34 pm */

#ifndef PLATFORM_H
#define PLATFORM_H


typedef struct 
{
    bool32 ended_down;
    b32 pressed;
}GameButtonState;

typedef struct 
{
    bool32 is_connected;
    bool32 is_analog;
    real32 stick_average_x;
    real32 stick_average_y;
    
    union
    {
        GameButtonState buttons[15];
        struct
        {
            GameButtonState move_up;
            GameButtonState move_down;
            GameButtonState move_left;
            GameButtonState move_right;
            
            GameButtonState action_up;
            GameButtonState action_down;
            GameButtonState action_left;
            GameButtonState action_right;
            
            GameButtonState left_shoulder;
            GameButtonState right_shoulder;
            
            GameButtonState back;
            GameButtonState start;
            
            GameButtonState shift;
            GameButtonState back_space;
            GameButtonState enter;
            
            //NOTE: add all buttons about this line
            
            GameButtonState terminator;
        };
    };
    
}GameControllerInput;


typedef struct{
    GameButtonState mouse_buttons[5];
    int32 mouse_x, mouse_y, mouse_z;
    r32 mouse_wheel;
    real32 elapsed_seconds;
    GameControllerInput controller;
}GameInput;




#endif //PLATFORM_H
