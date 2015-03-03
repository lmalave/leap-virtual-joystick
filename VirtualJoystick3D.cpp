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

#include "OculusUIPOC.h"
#include "Engine.h"
#include "VirtualJoystick3D.h"

VirtualJoystick3D::VirtualJoystick3D(ACharacter* Character)
{
    this->Character = Character;
    
    ActivationDiskRadius = 20.0;
    MovementDiskHeight = 0.7;
    MovementDiskRadius = 10.0;
    MovementDiskDonutHoleRadius = 2.0;
    TurnAngleThreshold = 10.0;
    TurnRateOffset = 0.0;
    MaxTurnRate = 1.5;
    TurnAngleToRateScale = 6.0;
    DeactivationBufferHeight = 10.0;
    SpeedScalingFactor = 1.0;
    
    ForwardMovement = 0.f;
    RightMovement = 0.f;
    TurnRate = 0.f;
    IsActivated = false;
    
    ActivationDiskLocation = FVector(60.0, -10.0, 45.0);
}

VirtualJoystick3D::~VirtualJoystick3D()
{
}

float VirtualJoystick3D::GetForwardMovement() {
    return ForwardMovement;
}

float VirtualJoystick3D::GetRightMovement() {
    return RightMovement;
}

float VirtualJoystick3D::GetTurnRate() {
    return TurnRate;
}

void VirtualJoystick3D::CalculateMovementFromHandLocation(FVector PalmLocation, FVector FingerLocation) {
    
    // Next figure out if movement disk is activated
    if (FingerLocation.Z > ActivationDiskLocation.Z) {
        if (!IsActivated) {
            // for unactivating crossing the Z plane is fine, but for activating want to make sure we are within the disk.
            FVector MovementFingerCharacterLocationOnDisk = FingerLocation;
            MovementFingerCharacterLocationOnDisk.Z = ActivationDiskLocation.Z;
            if (FVector::Dist(MovementFingerCharacterLocationOnDisk, ActivationDiskLocation) < ActivationDiskRadius) {
                IsActivated = true;
                DiskLocation = FVector(PalmLocation.X, PalmLocation.Y, ActivationDiskLocation.Z);
            }
        }
    }
    else {
        if (IsActivated && FingerLocation.Z < (ActivationDiskLocation.Z - DeactivationBufferHeight)) { // have some small buffer for finger going below disk if it's already activated, to make it more robust
            IsActivated = false;
            TurnRate = 0.0;
            ForwardMovement = 0.0;
            RightMovement = 0.0;
        }
    }
    
    // Next draw Leap Motion Donut
    FVector CylinderStart;
    FVector CylinderEnd;
    FColor CylinderColor = FColor::Cyan;
    float CylinderRadius;
    if (IsActivated) {
        // transform back to world space so can draw
        CylinderStart = Character->GetTransform().TransformPosition(DiskLocation); // this is top of cylinder
        CylinderEnd = CylinderStart - (Character->GetActorUpVector() * MovementDiskHeight); // this is bottom of cylinder
        CylinderRadius = MovementDiskRadius;
        // draw inner "donut hole" cylinder only in activated case
        DrawDebugCylinder(Character->GetWorld(), CylinderStart, CylinderEnd, MovementDiskDonutHoleRadius, 12, CylinderColor);
    }
    else {
        // transform back to world space so can draw
        CylinderStart = Character->GetTransform().TransformPosition(ActivationDiskLocation); // this is top of cylinder
        CylinderEnd = CylinderStart - (Character->GetActorUpVector() * MovementDiskHeight); // this is bottom of cylinder
        CylinderRadius = ActivationDiskRadius;
    }
    DrawDebugCylinder(Character->GetWorld(), CylinderStart, CylinderEnd, CylinderRadius, 12, CylinderColor);
    DrawDebugLine(Character->GetWorld(),
                  CylinderEnd - (Character->GetActorRightVector() * CylinderRadius),
                  CylinderEnd + (Character->GetActorRightVector() * CylinderRadius),
                  CylinderColor);
    DrawDebugLine(Character->GetWorld(),
                  CylinderEnd - (Character->GetActorForwardVector() * CylinderRadius),
                  CylinderEnd + (Character->GetActorForwardVector() * CylinderRadius),
                  CylinderColor);
    
    // TODO: add proper input mapping so that it can be configured in DefaultInput.ini as a gamepad input would
    // Finally check if we need to add character movement based on movement hand/finger position
    if (IsActivated) {
        // project the movement hand palm location onto the plane of the top of the donut
        FVector MovementHandCharacterLocationOnCylinderTop = PalmLocation;
        MovementHandCharacterLocationOnCylinderTop.Z = DiskLocation.Z;
        FVector CursorVectorFromCylinderOrigin = MovementHandCharacterLocationOnCylinderTop - DiskLocation;
        FVector MovementCursorPositionCharacter = DiskLocation + CursorVectorFromCylinderOrigin.ClampMaxSize(CylinderRadius);
        // transform "cursor" position back to world space so can draw
        FVector MovementCursorPositionWorld = Character->GetTransform().TransformPosition(MovementCursorPositionCharacter);
        DrawDebugSphere(Character->GetWorld(), MovementCursorPositionWorld, 0.4, 12, FColor::Blue);
        // now calculate forward and right movement
        float ForwardPosition = MovementHandCharacterLocationOnCylinderTop.X - DiskLocation.X;
        float RightPosition = MovementHandCharacterLocationOnCylinderTop.Y - DiskLocation.Y;
        ForwardMovement = CalculateSpeed(ForwardPosition);
        RightMovement = CalculateSpeed(RightPosition);
        // now calculate angle from palm to middle finger and use that to set turn rate
        float MovementHandAngleZ = (FingerLocation.Z - PalmLocation.Z);
        float MovementHandAngleY = (FingerLocation.Y - PalmLocation.Y);
        float MovementHandAngle = (atan2(MovementHandAngleY, MovementHandAngleZ) * (180 / PI));
        TurnRate = 0.0;
        if (abs(MovementHandAngle) >= TurnAngleThreshold) { // only turn if angle is greater than some threshold
            float PercentageRotation =  (abs(MovementHandAngle) - TurnAngleThreshold) / 90.0;
            TurnRate = pow(PercentageRotation, 1.5) * TurnAngleToRateScale * copysign(1.0, MovementHandAngle);
            if (TurnRate > MaxTurnRate) {
                TurnRate = MaxTurnRate;
            }
        }
    }
    
}

// TODO: tweak non-linear speed function to make it easier to control movement, especially at slower speeds
float VirtualJoystick3D::CalculateSpeed(float PositionOnMotionDonutAxis) {
    float Speed = 0.0;
    if ((abs(PositionOnMotionDonutAxis) > MovementDiskDonutHoleRadius)) {  // only add forward movement if we are outside of the donut hole
        float PercentagePosition = (abs(PositionOnMotionDonutAxis) - MovementDiskDonutHoleRadius) / (MovementDiskRadius - MovementDiskDonutHoleRadius);
        float SpeedFunctionValue = pow(PercentagePosition, 2.0); // for now just square it to make it non-linear.  TODO: need to tweak
        Speed = SpeedFunctionValue * copysign(SpeedScalingFactor, PositionOnMotionDonutAxis); // apply scaling factor and restore the positive/negative direction sign
    }
    return Speed;
}

