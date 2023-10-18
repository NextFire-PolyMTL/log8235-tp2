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

ASDTAIController::ASDTAIController(const FObjectInitializer &ObjectInitializer)
    : Super(ObjectInitializer.SetDefaultSubobjectClass<USDTPathFollowingComponent>(TEXT("PathFollowingComponent")))
{
}

void ASDTAIController::GoToBestTarget(float deltaTime)
{
    // Move to target depending on current behavior
    ShowNavigationPath();
    OnMoveToTarget();
    MoveToActor(TargetActor);
}

void ASDTAIController::OnMoveToTarget()
{
    m_ReachedTarget = false;
}

void ASDTAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult &Result)
{
    Super::OnMoveCompleted(RequestID, Result);

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
    else{

       auto points = currentPath->GetPathPoints();
       
        for (size_t i = 0; i < points.Num() - 1; i++)
        {
            DrawDebugLine(GetWorld(), points[i], points[i + 1], FColor::Green, false, 0.1f, 0, 5.f);
        }
        
        auto color = FColor::Red;
        DrawDebugSphere(GetWorld(), TargetActor->GetActorLocation(), 50.f, 10, color, false, 0.1f, 0, 5.f);
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
        auto min_dist = INFINITY;

        for (TActorIterator<ASDTCollectible> collectible(GetWorld()); collectible; ++collectible)
        {
           // GEngine->AddOnScreenDebugMessage(INDEX_NONE, 2.0, FColor::Red, FString("go to collectible outside sphere"));
            auto dist = FVector::Dist(GetPawn()->GetActorLocation(), collectible->GetActorLocation());
            if (!collectible->IsOnCooldown() && dist < min_dist)
            {
                TargetActor = *collectible;
                min_dist = dist;
            }
        }
        followPlayer = false;
    }
    else if (component != nullptr && (m_ReachedTarget || component->GetCollisionObjectType() == COLLISION_PLAYER))
    {
        TargetActor = detectionHit.GetActor();
        //followPlayer = false;
        //m_ReachedTarget = true; //We consider that the IA reached

    }
}

void ASDTAIController::AIStateInterrupted()
{
    StopMovement();
    m_ReachedTarget = true;
}
