// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Navigation/PathFollowingComponent.h"
#include "SDTPathFollowingComponent.generated.h"

/**
*
*/
UCLASS(ClassGroup = AI, config = Game)
class SOFTDESIGNTRAINING_API USDTPathFollowingComponent : public UPathFollowingComponent
{
    GENERATED_UCLASS_BODY()

public:
    virtual void FollowPathSegment(float deltaTime) override;
    virtual void SetMoveSegment(int32 segmentStartIndex) override;

private:
    /// During a jump segment, the time elapsed since the beginning of the jump.
    float TimeOnCurve = 0.0f;
    /// The time at which the FCurveFloat starts.
    double MinTimeCurve = 0.0;
    /// The time at which the FCurveFloat ends.
    double MaxTimeCurve = 0.0;
    /// Indicates the position in 2D along the start and end segment during the jump.
    FVector2D JumpVector2D = FVector2D::ZeroVector;
};