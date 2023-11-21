// Copyright 2020 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffectTypes.h"
#include "GASPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class GAS_API AGASPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:
	AGASPlayerState();

	// Implement IAbilitySystemInterface
	class UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	class UGASAttributeSetBase* GetAttributeSetBase() const;

	UFUNCTION(BlueprintCallable, Category = "GAS|GASPlayerState")
	bool IsAlive() const;

	UFUNCTION(BlueprintCallable, Category = "GASDocumenation|GASPlayerState|UI")
	void ShowAbilityConfirmCancelText(bool ShowText);


	/**
	* Getters for attributes from GASAttributeSetBase. Returns Current Value unless otherwise specified.
	*/

	UFUNCTION(BlueprintCallable, Category = "GAS|GASPlayerState|Attributes")
	float GetHealth() const;

	UFUNCTION(BlueprintCallable, Category = "GAS|GASPlayerState|Attributes")
	float GetMaxHealth() const;

	UFUNCTION(BlueprintCallable, Category = "GAS|GASPlayerState|Attributes")
	float GetHealthRegenRate() const;

	UFUNCTION(BlueprintCallable, Category = "GAS|GASPlayerState|Attributes")
	float GetMana() const;

	UFUNCTION(BlueprintCallable, Category = "GAS|GASPlayerState|Attributes")
	float GetMaxMana() const;

	UFUNCTION(BlueprintCallable, Category = "GAS|GASPlayerState|Attributes")
	float GetManaRegenRate() const;

	UFUNCTION(BlueprintCallable, Category = "GAS|GASPlayerState|Attributes")
	float GetStamina() const;

	UFUNCTION(BlueprintCallable, Category = "GAS|GASPlayerState|Attributes")
	float GetMaxStamina() const;

	UFUNCTION(BlueprintCallable, Category = "GAS|GASPlayerState|Attributes")
	float GetStaminaRegenRate() const;

	UFUNCTION(BlueprintCallable, Category = "GAS|GASPlayerState|Attributes")
	float GetArmor() const;

	UFUNCTION(BlueprintCallable, Category = "GAS|GASPlayerState|Attributes")
	float GetMoveSpeed() const;

	UFUNCTION(BlueprintCallable, Category = "GAS|GASPlayerState|Attributes")
	int32 GetCharacterLevel() const;

	UFUNCTION(BlueprintCallable, Category = "GAS|GASPlayerState|Attributes")
	int32 GetXP() const;

	UFUNCTION(BlueprintCallable, Category = "GAS|GASPlayerState|Attributes")
	int32 GetXPBounty() const;

	UFUNCTION(BlueprintCallable, Category = "GAS|GASPlayerState|Attributes")
	int32 GetGold() const;

	UFUNCTION(BlueprintCallable, Category = "GAS|GASPlayerState|Attributes")
	int32 GetGoldBounty() const;

protected:
	UPROPERTY()
	class UGASAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY()
	class UGASAttributeSetBase* AttributeSetBase;

	FGameplayTag DeadTag;

	FDelegateHandle HealthChangedDelegateHandle;
	FDelegateHandle MaxHealthChangedDelegateHandle;
	FDelegateHandle HealthRegenRateChangedDelegateHandle;
	FDelegateHandle ManaChangedDelegateHandle;
	FDelegateHandle MaxManaChangedDelegateHandle;
	FDelegateHandle ManaRegenRateChangedDelegateHandle;
	FDelegateHandle StaminaChangedDelegateHandle;
	FDelegateHandle MaxStaminaChangedDelegateHandle;
	FDelegateHandle StaminaRegenRateChangedDelegateHandle;
	FDelegateHandle XPChangedDelegateHandle;
	FDelegateHandle GoldChangedDelegateHandle;
	FDelegateHandle CharacterLevelChangedDelegateHandle;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Attribute changed callbacks
	virtual void HealthChanged(const FOnAttributeChangeData& Data);
	virtual void MaxHealthChanged(const FOnAttributeChangeData& Data);
	virtual void HealthRegenRateChanged(const FOnAttributeChangeData& Data);
	virtual void ManaChanged(const FOnAttributeChangeData& Data);
	virtual void MaxManaChanged(const FOnAttributeChangeData& Data);
	virtual void ManaRegenRateChanged(const FOnAttributeChangeData& Data);
	virtual void StaminaChanged(const FOnAttributeChangeData& Data);
	virtual void MaxStaminaChanged(const FOnAttributeChangeData& Data);
	virtual void StaminaRegenRateChanged(const FOnAttributeChangeData& Data);
	virtual void XPChanged(const FOnAttributeChangeData& Data);
	virtual void GoldChanged(const FOnAttributeChangeData& Data);
	virtual void CharacterLevelChanged(const FOnAttributeChangeData& Data);

	// Tag change callbacks
	virtual void StunTagChanged(const FGameplayTag CallbackTag, int32 NewCount);
};
