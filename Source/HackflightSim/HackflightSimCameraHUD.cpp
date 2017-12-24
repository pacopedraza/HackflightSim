// Fill out your copyright notice in the Description page of Project Settings.

#include "HackflightSimCameraHUD.h"


void AHackflightSimCameraHUD::DrawHUD()
{
	Super::DrawHUD();

	float rightx = LEFTX + WIDTH;
	float bottomy = TOPY + HEIGHT;

	drawBorderLine(LEFTX, TOPY, rightx, TOPY);
	drawBorderLine(rightx, TOPY, rightx, bottomy);
	drawBorderLine(rightx, bottomy, LEFTX, bottomy);
	drawBorderLine(LEFTX, bottomy, LEFTX, TOPY);



}

void AHackflightSimCameraHUD::drawBorderLine(float lx, float uy, float rx, float by)
{
	DrawLine(lx, uy, rx, by, BORDER_COLOR, BORDER_WIDTH);
}



