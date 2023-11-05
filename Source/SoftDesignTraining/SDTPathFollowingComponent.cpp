// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTPathFollowingComponent.h"
#include "SoftDesignTraining.h"
#include "SDTUtils.h"
#include "SDTAIController.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "DrawDebugHelpers.h"

USDTPathFollowingComponent::USDTPathFollowingComponent(const FObjectInitializer &ObjectInitializer)
{
}

void USDTPathFollowingComponent::FollowPathSegment(float DeltaTime)
{
    const TArray<FNavPathPoint> &points = Path->GetPathPoints();
    const FNavPathPoint &segmentStart = points[MoveSegmentStartIndex];

    if (SDTUtils::HasJumpFlag(segmentStart))
    {
        // Update jump
        auto owner = GetOwner<ASDTAIController>();
        if (owner == nullptr)
        {
            return;
        }

        auto &segmentEnd = points[MoveSegmentStartIndex + 1];
        auto jumpDirection = segmentEnd.Location - segmentStart.Location;

        auto forwardVector = owner->GetCharacter()->GetActorForwardVector();

        auto angleBetweenJumpDirectionAndForwardVector = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(jumpDirection.GetSafeNormal(), forwardVector.GetSafeNormal())));

        if (FMath::Abs(angleBetweenJumpDirectionAndForwardVector) > 1.0f)
        {
            GEngine->AddOnScreenDebugMessage(INDEX_NONE, 0.0f, FColor::Blue, FString("Correcting angle"));
            owner->GetCharacter()->SetActorRotation(jumpDirection.Rotation());
        }
        else if (UseProvidedJumpCurve)
        {
            FVector newLocation;
            TimeOnCurve += DeltaTime * owner->JumpSpeed;
            if (TimeOnCurve >= MaxTimeCurve)
            {
                // Do not set the end position too far away from the segment end point in the case there is a gap in the DeltaTime.
                // The TickComponent method calls UpdatePathSegment first followed by FollowPathSegment. It is on the next tick after this one
                // that the transition to the next segment will occur.
                if (TimeOnCurve >= MaxTimeCurve + 0.2f)
                {
                    TimeOnCurve = MaxTimeCurve + 0.2f;
                }
                // This assumes that the starting and ending point are at the same Z coordinate.
                // The detection of the character back on the floor is not properly detected and it prevents to move to the next segment. Add a small shift in the Z coordinate to help
                // the detection of the floor.
                newLocation = segmentStart.Location + FVector(JumpVector2D * (TimeOnCurve - MinTimeCurve) / (MaxTimeCurve - MinTimeCurve), owner->GetCharacter()->GetSimpleCollisionHalfHeight() - 20.0f * ((TimeOnCurve - MaxTimeCurve) / 0.2f));
            }
            else
            {
                // For the position in X,Y, Do a linear scaling of the JumpVector2D vector. For the position in Z, use the jump curve with the time elapsed.
                auto heightOnCurve = owner->JumpCurve->GetFloatValue(TimeOnCurve) * owner->JumpApexHeight;
                newLocation = segmentStart.Location + FVector(JumpVector2D * (TimeOnCurve - MinTimeCurve) / (MaxTimeCurve - MinTimeCurve), heightOnCurve + owner->GetCharacter()->GetSimpleCollisionHalfHeight());
            }
            owner->GetCharacter()->SetActorLocation(newLocation);

            DrawDebugPoint(GetWorld(), newLocation, 3.0f, FColor::Magenta, false, 3.0f);
            DrawDebugDirectionalArrow(GetWorld(), segmentStart.Location, newLocation, 3.0f, FColor::Cyan, false, -1.0f, 0U, 2.0f);
        }
    }
    else
    {
        // update navigation along path
        Super::FollowPathSegment(DeltaTime);
    }
}

void USDTPathFollowingComponent::SetMoveSegment(int32 segmentStartIndex)
{
    Super::SetMoveSegment(segmentStartIndex);

    ASDTAIController *owner = GetOwner<ASDTAIController>();
    if (owner == nullptr)
    {
        return;
    }

    const TArray<FNavPathPoint> &points = Path->GetPathPoints();
    const FNavPathPoint &segmentStart = points[MoveSegmentStartIndex];
    const FNavPathPoint &segmentEnd = points[MoveSegmentStartIndex + 1];

    // For Debugging.
    if (SDTUtils::HasJumpFlag(segmentStart))
    {
        GEngine->AddOnScreenDebugMessage(INDEX_NONE, 2.0f, FColor::Blue, FString("Has Jump Flag"));
    }
    if (FNavMeshNodeFlags(segmentStart.Flags).IsNavLink())
    {
        GEngine->AddOnScreenDebugMessage(INDEX_NONE, 2.0f, FColor::Blue, FString("Is Nav Link"));
    }

    if (SDTUtils::HasJumpFlag(segmentStart) && FNavMeshNodeFlags(segmentStart.Flags).IsNavLink())
    {
        // Handle starting jump
        owner->AtJumpSegment = true;

        if (!UseProvidedJumpCurve)
        {
            FVector velocity;
            UGameplayStatics::SuggestProjectileVelocity_CustomArc(GetWorld(), velocity, segmentStart.Location, segmentEnd.Location);
            owner->GetCharacter()->LaunchCharacter(velocity, true, true);
        }
        else
        {
            // The X axis of the curve is the time elapsed. The Y axis of the curve is the height of the jump.
            // Retrieve the start and end time of the jump curve.
            owner->JumpCurve->GetTimeRange(MinTimeCurve, MaxTimeCurve);
            // Configure the time elapsed on the curve to start at the first time of the curve.
            TimeOnCurve = MinTimeCurve;
            // Since all the start and end points are at the same level, assume that the movement in the X,Y plane is linear during the jump.
            // To determine the position, we will just scale this vector according to the time that passed.
            JumpVector2D = FVector2D(segmentEnd.Location) - FVector2D(segmentStart.Location);
        }

        DrawDebugPoint(GetWorld(), segmentStart, 10.0f, FColor::Magenta, false, 3.0f);
    }
    else
    {
        // Handle normal segments
        owner->AtJumpSegment = false;

        DrawDebugPoint(GetWorld(), segmentStart, 10.0f, FColor::Green, false, 3.0f);
    }
}
