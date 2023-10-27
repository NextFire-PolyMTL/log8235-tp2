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
        TimeOnCurve += DeltaTime / owner->JumpSpeed;
        auto heightOnCurve = owner->JumpCurve->GetFloatValue(TimeOnCurve) * owner->JumpApexHeight;
        auto newLocation = segmentStart.Location + FVector(JumpVector2D * (TimeOnCurve - MinTimeCurve) / (MaxTimeCurve - MinTimeCurve), heightOnCurve);
        owner->GetCharacter()->SetActorLocation(newLocation + owner->GetCharacter()->GetActorLocation() - owner->GetCharacter()->GetCharacterMovement()->GetActorFeetLocation());

        DrawDebugDirectionalArrow(GetWorld(), segmentStart.Location, newLocation, 3.0f, FColor::Cyan, false, -1.0f, 0U, 2.0f);
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
        owner->JumpCurve->GetTimeRange(MinTimeCurve, MaxTimeCurve);
        TimeOnCurve = MinTimeCurve;
        JumpVector2D = FVector2D(segmentEnd.Location) - FVector2D(segmentStart.Location);

        GEngine->AddOnScreenDebugMessage(INDEX_NONE, 2.0f, FColor::Magenta, FString("Is Jumping true"));
    }
    else
    {
        // Handle normal segments
        owner->AtJumpSegment = false;

        GEngine->AddOnScreenDebugMessage(INDEX_NONE, 2.0f, FColor::Purple, FString("No Jump Flag"));
    }
}
