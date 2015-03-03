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
#include "IHeadMountedDisplay.h"
#include "LeapInputReader.h"

LeapInputReader::LeapInputReader(Leap::Controller* Controller, ACharacter* Character)
{
    this->Controller = Controller;
    this->Character = Character;
    LeapDrawSimpleHands = true;
    LeapToUnrealScalingFactor = 0.1;  // Leap Unit is a millimiter and Unreal units are centimeters
    // NOTE: Mount offset is still in Leap coordinates, not Unreal units!!
    LeapMountOffset = FVector(150.f, 0.f, -20.f);
    LeapHandOffset = FVector(10.0, 0.0, 45.0); // note: x=forward, y=right, z=up
}

LeapInputReader::~LeapInputReader()
{
}

FVector LeapInputReader::GetLeftPalmLocation_WorldSpace() {
    return LeftPalmLocation_WorldSpace;
}

FVector LeapInputReader::GetLeftFingerLocation_WorldSpace() {
    return LeftFingerLocation_WorldSpace;
}

FVector LeapInputReader::GetRightPalmLocation_WorldSpace() {
    return RightPalmLocation_WorldSpace;
}

FVector LeapInputReader::GetRightFingerLocation_WorldSpace() {
    return RightFingerLocation_WorldSpace;
}

FVector LeapInputReader::GetLeftPalmLocation_CharacterSpace() {
    return LeftPalmLocation_CharacterSpace;
}

FVector LeapInputReader::GetLeftFingerLocation_CharacterSpace() {
    return LeftFingerLocation_CharacterSpace;
}

FVector LeapInputReader::GetRightPalmLocation_CharacterSpace() {
    return RightPalmLocation_CharacterSpace;
}

FVector LeapInputReader::GetRightFingerLocation_CharacterSpace() {
    return RightFingerLocation_CharacterSpace;
}

void LeapInputReader::CalculateUnrealHandLocationsFromLeapCoordinates()
{
    // First just get hand and finger positions and draw the hands
    Leap::Frame Frame = Controller->frame();
    Leap::HandList Hands = Frame.hands();
    Leap::PointableList Pointables = Frame.pointables();
    Leap::GestureList gestures = Frame.gestures();
    FColor handColor = FColor::Magenta;
    FColor fingertipColor = handColor;
    
    UWorld* World = Character->GetWorld();
    
    for (Leap::HandList::const_iterator HandsIter = Hands.begin(); HandsIter != Hands.end(); HandsIter++) {
        Leap::Hand Hand = (Leap::Hand)(*HandsIter);
        Leap::Vector palmPosition = Hand.palmPosition();
        FVector palmLocation = LeapPositionToUnrealLocation(palmPosition, LeapHandOffset);
        if (Hand.isLeft()) {
            LeftPalmLocation_WorldSpace = palmLocation;
        }
        else {
            RightPalmLocation_WorldSpace = palmLocation;
        }
        if (LeapDrawSimpleHands) {
            DrawDebugSphere(World, palmLocation, 1.0, 12, handColor);
        }
        Leap::FingerList Fingers = Hand.fingers();
        for (Leap::FingerList::const_iterator FingersIter = Fingers.begin(); FingersIter != Fingers.end(); FingersIter++) {
            Leap::Finger finger = (Leap::Finger)(*FingersIter);
            Leap::Vector tipPosition = finger.tipPosition();
            FString fingerPosition(tipPosition.toString().c_str());
            FVector fingerLocation = LeapPositionToUnrealLocation(tipPosition, LeapHandOffset);
            if (finger.type() == Leap::Finger::TYPE_MIDDLE) { // only middle finger used for leap input
                fingertipColor = FColor::Red;
               if (Hand.isLeft()) {
                    LeftFingerLocation_WorldSpace = fingerLocation;
                }
                else {
                    RightFingerLocation_WorldSpace = fingerLocation;
                }
            }
            else {
                fingertipColor = handColor;
            }
            if (LeapDrawSimpleHands) {
                DrawDebugSphere(World, fingerLocation, 0.5, 12, fingertipColor);
                DrawDebugLine(World, palmLocation, fingerLocation, handColor);
            }
        }
    }
    // end result: get hand/finger locations in Character space
    FTransform CharacterTransform = Character->GetTransform();
    LeftPalmLocation_CharacterSpace = CharacterTransform.InverseTransformPosition(LeftPalmLocation_WorldSpace);
    LeftFingerLocation_CharacterSpace = CharacterTransform.InverseTransformPosition(LeftFingerLocation_WorldSpace);
    RightPalmLocation_CharacterSpace = CharacterTransform.InverseTransformPosition(RightPalmLocation_WorldSpace);
    RightFingerLocation_CharacterSpace = CharacterTransform.InverseTransformPosition(RightFingerLocation_WorldSpace);
    
}

// NOTE: because of the different coordinate systems for Leap forward = Y whereas for a character Forward = X
FVector LeapInputReader::LeapPositionToUnrealLocation(Leap::Vector LeapVector, FVector UnrealOffset) {
    
    // Adjust for mount offset and also current HMD orientation
    // NOTE: reverse X and Y because of different coordinate systems
    FVector LeapVectorCorrected = FVector(LeapVector.y, LeapVector.x, LeapVector.z) + LeapMountOffset;
    if (GEngine->HMDDevice.IsValid() && GEngine->HMDDevice->IsHeadTrackingAllowed())
    {
        FQuat HMDOrientation;
        FVector HMDPosition;
        
        GEngine->HMDDevice->GetCurrentOrientationAndPosition(HMDOrientation, HMDPosition);
        
        FRotator HMDRotator = HMDOrientation.Rotator();
        LeapVectorCorrected = HMDRotator.UnrotateVector(LeapVectorCorrected);  // TODO: adjust for leap rotation not being the same as HMD rotation
    }
    FVector ScaledLeapVector = LeapVectorCorrected * LeapToUnrealScalingFactor;
    FVector UnrealLocation = Character->GetActorLocation() - (Character->GetActorRightVector() * ScaledLeapVector.Y) + (Character->GetActorForwardVector() * (ScaledLeapVector.X + UnrealOffset.X)) - (Character->GetActorUpVector() * (ScaledLeapVector.Z - UnrealOffset.Z));
    return UnrealLocation;
}