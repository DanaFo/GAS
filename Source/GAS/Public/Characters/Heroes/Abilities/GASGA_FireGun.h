// Copyright 2020 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "Characters/Abilities/GASGameplayAbility.h"
#include "Characters/Abilities/AbilityTasks/GASAT_PlayMontageAndWaitForEvent.h"
#include "Characters/GASProjectile.h"
#include "GASGA_FireGun.generated.h"

/**
 * 
 */
UCLASS()
class GAS_API UGASGA_FireGun : public UGASGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGASGA_FireGun();

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	UAnimMontage* FireHipMontage;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	UAnimMontage* FireIronsightsMontage;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TSubclassOf<AGASProjectile> ProjectileClass;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TSubclassOf<UGameplayEffect> DamageGameplayEffect;

	/** Actually activate ability, do not call this directly. We'll call it from APAHeroCharacter::ActivateAbilitiesWithTags(). */
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	float Range;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	float Damage;

	UFUNCTION()
	void OnCancelled(FGameplayTag EventTag, FGameplayEventData EventData);

	UFUNCTION()
	void OnCompleted(FGameplayTag EventTag, FGameplayEventData EventData);

	UFUNCTION()
	void EventReceived(FGameplayTag EventTag, FGameplayEventData EventData);
};
