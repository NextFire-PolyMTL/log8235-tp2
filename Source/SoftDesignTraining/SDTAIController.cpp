// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTAIController.h"
#include "SoftDesignTraining.h"
#include "SDTCollectible.h"
#include "SDTFleeLocation.h"
#include "SDTPathFollowingComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
// #include "UnrealMathUtility.h"
#include "SDTUtils.h"
#include "EngineUtils.h"
#include "NavigationSystem.h"
#include "Algo/MaxElement.h"
#include "SoftDesignTrainingMainCharacter.h"

ASDTAIController::ASDTAIController(const FObjectInitializer &ObjectInitializer)
    : Super(ObjectInitializer.SetDefaultSubobjectClass<USDTPathFollowingComponent>(TEXT("PathFollowingComponent")))
{
}

void ASDTAIController::GoToBestTarget(float deltaTime)
{
    // Move to target depending on current behavior
    ShowNavigationPath();
    OnMoveToTarget();

    if (Cast<const INavAgentInterface>(TargetActor) != nullptr && canSeePlayer) //We update the path only when we see the player
    {
        MoveToActor(TargetActor,-1.0F,false);// Directly update the path when the actor move.
    }
    else if (Cast<const INavAgentInterface>(TargetActor) != nullptr && PlayerBehaviorChoice == PlayerBehavior::CHASE)
    {
        MoveToLocation(lastPlayerPosition);
        PlayerBehaviorChoice = PlayerBehavior::NO_PLAYER;
    }
    else if (TargetActor != nullptr)
    {
        MoveToLocation(TargetActor->GetActorLocation());
    }
    
}

void ASDTAIController::OnMoveToTarget()
{
    m_ReachedTarget = false;
}

void ASDTAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult &Result)
{
    Super::OnMoveCompleted(RequestID, Result);
    if (PlayerBehaviorChoice == PlayerBehavior::CHASE) {
        canSeePlayer = false;
        //We cannot set followPlayer to false now because otherwise the agent will go to the starting point of the player if he eliminate him
    }
    m_ReachedTarget = true;
}

void ASDTAIController::ShowNavigationPath()
{
    auto currentPath = GetPathFollowingComponent()->GetPath();
    // Show current navigation path DrawDebugLine and DrawDebugSphere
    if(currentPath == nullptr)
    {
        return;
    }

    auto points = currentPath->GetPathPoints();
       
    for (size_t i = 0; i < points.Num() - 1; i++)
    {
        DrawDebugLine(GetWorld(), points[i], points[i + 1], FColor::Green, false, 0.1f, 0, 5.f);
    }
}

void ASDTAIController::ChooseBehavior(float deltaTime)
{
    UpdatePlayerInteraction(deltaTime);
}

void ASDTAIController::UpdatePlayerInteraction(float deltaTime)
{
    // finish jump before updating AI state
    if (AtJumpSegment)
        return;

    APawn *selfPawn = GetPawn();
    if (!selfPawn)
        return;

    ACharacter *playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!playerCharacter)
        return;

    FVector detectionStartLocation = selfPawn->GetActorLocation() + selfPawn->GetActorForwardVector() * m_DetectionCapsuleForwardStartingOffset;
    FVector detectionEndLocation = detectionStartLocation + selfPawn->GetActorForwardVector() * m_DetectionCapsuleHalfLength * 2;

    TArray<TEnumAsByte<EObjectTypeQuery>> detectionTraceObjectTypes;
    detectionTraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(COLLISION_COLLECTIBLE));
    detectionTraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(COLLISION_PLAYER));
    TArray<FHitResult> allDetectionHits;
    GetWorld()->SweepMultiByObjectType(allDetectionHits, detectionStartLocation, detectionEndLocation, FQuat::Identity, detectionTraceObjectTypes, FCollisionShape::MakeSphere(m_DetectionCapsuleRadius));

    FHitResult detectionHit;
    GetHightestPriorityDetectionHit(allDetectionHits, detectionHit);

    // Set behavior based on hit
    SetBehavior(deltaTime, detectionHit);

    DrawDebugCapsule(GetWorld(), detectionStartLocation + m_DetectionCapsuleHalfLength * selfPawn->GetActorForwardVector(), m_DetectionCapsuleHalfLength, m_DetectionCapsuleRadius, selfPawn->GetActorQuat() * selfPawn->GetActorUpVector().ToOrientationQuat(), FColor::Blue);
}

void ASDTAIController::GetHightestPriorityDetectionHit(const TArray<FHitResult> &hits, FHitResult &outDetectionHit)
{
    auto dist = INFINITY;
    for (const FHitResult &hit : hits)
    {
        if (UPrimitiveComponent *component = hit.GetComponent())
        {
            APawn* selfPawn = GetPawn();
            if (!selfPawn)
                return;
            //We make sure the agent cannot see the target trough walls.
            if (SDTUtils::Raycast(GetWorld(), selfPawn->GetActorLocation(), hit.GetActor()->GetActorLocation()))
                break;
            if (component->GetCollisionObjectType() == COLLISION_PLAYER)
            {
                // we can't get more important than the player
                outDetectionHit = hit;
                return;
            }
            else if (component->GetCollisionObjectType() == COLLISION_COLLECTIBLE)
            {
                if (ASDTCollectible *collectible = Cast<ASDTCollectible>(hit.GetActor()))
                {
                    if (!collectible->IsOnCooldown() && hit.Distance < dist)
                    {
                        outDetectionHit = hit;
                        dist = hit.Distance;
                    }
                }
            }
        }
    }
}

