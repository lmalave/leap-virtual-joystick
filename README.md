# leap-virtual-joystick
This is a virtual joystick implemented with the Leap Motion C++ SDK for Unreal Engine 4

You can see a video of the virtual joystick implementation here: 

https://www.youtube.com/watch?v=ATkQGcukCgY


The idea is to leverage the Leap Controller hand detection create a control system similar to a game controller, using the following as inspiration:
Game controller analog sticks
The analogous touch-screen implementation where the finger position on a circle simulates an analog stick control
In 3D, the equivalent implementation is a disk floating in 3D space, where the movement of the hand within this disk controls forward/backward and left/right movement the same way the left analog stick would on a controller.   So the forward/backward or left/right position of the hand within the disk would translate directly to the movement direction and speed.   To include turning left/right as part of the controls, we can map the turn to the yaw angle of the hand. 

## Movement control Leap implementation:

* Movement is only activated if the middle finger crosses the plane of the disk
* If the middle finger drops back below the plane, the movement disk is deactivated and movement stops
* In Leap, the palm position is tracked and projected to the plane of the disk (palm position tracking is more stable than any of the finger positions) and this position is shown as a "cursor"
* The position of this "cursor" determines the speed and direction of movement
* If movement is activated and the palm position moves outside of the bounds of the disk, the cursor will be shown only up to the edge of the disk. This places an upper limit on speed. This also makes the input more tolerant and robust so the user can move very fluidly. 
* The angle of the palm with the middle finger is used to determine the turn rate
 
## Movement control design principles

### Visual Feedback!!: 

Show the user exactly how their actions are directly controlling the movement. Many Leap demos just have the user waving their hands in space without displaying any kind of visual control or coordinate system.  The intent might be to make the Leap seem like "magic" but at the expense of having an actually practical and usable control system. 

###Tolerance/robustness: 

Wherever possible should have tolerances and damping built in so user can more easily control movement

Examples include:
* When disk is activated, user can go outside bounds of disk
* There is a "buffer" so even if the middle finger drops slightly below the plane of the disk movement stays active
* There is a "donut hole" dead zone on the disk so it's easier for the user to stop movement.   There is a dead zone for the angle of rotation as well.
* A non-linear function is applied for both movement and turning, so that it's easier for the user to move both slowly or quickly as needed

Implementation challenges:
* Leap motion tracking is very limited:  all fingers must be extended and visible to the Leap
* A lot of tweaking and trial and error to get the controls just right: (positioning of disk, radius of disk, nonlinear function applied to movement)
