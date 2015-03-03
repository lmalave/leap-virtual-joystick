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

#pragma once

/**
 *
 */
class VirtualJoystick3D
{
public:
    /*
     Constructor takes Character as parameter since the actual virtual joystick is positioned relative to the character
     */
    VirtualJoystick3D(ACharacter* Character);
    ~VirtualJoystick3D();
    
    /*
     This method calculates the forward and right movement speeds as well as the turn rate.  The intended use is that this method should be called first, and then the values retrieved through the 3 corresponding Getter methods.
     
     */
    void CalculateMovementFromHandLocation(FVector PalmLocation, FVector FingerLocation);
    
    float GetForwardMovement();
    float GetRightMovement();
    float GetTurnRate();
    
    
    float ActivationDiskRadius;
    float MovementDiskHeight;
    float MovementDiskRadius;
    float MovementDiskDonutHoleRadius;
    float TurnAngleThreshold;
    float TurnRateOffset;
    float MaxTurnRate;
    float TurnAngleToRateScale;
    float DeactivationBufferHeight;
    float SpeedScalingFactor;
    
    /*
     Location to draw activation disk
     */
    FVector ActivationDiskLocation;
    
protected:
    
    /*
     The speed function is nonlinear so that the character can move fluidly either fast or slow.
     */
    float CalculateSpeed(float PositionOnMotionDonutAxis);
    
    float ForwardMovement;
    float RightMovement;
    float TurnRate;
    
    ACharacter* Character;
    bool IsActivated;
    FVector DiskLocation; // in Character space
};
