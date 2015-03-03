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
#include "LeapVirtualJoystick.h"

LeapVirtualJoystick::LeapVirtualJoystick(ACharacter* Character)
{
    this->Character = Character;

    LeapMotionDonutHeight = 0.7;
    LeapMotionActivationDiskHeight = 0.7;
    LeapMotionDonutRadius = 10.0;
    LeapMotionDonutHoleRadius = 2.0;
    LeapMotionActivationDiskRadius = 20.0;
    LeapMotionTurnAngleThreshold = 10.0;
    LeapMotionTurnRateOffset = 0.0;
    LeapMotionMaxTurnRate = 1.5;
    LeapMotionTurnAngleToRateScale = 6.0;
    LeapMotionDeactivationBufferHeight = 10.0;
    LeapMotionSpeedScalingFactor = 1.0;
    
    ForwardMovement = 0.f;
    RightMovement = 0.f;
    TurnRate = 0.f;
    IsActivated = false;

    LeapMotionActivationDiskOffset = FVector(60.0, -10.0, 45.0); // TODO: move to variable
}

LeapVirtualJoystick::~LeapVirtualJoystick()
{
}

float LeapVirtualJoystick::GetForwardMovement() {
    return ForwardMovement;
}

float LeapVirtualJoystick::GetRightMovement() {
    return RightMovement;
}

float LeapVirtualJoystick::GetTurnRate() {
    return TurnRate;
}

void LeapVirtualJoystick::CalculateMovementFromLeapInput(FVector PalmLocation, FVector FingerLocation) {

    // Next figure out if movement disk is activated
    if (FingerLocation.Z > LeapMotionActivationDiskOffset.Z) {
        if (!IsActivated) {
            // for unactivating crossing the Z plane is fine, but for activating want to make sure we are within the disk.
            FVector MovementFingerCharacterLocationOnDisk = FingerLocation;
            MovementFingerCharacterLocationOnDisk.Z = LeapMotionActivationDiskOffset.Z;
            if (FVector::Dist(MovementFingerCharacterLocationOnDisk, LeapMotionActivationDiskOffset) < LeapMotionActivationDiskRadius) {
                IsActivated = true;
                DiskLocation = FVector(PalmLocation.X, PalmLocation.Y, LeapMotionActivationDiskOffset.Z);
            }
        }
    }
    else {
        if (IsActivated && FingerLocation.Z < (LeapMotionActivationDiskOffset.Z - LeapMotionDeactivationBufferHeight)) { // have some small buffer for finger going below disk if it's already activated, to make it more robust
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
        CylinderEnd = CylinderStart - (Character->GetActorUpVector() * LeapMotionDonutHeight); // this is bottom of cylinder
        CylinderRadius = LeapMotionDonutRadius;
        // draw inner "donut hole" cylinder only in activated case
        DrawDebugCylinder(Character->GetWorld(), CylinderStart, CylinderEnd, LeapMotionDonutHoleRadius, 12, CylinderColor);
    }
    else {
        // transform back to world space so can draw
        CylinderStart = Character->GetTransform().TransformPosition(LeapMotionActivationDiskOffset); // this is top of cylinder
        CylinderEnd = CylinderStart - (Character->GetActorUpVector() * LeapMotionActivationDiskHeight); // this is bottom of cylinder
        CylinderRadius = LeapMotionActivationDiskRadius;
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
        if (abs(MovementHandAngle) >= LeapMotionTurnAngleThreshold) { // only turn if angle is greater than some threshold
            float PercentageRotation =  (abs(MovementHandAngle) - LeapMotionTurnAngleThreshold) / 90.0;
            TurnRate = pow(PercentageRotation, 1.5) * LeapMotionTurnAngleToRateScale * copysign(1.0, MovementHandAngle);
            if (TurnRate > LeapMotionMaxTurnRate) {
                TurnRate = LeapMotionMaxTurnRate;
            }
        }
    }
    
}

// TODO: tweak non-linear speed function to make it easier to control movement, especially at slower speeds
float LeapVirtualJoystick::CalculateSpeed(float PositionOnMotionDonutAxis) {
    float Speed = 0.0;
    if ((abs(PositionOnMotionDonutAxis) > LeapMotionDonutHoleRadius)) {  // only add forward movement if we are outside of the donut hole
        float PercentagePosition = (abs(PositionOnMotionDonutAxis) - LeapMotionDonutHoleRadius) / (LeapMotionDonutRadius - LeapMotionDonutHoleRadius);
        float SpeedFunctionValue = pow(PercentagePosition, 2.0); // for now just square it to make it non-linear.  TODO: need to tweak
        Speed = SpeedFunctionValue * copysign(LeapMotionSpeedScalingFactor, PositionOnMotionDonutAxis); // apply scaling factor and restore the positive/negative direction sign
    }
    return Speed;
}

