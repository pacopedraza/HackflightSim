// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "HackflightSimCameraHUD.generated.h"

UCLASS(Config = Game)
class HACKFLIGHTSIM_API AHackflightSimCameraHUD : public AHUD
{
	GENERATED_BODY()

	virtual void DrawHUD() override;
		
};
