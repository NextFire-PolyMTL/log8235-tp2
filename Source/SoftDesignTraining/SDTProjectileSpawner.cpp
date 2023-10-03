// Fill out your copyright notice in the Description page of Project Settings.

#include "SDTProjectileSpawner.h"
#include "SoftDesignTraining.h"

#include "Engine/World.h"
#include "TimerManager.h"


ASDTProjectileSpawner::ASDTProjectileSpawner()
{
}

void ASDTProjectileSpawner::BeginPlay()
{
    Super::BeginPlay();

    //shoot a projectile on spawn
    FireProjectile();

    GetWorldTimerManager().SetTimer(m_ShotCooldownTimer, this, &ASDTProjectileSpawner::OnReadyToShoot, m_TimeToShoot, true);
}

void ASDTProjectileSpawner::OnReadyToShoot()
{
    FireProjectile();
}

void ASDTProjectileSpawner::FireProjectile()
{
    if (m_Projectiles.Num() >= m_MaxSimultaneousProjectiles)
    {
        ResetOldestProjectile();
    }
    else
    {
        ASDTProjectile* projectile = GetWorld()->SpawnActor<class ASDTProjectile>(m_SDTProjectileBP, GetActorLocation(), GetActorRotation());
        m_Projectiles.Add(projectile);

        projectile->FireProjectile(m_ShotDirection, m_ShotSpeed);
    }
}

void ASDTProjectileSpawner::ResetOldestProjectile()
{
    m_Projectiles[m_OldestProjectileIndex]->ResetProjectile();
    ++m_OldestProjectileIndex;

    if (m_OldestProjectileIndex >= m_Projectiles.Num())
        m_OldestProjectileIndex = 0;
}




