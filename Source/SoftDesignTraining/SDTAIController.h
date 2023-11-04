// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SDTBaseAIController.h"
#include "SDTAIController.generated.h"

/**
 *
 */
UCLASS(ClassGroup = AI, config = Game)
class SOFTDESIGNTRAINING_API ASDTAIController : public ASDTBaseAIController
{
	GENERATED_BODY()

public:
    ASDTAIController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    //Half lenght of the capsule corresponding to the field of vision.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
    float m_DetectionCapsuleHalfLength = 500.f;

    //radius of the capsule corresponding to the field of vision.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
    float m_DetectionCapsuleRadius = 250.f;

    //offset beyhond which is the vision field
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
    float m_DetectionCapsuleForwardStartingOffset = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
    UCurveFloat* JumpCurve;

    //Scalar which regulate the height of the jump
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
    float JumpApexHeight = 300.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
    float JumpSpeed = 1.f;

    //Boolean which tells if the segment in a path is one where the agent has to jump
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI)
    bool AtJumpSegment = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI)
    bool InAir = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI)
    bool Landing = false;


public:
    virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;
    void AIStateInterrupted();

protected:
    void OnMoveToTarget();
    void GetHightestPriorityDetectionHit(const TArray<FHitResult>& hits, FHitResult& outDetectionHit);
    void UpdatePlayerInteraction(float deltaTime);

    AActor* TargetActor;

    void SetBehavior(float deltaTime, FHitResult detectionHit);

private:
    /// Define the behaviors that the AI can take when choosing what he should do.
    enum class PlayerBehavior
    {
        /// No player was detected. The AI is moving to a collectible.
        NO_PLAYER,
        /// The player was detected. The AI is moving to the player.
        CHASE,
        /// The player was detected and has a power up. The AI is going away from the player.
        FLEE
    };

    virtual void GoToBestTarget(float deltaTime) override;
    virtual void ChooseBehavior(float deltaTime) override;
    virtual void ShowNavigationPath() override;

    /// When going away from the player, the AI chooses the location corresponding to a list of actors (flee positions).
    ///
    /// The list of actors is taken from the Pawn that owns this AController.
    /// \param playerPosition The position of the player. Used to determine the best flee position to go.
    /// \return AActor The flee position that goes away from the player.
    AActor *ChooseFleePoint(FVector playerPosition) const;

    /// Find the closest collectible not on cooldown from the position of the actor.
    ///
    /// The distance calculated is just the distance between two points. It does not take into account the walls.
    /// \return AActor The closest collectible not on cooldown, or null if there is no collectible available.
    AActor *FindClosestCollectible() const;

    /// The actual behavior of the AI.
    PlayerBehavior PlayerBehaviorChoice = PlayerBehavior::NO_PLAYER;

    bool canSeePlayer = false;
    FVector lastPlayerPosition = FVector(0, 0, 0);
};
