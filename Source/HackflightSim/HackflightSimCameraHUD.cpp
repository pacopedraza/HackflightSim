// Fill out your copyright notice in the Description page of Project Settings.

#include "HackflightSimCameraHUD.h"


void AHackflightSimCameraHUD::DrawHUD()
{
	Super::DrawHUD();

	DrawLine(200, 300, 400, 500, FLinearColor::Blue);

}

