// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTProjectile.h"
#include "SoftDesignTraining.h"

ASDTProjectile::ASDTProjectile()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
}

void ASDTProjectile::Tick(float deltaTime)
{
    Super::Tick(deltaTime);

    if (m_Fired)
    {
        FVector impulse = m_Direction * m_Speed * deltaTime;
        SetActorLocation(GetActorLocation() + impulse);
    }
}

void ASDTProjectile::FireProjectile(const FVector& direction, float speed)
{
    m_Direction = direction;
    m_Speed = speed;

    m_Fired = true;
    m_StartingPosition = GetActorLocation();
}

void ASDTProjectile::ResetProjectile()
{
    SetActorLocation(m_StartingPosition);
}