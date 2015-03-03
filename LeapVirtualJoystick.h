// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

/**
 * 
 */
class OCULUSUIPOC_API LeapVirtualJoystick
{
public:
    /*
     Constructor takes Character as parameter since the actual virtual joystick is positioned relative to the character
     */
	LeapVirtualJoystick(ACharacter* Character);
	~LeapVirtualJoystick();
    
    /*
     This method calculates the forward and right movement speeds as well as the turn rate.  The intended use is that this method should be called first, and then the values retrieved through the 3 corresponding Getter methods.
     
    */
    void CalculateMovementFromLeapInput(FVector PalmLocation, FVector FingerLocation);
    
    float GetForwardMovement();
    float GetRightMovement();
    float GetTurnRate();


    float LeapMotionDonutHeight;
    float LeapMotionActivationDiskHeight;
    float LeapMotionDonutRadius;
    float LeapMotionDonutHoleRadius;
    float LeapMotionActivationDiskRadius;
    float LeapMotionTurnAngleThreshold;
    float LeapMotionTurnRateOffset;
    float LeapMotionMaxTurnRate;
    float LeapMotionTurnAngleToRateScale;
    float LeapMotionDeactivationBufferHeight;
    float LeapMotionSpeedScalingFactor;

    /*
      Location to draw activation disk
    */
    FVector LeapMotionActivationDiskOffset;
    
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
