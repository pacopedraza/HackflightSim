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

	const float LEFTX = 1600.f;
	const float TOPY = 600.f;
	const float WIDTH = 256.f;
	const float HEIGHT = 128.f;
	
	const FLinearColor BORDER_COLOR = FLinearColor::Yellow;
	const float BORDER_WIDTH = 2.0f;

	void drawBorder(float lx, float uy, float rx, float by);
};
