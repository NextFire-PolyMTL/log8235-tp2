// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "SDTProjectile.generated.h"

/**
 * 
 */
UCLASS()
class SOFTDESIGNTRAINING_API ASDTProjectile : public AStaticMeshActor
{
	GENERATED_BODY()

public:
    ASDTProjectile();

    virtual void Tick(float deltaTime) override;

    void FireProjectile(const FVector& direction, float speed);
    void ResetProjectile();

protected:
    float m_Speed;
    FVector m_Direction;

    UPROPERTY(EditAnywhere)
        float m_TimeToLive = 5.f;

    bool m_Fired = false;
    FVector m_StartingPosition;
};
