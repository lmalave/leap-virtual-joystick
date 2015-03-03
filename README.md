# leap-virtual-joystick

This is a virtual joystick implemented with the Leap Motion C++ SDK for Unreal Engine 4.  It was intended to be a very simple way to start using the Leap Motion as a practical and robust controller for virtual reality applications.    

Let's go straight to the video - you can see the virtual joystick in action here: 

https://www.youtube.com/watch?v=ATkQGcukCgY


The idea is to leverage the Leap Controller hand detection create a control system similar to a game controller, using the following as inspiration:

* Game controller analog sticks
* The corresponding touch-screen implementation where the finger position on a circle simulates an analog stick control

In 3D, the equivalent implementation is a disk floating in 3D space, where the movement of the hand within this disk controls forward/backward and left/right movement the same way the left analog stick would on a controller.   So the forward/backward or left/right position of the hand within the disk would translate directly to the movement direction and speed.   To include turning left/right as part of the controls, we can map the turn to the yaw angle of the hand. 

## How to integrate into Unreal Engine C++ project

* The Leap SDK is required, so please download and add to your project's lib paths

* The Leap::Controller class from the SDK should be instantiated in your application (for example in your character's constructor or BeginPlay() function) so that it can be passed to the LeapInputReader class described below

* Once the Leap::Controller class is available, the virtual joystick can be implemented by just adding a few lines of code to the character's BeginPlay() and Tick() functions as in the below example:

add to BeginPlay(): 

	    controller = new Leap::Controller();
	    controller->setPolicy(Leap::Controller::POLICY_OPTIMIZE_HMD);
	    LeapInput = new LeapInputReader(controller, this);
	    VirtualJoystick = new VirtualJoystick3D(this);

add to Tick(): 

        LeapInput->UpdateHandLocations();
        FVector MovementHandPalmLocation_CharacterSpace = LeapInput->GetLeftPalmLocation_CharacterSpace();
        FVector MovementHandFingerLocation_CharacterSpace = LeapInput->GetLeftFingerLocation_CharacterSpace();
        VirtualJoystick->CalculateMovementFromHandLocation(MovementHandPalmLocation_CharacterSpace, MovementHandFingerLocation_CharacterSpace);
        MoveForward(VirtualJoystick->GetForwardMovement());
        MoveRight(VirtualJoystick->GetRightMovement());
        TurnAtRate(VirtualJoystick->GetTurnRate());

That's it!  The virtual joystick implementation is intended to be simple and minimalist and yet very effective as an actually usable Leap-based control system.

The actual virtual joystick implementation only involves 2 classes (LeapInputReader and VirtualJoystick3D), which are described below.  

## C++ Class Design and Implementation

Although the provided example uses the Leap Motion controller as input for the 3D virtual joystick, the idea was to decouple the implementation so in the future other hand tracking technologies could be substituted for the Leap Motion.  The implementation was thus divided into two classes:

* LeapInputReader
* VirtualJoystick3D

### LeapInputReader class
 
The job of the LeapInputReader class is simply to read the raw palm and finger coordinates provided by the Leap Controller, and then translate those to Unreal position vectors (both in world space an also relative to the character).   The LeapInputReader will also draw the most minimal representation of the hand (just lines connecting the palm location to the fingertip locations), although this is optional so that another representation could be used instead (for example a proper skeletal mesh animation). 

NOTE: for now the LeapInputReader only returns the finger positions of the middle fingers since through trial and error those were the most stable and useable. As the Leap hand tracking improves this can be changed. 

The intended use is that the UpdateHandLocations() function would first be called (for example from the character's Tick() function), and the the getter methods would be used to retrieve these updated palm and finger locations.

### VirtualJoystick3D class
 
The job of the VirtualJoystick3D class is to translate a provided palm and finger position to a forward vector magnitude, a right vector magnitude, and a turn rate. The The way these values are calculated are by looking at where the palm and finger positions are relative to a "movement disk" that represents the virtual joystick.  Please see video above for a clear visualization.   An explanation of the formulas used to calculate movement is also provided below.  

The intended use is that the CalculateMovementFromHandLocation() function would first be called (for example from a character's Tick() function), and then the 3 getter methods would be used to get the forward, right, and turn magnitudes.   



## Explanation of 3D Virtual Joystick Mechanism

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

## Roadmap

* Integrate with Unreal Engine input system so can be configured through DefaultInput.ini, etc.
* Implement as Unreal Plugin and/or Components so it's easier to integrate into existing projects. 

