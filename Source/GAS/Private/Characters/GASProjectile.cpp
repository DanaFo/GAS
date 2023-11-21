// Copyright 2020 Dan Kestranek.


#include "..\..\Public\Characters\GASProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"

// Sets default values
AGASProjectile::AGASProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(FName("ProjectileMovement"));
}

// Called when the game starts or when spawned
void AGASProjectile::BeginPlay()
{
	Super::BeginPlay();
	
}