void ASDTAIController::SetBehavior(float deltaTime, FHitResult detectionHit)
{
    auto component = detectionHit.GetComponent();
    if (component == nullptr && m_ReachedTarget)
    {
        TargetActor = FindClosestCollectible();
        PlayerBehaviorChoice = PlayerBehavior::NO_PLAYER;
        canSeePlayer = false;
    }

    else if (component != nullptr && component->GetCollisionObjectType() == COLLISION_PLAYER) {
        auto playerPoweredUp = SDTUtils::IsPlayerPoweredUp(GetWorld());
        if (PlayerBehaviorChoice == PlayerBehavior::NO_PLAYER) {
            TargetActor = playerPoweredUp ? ChooseFleePoint(detectionHit.GetActor()->GetActorLocation()) : detectionHit.GetActor();
            AIStateInterrupted(); //We consider that the IA reached is previous objectives
            PlayerBehaviorChoice = playerPoweredUp ? PlayerBehavior::FLEE : PlayerBehavior::CHASE;
            canSeePlayer = true;
        }
        else if (PlayerBehaviorChoice == PlayerBehavior::CHASE && playerPoweredUp)
        {
            TargetActor = ChooseFleePoint(detectionHit.GetActor()->GetActorLocation());
            AIStateInterrupted();
            PlayerBehaviorChoice = PlayerBehavior::FLEE;
        }
        else if (PlayerBehaviorChoice == PlayerBehavior::FLEE && !playerPoweredUp)
        {
            TargetActor = detectionHit.GetActor();
            AIStateInterrupted();
            PlayerBehaviorChoice = PlayerBehavior::CHASE;
            canSeePlayer = true;
        }

        lastPlayerPosition = TargetActor != nullptr ? TargetActor->GetActorLocation() : FVector::ZeroVector;
    }
    else if (component != nullptr && m_ReachedTarget)
    {
        TargetActor = detectionHit.GetActor();
        PlayerBehaviorChoice = PlayerBehavior::NO_PLAYER;
        canSeePlayer = false;
    }
    else
    {
        if (PlayerBehaviorChoice == PlayerBehavior::CHASE)
        {
            // The player cannot be seen so we just stop the current path which is update to follow a path to the last position where the player was seen.
            AIStateInterrupted();
        }
        else if (PlayerBehaviorChoice == PlayerBehavior::NO_PLAYER)
        {
            auto actorAsCollectible = Cast<ASDTCollectible>(TargetActor);
            if (actorAsCollectible != nullptr && actorAsCollectible->IsOnCooldown())
            {
                AIStateInterrupted();
                // If component is not null, we know it is not the player because of the else if condition before.
                TargetActor = component != nullptr ? detectionHit.GetActor() : FindClosestCollectible();
            }
        }
        canSeePlayer = false;
    }
}

void ASDTAIController::AIStateInterrupted()
{
    GetPathFollowingComponent()->AbortMove(*this, FPathFollowingResultFlags::MovementStop | FPathFollowingResultFlags::ForcedScript, GetPathFollowingComponent()->GetCurrentRequestId(), EPathFollowingVelocityMode::Keep);
    m_ReachedTarget = true;
}

AActor *ASDTAIController::ChooseFleePoint(FVector playerPosition) const
{
    // Retrieve the actors for the positions to consider to go away from the player position.
    auto owningAICharacter = Cast<ASoftDesignTrainingCharacter>(GetCharacter());
    auto &fleePoints = owningAICharacter->FleeLocations;
    if (owningAICharacter == nullptr || fleePoints.IsEmpty()) {
        return nullptr;
    }

    // Using the position of the AI and the player, define a plane, then define a perpendicular plane from the first plane.
    // A point is more interesting if it is far from the first plane in the direction of the normal, but a point also becomes interesting if
    // if is far from the second plane on any side.
    auto AIPosition = owningAICharacter->GetActorLocation();
    auto planeNormal = FVector(AIPosition.X - playerPosition.X, AIPosition.Y - playerPosition.Y, 0).GetSafeNormal();
    FPlane playerPlane(playerPosition, planeNormal);
    FPlane playerParallelPlane(playerPosition, planeNormal.Cross(FVector::UpVector));
    return Algo::MaxElementBy(fleePoints, [&](auto element) {
        // The multiplication by 2 is a weight to prefer a point that is far from the first plane at first.
        return 2 * playerPlane.PlaneDot(element->GetActorLocation()) + FMath::Abs(playerParallelPlane.PlaneDot(element->GetActorLocation()));
    })->Get();
}

AActor* ASDTAIController::FindClosestCollectible() const
{
    auto min_dist = INFINITY;
    AActor *actor = nullptr;

    for (TActorIterator<ASDTCollectible> collectible(GetWorld()); collectible; ++collectible)
    {
        auto dist = FVector::Dist(GetPawn()->GetActorLocation(), collectible->GetActorLocation());
        if (!collectible->IsOnCooldown() && dist < min_dist)
        {
            actor = *collectible;
            min_dist = dist;
        }
    }

    return actor;
}
