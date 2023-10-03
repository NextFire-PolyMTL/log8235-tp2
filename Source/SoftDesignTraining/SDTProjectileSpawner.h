// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "SDTProjectile.h"

#include "SDTProjectileSpawner.generated.h"

/**
 * 
 */
UCLASS()
class SOFTDESIGNTRAINING_API ASDTProjectileSpawner : public AStaticMeshActor
{
	GENERATED_BODY()
public:
    // Sets default values for this actor's properties
    ASDTProjectileSpawner();

protected:
    virtual void BeginPlay() override;

    void OnReadyToShoot();
    void FireProjectile();
    void ResetOldestProjectile();

    UPROPERTY(EditDefaultsOnly, Category = "ActorSpawning")
        TSubclassOf<ASDTProjectile> m_SDTProjectileBP;

    UPROPERTY(EditAnywhere)
        float m_TimeToShoot = 1.f;

    UPROPERTY(EditAnywhere)
        FVector m_ShotDirection;

    UPROPERTY(EditAnywhere)
        float m_ShotSpeed = 400.f;

    UPROPERTY(EditAnywhere)
        int32 m_MaxSimultaneousProjectiles = 5;

    FTimerHandle m_ShotCooldownTimer;
    TArray<ASDTProjectile*> m_Projectiles;
    int32 m_OldestProjectileIndex = 0;
};
