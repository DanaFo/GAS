// Copyright 2020 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "Characters/GASCharacterMain.h"
#include "GameplayEffectTypes.h"
#include "GASMinionCharacter.generated.h"

/**
 * An AI controlled minion character.
 */
UCLASS()
class GAS_API AGASMinionCharacter : public AGASCharacterMain
{
	GENERATED_BODY()

public:
	AGASMinionCharacter(const class FObjectInitializer& ObjectInitializer);

protected:

	// Actual hard pointer to AbilitySystemComponent
	UPROPERTY()
	class UGASAbilitySystemComponent* HardRefAbilitySystemComponent;

	// Actual hard pointer to AttributeSetBase
	UPROPERTY()
	class UGASAttributeSetBase* HardRefAttributeSetBase;
	
	virtual void BeginPlay() override;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gas|UI")
	TSubclassOf<class UGASFloatingStatusBarWidget> UIFloatingStatusBarClass;

	UPROPERTY()
	class UGASFloatingStatusBarWidget* UIFloatingStatusBar;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Gas|UI")
	class UWidgetComponent* UIFloatingStatusBarComponent;

	FDelegateHandle HealthChangedDelegateHandle;

	// Attribute changed callbacks
	virtual void HealthChanged(const FOnAttributeChangeData& Data);

	// Tag change callbacks
	virtual void StunTagChanged(const FGameplayTag CallbackTag, int32 NewCount);
};
