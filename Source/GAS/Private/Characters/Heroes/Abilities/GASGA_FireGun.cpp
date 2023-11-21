// Copyright 2020 Dan Kestranek.


#include "Characters/Heroes/Abilities/GASGA_FireGun.h"
#include "AbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "Characters/Heroes/GASHeroCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"

UGASGA_FireGun::UGASGA_FireGun()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	FGameplayTag Ability1Tag = FGameplayTag::RequestGameplayTag(FName("Ability.Skill.Ability1"));
	AbilityTags.AddTag(Ability1Tag);
	ActivationOwnedTags.AddTag(Ability1Tag);

	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Skill")));

	Range = 1000.0f;
	Damage = 12.0f;
}

void UGASGA_FireGun::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo * ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData * TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
	}

	UAnimMontage* MontageToPlay = FireHipMontage;

	if (GetAbilitySystemComponentFromActorInfo()->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.AimDownSights"))) &&
		!GetAbilitySystemComponentFromActorInfo()->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.AimDownSights.Removal"))))
	{
		MontageToPlay = FireIronsightsMontage;
	}

	// Play fire montage and wait for event telling us to spawn the projectile
	UGASAT_PlayMontageAndWaitForEventT_WaitReceiveDamage* Task = UGASAT_PlayMontageAndWaitForEventT_WaitReceiveDamage::PlayMontageAndWaitForEvent(this, NAME_None, MontageToPlay, FGameplayTagContainer(), 1.0f, NAME_None, false, 1.0f);
	Task->OnBlendOut.AddDynamic(this, &UGASGA_FireGun::OnCompleted);
	Task->OnCompleted.AddDynamic(this, &UGASGA_FireGun::OnCompleted);
	Task->OnInterrupted.AddDynamic(this, &UGASGA_FireGun::OnCancelled);
	Task->OnCancelled.AddDynamic(this, &UGASGA_FireGun::OnCancelled);
	Task->EventReceived.AddDynamic(this, &UGASGA_FireGun::EventReceived);
	// ReadyForActivation() is how you activate the AbilityTask in C++. Blueprint has magic from K2Node_LatentGameplayTaskCall that will automatically call ReadyForActivation().
	Task->ReadyForActivation();
}

void UGASGA_FireGun::OnCancelled(FGameplayTag EventTag, FGameplayEventData EventData)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGASGA_FireGun::OnCompleted(FGameplayTag EventTag, FGameplayEventData EventData)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGASGA_FireGun::EventReceived(FGameplayTag EventTag, FGameplayEventData EventData)
{
	// Montage told us to end the ability before the montage finished playing.
	// Montage was set to continue playing animation even after ability ends so this is okay.
	if (EventTag == FGameplayTag::RequestGameplayTag(FName("Event.Montage.EndAbility")))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	// Only spawn projectiles on the Server.
	// Predicting projectiles is an advanced topic not covered in this example.
	if (GetOwningActorFromActorInfo()->GetLocalRole() == ROLE_Authority && EventTag == FGameplayTag::RequestGameplayTag(FName("Event.Montage.SpawnProjectile")))
	{
		AGASHeroCharacter* Hero = Cast<AGASHeroCharacter>(GetAvatarActorFromActorInfo());
		if (!Hero)
		{
			EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		}

		FVector Start = Hero->GetGunComponent()->GetSocketLocation(FName("Muzzle"));
		FVector End = Hero->GetCameraBoom()->GetComponentLocation() + Hero->GetFollowCamera()->GetForwardVector() * Range;
		FRotator Rotation = UKismetMathLibrary::FindLookAtRotation(Start, End);

		FGameplayEffectSpecHandle DamageEffectSpecHandle = MakeOutgoingGameplayEffectSpec(DamageGameplayEffect, GetAbilityLevel());
		
		// Pass the damage to the Damage Execution Calculation through a SetByCaller value on the GameplayEffectSpec
		DamageEffectSpecHandle.Data.Get()->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Damage")), Damage);

		FTransform MuzzleTransform = Hero->GetGunComponent()->GetSocketTransform(FName("Muzzle"));
		MuzzleTransform.SetRotation(Rotation.Quaternion());
		MuzzleTransform.SetScale3D(FVector(1.0f));

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AGASProjectile* Projectile = GetWorld()->SpawnActorDeferred<AGASProjectile>(ProjectileClass, MuzzleTransform, GetOwningActorFromActorInfo(),
			Hero, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		Projectile->DamageEffectSpecHandle = DamageEffectSpecHandle;
		Projectile->Range = Range;
		Projectile->FinishSpawning(MuzzleTransform);
	}
}
