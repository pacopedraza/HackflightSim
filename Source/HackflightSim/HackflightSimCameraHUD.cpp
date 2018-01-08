/*
   HackflihtSimCameraHUD.cpp: heads-up display implementation

   Copyright (C) Simon D. Levy 2017

   This file is part of HackflightSim.

   HackflightSim is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   HackflightSim is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   You should have received a copy of the GNU General Public License
   along with Hackflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "HackflightSimCameraHUD.h"


#include <debug.hpp>

AHackflightSimCameraHUD::AHackflightSimCameraHUD()
{
	// Get Vision render target from blueprint
	static ConstructorHelpers::FObjectFinder<UTextureRenderTarget2D> VisionTexObj(TEXT("/Game/Hackflight/T_Vision"));
	VisionTextureRenderTarget = VisionTexObj.Object;

	// Creates Texture2D to store VisionTex content
	UTexture2D* VisionTexture = UTexture2D::CreateTransient(VisionTextureRenderTarget->SizeX, VisionTextureRenderTarget->SizeY, PF_B8G8R8A8);

#if WITH_EDITORONLY_DATA
	VisionTexture->MipGenSettings = TMGS_NoMipmaps;
#endif
	VisionTexture->SRGB = VisionTextureRenderTarget->SRGB;

	VisionRenderTarget = VisionTextureRenderTarget->GameThread_GetRenderTargetResource();
}


void AHackflightSimCameraHUD::DrawHUD()
{
	Super::DrawHUD();

	// Draw the image to the HUD
	DrawTextureSimple(VisionTextureRenderTarget, LEFTX, TOPY, 1.0f, true);

	float rightx = LEFTX + WIDTH;
	float bottomy = TOPY + HEIGHT;

	drawBorder(LEFTX, TOPY, rightx, TOPY);
	drawBorder(rightx, TOPY, rightx, bottomy);
	drawBorder(rightx, bottomy, LEFTX, bottomy);
	drawBorder(LEFTX, bottomy, LEFTX, TOPY);
}

void AHackflightSimCameraHUD::drawBorder(float lx, float uy, float rx, float by)
{
	DrawLine(lx, uy, rx, by, BORDER_COLOR, BORDER_WIDTH);
}



