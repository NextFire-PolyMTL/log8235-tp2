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

    /// Half lenght of the capsule corresponding to the field of vision.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
    float m_DetectionCapsuleHalfLength = 500.f;

    /// Radius of the capsule corresponding to the field of vision.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
    float m_DetectionCapsuleRadius = 250.f;

    /// Offset beyhond which is the vision field.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
    float m_DetectionCapsuleForwardStartingOffset = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
    UCurveFloat* JumpCurve;

    /// Scalar which regulate the height of the jump.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
    float JumpApexHeight = 300.f;

    /// Scalar that regulates the speed at which the player follows JumpCurve.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
    float JumpSpeed = 1.f;

    /// Boolean which tells if the segment in a path is one where the agent has to jump
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI)
    bool AtJumpSegment = false;

public:
    virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;
    void AIStateInterrupted();

protected:
    void OnMoveToTarget();
    /// Get the detection that should be priorize among other.
    ///
    /// The list of actors is taken from the Pawn that owns this AController.
    /// \param hits list of hits obtain through physical requests on the world
    /// \param outDetectionHit [out] the hit that should be considered in priority among the other hits.
    void GetHightestPriorityDetectionHit(const TArray<FHitResult>& hits, FHitResult& outDetectionHit);

    /// Update the agent behavior by making sweep request on the world to get a list of hit actors,
    /// and call SetBehavior by given the prioritized hit.
    ///
    /// \param deltaTime.
    void UpdatePlayerInteraction(float deltaTime);

    //Agent's target
    AActor *TargetActor;

    /// Set the behavior of the agent regarding the FHitResult giv in parameters
    ///
    /// \param detectionHit hit's result.
    /// \param deltaTime.
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


    /// Initiate the move to the target.
    ///
    /// \param deltaTime.
    virtual void GoToBestTarget(float deltaTime) override;

    /// Call to update the behavior.
    ///
    /// \param deltaTime.
    virtual void ChooseBehavior(float deltaTime) override;

    /// Show the path currently teken by the agent
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

    /// Bool which say if the player is in the vision field of the agent
    bool CanSeePlayer = false;
    /// Last player position known by the player
    FVector LastPlayerPosition = FVector::ZeroVector;
};
