// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Public/Characters/GASCharacterMain.h"

// Sets default values
AGASCharacterMain::AGASCharacterMain()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AGASCharacterMain::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGASCharacterMain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AGASCharacterMain::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

