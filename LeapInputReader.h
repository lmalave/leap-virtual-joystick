/*****************************
Copyright 2015 (c) Leonardo Malave. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are
permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice, this list of
      conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright notice, this list
      of conditions and the following disclaimer in the documentation and/or other materials
      provided with the distribution.

THIS SOFTWARE IS PROVIDED BY Leonardo Malave ''AS IS'' AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Rafael Muñoz Salinas OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those of the
authors and should not be interpreted as representing official policies, either expressed
or implied, of Leonardo Malave.
********************************/
#include "Leap.h"

#pragma once

/**
 * 
 */
class LeapInputReader
{
public:
    /* 
     Constructor takes two parameters:  a Leap::Controller and a Character
     -- the Leap::Controller parameter is so that the controller can be initialized outside of this class, since the Leap::Controller should only be initialized once in an application
     -- the Character parameter is so that the position of the Leap hands/fingers can be returned in the coordinate space of the Character.  This makes it easier to calculate the desired movement based on these character-space coordinates.
     */
    LeapInputReader(Leap::Controller* Controller,  ACharacter* Character);
	~LeapInputReader();
    
    /*
     As the method name implies, this method calculates the location coordinates in world space of the Leap hand and finger coordinates.  Note that for now this assumes a scaling factor of 0.1, since Leap coordinates are always in millimeters and the default Unreal world scale is 1 Unreal Unit = 1 centimeter.  
     Note that the intention is that this method should be called first, and then the values retrieved through the available Getter methods. 
     */
    void UpdateHandLocations();
    
    FVector GetLeftPalmLocation_WorldSpace();
    FVector GetLeftFingerLocation_WorldSpace();
    FVector GetRightPalmLocation_WorldSpace();
    FVector GetRightFingerLocation_WorldSpace();
    FVector GetLeftPalmLocation_CharacterSpace();
    FVector GetLeftFingerLocation_CharacterSpace();
    FVector GetRightPalmLocation_CharacterSpace();
    FVector GetRightFingerLocation_CharacterSpace();
    bool IsValidInputLastFrame();
    
    
    /*
     If true - draws minimalist hands just connecting the palm location to the fingertips
     Making optional in case someone is implementing a proper skeletal animation and doesn't want the hands drawn.
    */
    bool LeapDrawSimpleHands;
    
    /*
     Leap Unit is a millimiter and Unreal units are centimeters, so default is 0.1
     */
    float LeapToUnrealScalingFactor;

    /*
     Offset to account for position of Leap mount on Oculus headset
     NOTE: for simplicity this is still in Leap coordinates!  This is intended to first correct the Leap coordinates for the fact that the mount positions the Leap a few inches in front of the eyes.  
     */
    FVector LeapMountOffset;

    /*
     Offset to account for the fact the Leap is head mounted so the proper location to draw the hand to look natural will require some trial and error.
     */
    FVector LeapHandOffset;
    
    
protected:

    /*
     Translates the Leap coordinates to Unreal location coordinates. 
     */
    FVector LeapPositionToUnrealLocation(Leap::Vector LeapVector, FVector UnrealOffset);
    
    ACharacter* Character;
    Leap::Controller* Controller;

    bool ValidInputLastFrame;
    FVector LeftPalmLocation_WorldSpace;
    FVector LeftFingerLocation_WorldSpace;
    FVector RightPalmLocation_WorldSpace;
    FVector RightFingerLocation_WorldSpace;
    FVector LeftPalmLocation_CharacterSpace;
    FVector LeftFingerLocation_CharacterSpace;
    FVector RightPalmLocation_CharacterSpace;
    FVector RightFingerLocation_CharacterSpace;

};
