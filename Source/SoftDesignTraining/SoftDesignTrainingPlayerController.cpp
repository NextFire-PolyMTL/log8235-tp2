// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "SoftDesignTrainingPlayerController.h"
#include "SoftDesignTraining.h"
#include "SoftDesignTrainingCharacter.h"

#include "DrawDebugHelpers.h"

ASoftDesignTrainingPlayerController::ASoftDesignTrainingPlayerController()
{
}

void ASoftDesignTrainingPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    InputComponent->BindAxis("MoveForward", this, &ASoftDesignTrainingPlayerController::MoveForward);
    InputComponent->BindAxis("MoveRight", this, &ASoftDesignTrainingPlayerController::MoveRight);

    InputComponent->BindAction("Jump", EInputEvent::IE_Pressed, this, &ASoftDesignTrainingPlayerController::Jump);
}

void ASoftDesignTrainingPlayerController::MoveForward(float value)
{
    APawn* const pawn = GetPawn();

    if (pawn)
    {
        pawn->AddMovementInput(FVector(value, 0.0f, 0.0f));
    }
}

void ASoftDesignTrainingPlayerController::MoveRight(float value)
{
    APawn* const pawn = GetPawn();

    if (pawn)
    {
        pawn->AddMovementInput(FVector(0.0f, value, 0.0f));
    }
}

void ASoftDesignTrainingPlayerController::Jump()
{
    ACharacter* character = Cast<ACharacter>(GetPawn());
    if (character)
    {
        character->Jump();
    }
}