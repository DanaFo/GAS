// Copyright 2020 Dan Kestranek.


#include "Characters/Abilities/AttributeSets/GASAttributeSetBase.h"
#include "Characters/GASCharacterMain.h"
#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"
#include "..\..\..\..\Public\Player\GASPlayerController.h"

UGASAttributeSetBase::UGASAttributeSetBase()
{
	// Cache tags
	HitDirectionFrontTag = FGameplayTag::RequestGameplayTag(FName("Effect.HitReact.Front"));
	HitDirectionBackTag = FGameplayTag::RequestGameplayTag(FName("Effect.HitReact.Back"));
	HitDirectionRightTag = FGameplayTag::RequestGameplayTag(FName("Effect.HitReact.Right"));
	HitDirectionLeftTag = FGameplayTag::RequestGameplayTag(FName("Effect.HitReact.Left"));
}

void UGASAttributeSetBase::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	// This is called whenever attributes change, so for max health/mana we want to scale the current totals to match
	Super::PreAttributeChange(Attribute, NewValue);

	// If a Max value changes, adjust current to keep Current % of Current to Max
	if (Attribute == GetMaxHealthAttribute()) // GetMaxHealthAttribute comes from the Macros defined at the top of the header
	{
		AdjustAttributeForMaxChange(Health, MaxHealth, NewValue, GetHealthAttribute());
	}
	else if (Attribute == GetMaxManaAttribute())
	{
		AdjustAttributeForMaxChange(Mana, MaxMana, NewValue, GetManaAttribute());
	}
	else if (Attribute == GetMaxStaminaAttribute())
	{
		AdjustAttributeForMaxChange(Stamina, MaxStamina, NewValue, GetStaminaAttribute());
	}
	else if (Attribute == GetMoveSpeedAttribute())
	{
		// Cannot slow less than 150 units/s and cannot boost more than 1000 units/s
		NewValue = FMath::Clamp<float>(NewValue, 150, 1000);
	}
}

void UGASAttributeSetBase::PostGameplayEffectExecute(const FGameplayEffectModCallbackData & Data)
{
	Super::PostGameplayEffectExecute(Data);

	FGameplayEffectContextHandle Context = Data.EffectSpec.GetContext();
	UAbilitySystemComponent* Source = Context.GetOriginalInstigatorAbilitySystemComponent();
	const FGameplayTagContainer& SourceTags = *Data.EffectSpec.CapturedSourceTags.GetAggregatedTags();
	FGameplayTagContainer SpecAssetTags;
	Data.EffectSpec.GetAllAssetTags(SpecAssetTags);

	// Get the Target actor, which should be our owner
	AActor* TargetActor = nullptr;
	AController* TargetController = nullptr;
	AGASCharacterMain* TargetCharacter = nullptr;
	if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
	{
		TargetActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
		TargetController = Data.Target.AbilityActorInfo->PlayerController.Get();
		TargetCharacter = Cast<AGASCharacterMain>(TargetActor);
	}

	// Get the Source actor
	AActor* SourceActor = nullptr;
	AController* SourceController = nullptr;
	AGASCharacterMain* SourceCharacter = nullptr;
	if (Source && Source->AbilityActorInfo.IsValid() && Source->AbilityActorInfo->AvatarActor.IsValid())
	{
		SourceActor = Source->AbilityActorInfo->AvatarActor.Get();
		SourceController = Source->AbilityActorInfo->PlayerController.Get();
		if (SourceController == nullptr && SourceActor != nullptr)
		{
			if (APawn* Pawn = Cast<APawn>(SourceActor))
			{
				SourceController = Pawn->GetController();
			}
		}

		// Use the controller to find the source pawn
		if (SourceController)
		{
			SourceCharacter = Cast<AGASCharacterMain>(SourceController->GetPawn());
		}
		else
		{
			SourceCharacter = Cast<AGASCharacterMain>(SourceActor);
		}

		// Set the causer actor based on context if it's set
		if (Context.GetEffectCauser())
		{
			SourceActor = Context.GetEffectCauser();
		}
	}

	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		// Try to extract a hit result
		FHitResult HitResult;
		if (Context.GetHitResult())
		{
			HitResult = *Context.GetHitResult();
		}

		// Store a local copy of the amount of damage done and clear the damage attribute
		const float LocalDamageDone = GetDamage();
		SetDamage(0.f);
	
		if (LocalDamageDone > 0.0f)
		{
			// If character was alive before damage is added, handle damage
			// This prevents damage being added to dead things and replaying death animations
			bool WasAlive = true;

			if (TargetCharacter)
			{
				WasAlive = TargetCharacter->IsAlive();
			}

			if (!TargetCharacter->IsAlive())
			{
				//UE_LOG(LogTemp, Warning, TEXT("%s() %s is NOT alive when receiving damage"), TEXT(__FUNCTION__), *TargetCharacter->GetName());
			}

			// Apply the health change and then clamp it
			const float NewHealth = GetHealth() - LocalDamageDone;
			SetHealth(FMath::Clamp(NewHealth, 0.0f, GetMaxHealth()));

			if (TargetCharacter && WasAlive)
			{
				// This is the log statement for damage received. Turned off for live games.
				//UE_LOG(LogTemp, Log, TEXT("%s() %s Damage Received: %f"), TEXT(__FUNCTION__), *GetOwningActor()->GetName(), LocalDamageDone);

				// Play HitReact animation and sound with a multicast RPC.
				const FHitResult* Hit = Data.EffectSpec.GetContext().GetHitResult();

				if (Hit)
				{
					EGASHitReactDirection HitDirection = TargetCharacter->GetHitReactDirection(Data.EffectSpec.GetContext().GetHitResult()->Location);
					switch (HitDirection)
					{
					case EGASHitReactDirection::Left:
						TargetCharacter->PlayHitReact(HitDirectionLeftTag, SourceCharacter);
						break;
					case EGASHitReactDirection::Front:
						TargetCharacter->PlayHitReact(HitDirectionFrontTag, SourceCharacter);
						break;
					case EGASHitReactDirection::Right:
						TargetCharacter->PlayHitReact(HitDirectionRightTag, SourceCharacter);
						break;
					case EGASHitReactDirection::Back:
						TargetCharacter->PlayHitReact(HitDirectionBackTag, SourceCharacter);
						break;
					}
				}
				else
				{
					// No hit result. Default to front.
					TargetCharacter->PlayHitReact(HitDirectionFrontTag, SourceCharacter);
				}

				// Show damage number for the Source player unless it was self damage
				if (SourceActor != TargetActor)
				{
					AGASPlayerController* PC = Cast<AGASPlayerController>(SourceController);
					if (PC)
					{
						PC->ShowDamageNumber(LocalDamageDone, TargetCharacter);
					}
				}

				if (!TargetCharacter->IsAlive())
				{
					// TargetCharacter was alive before this damage and now is not alive, give XP and Gold bounties to Source.
					// Don't give bounty to self.
					if (SourceController != TargetController)
					{
						// Create a dynamic instant Gameplay Effect to give the bounties
						UGameplayEffect* GEBounty = NewObject<UGameplayEffect>(GetTransientPackage(), FName(TEXT("Bounty")));
						GEBounty->DurationPolicy = EGameplayEffectDurationType::Instant;

						int32 Idx = GEBounty->Modifiers.Num();
						GEBounty->Modifiers.SetNum(Idx + 2);

						FGameplayModifierInfo& InfoXP = GEBounty->Modifiers[Idx];
						InfoXP.ModifierMagnitude = FScalableFloat(GetXPBounty());
						InfoXP.ModifierOp = EGameplayModOp::Additive;
						InfoXP.Attribute = UGASAttributeSetBase::GetXPAttribute();

						FGameplayModifierInfo& InfoGold = GEBounty->Modifiers[Idx + 1];
						InfoGold.ModifierMagnitude = FScalableFloat(GetGoldBounty());
						InfoGold.ModifierOp = EGameplayModOp::Additive;
						InfoGold.Attribute = UGASAttributeSetBase::GetGoldAttribute();

						Source->ApplyGameplayEffectToSelf(GEBounty, 1.0f, Source->MakeEffectContext());
					}
				}
			}
		}
	}// Damage
	else if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		// Handle other health changes.
		// Health loss should go through Damage.
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
	} // Health
	else if (Data.EvaluatedData.Attribute == GetManaAttribute())
	{
		// Handle mana changes.
		SetMana(FMath::Clamp(GetMana(), 0.0f, GetMaxMana()));
	} // Mana
	else if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
	{
		// Handle stamina changes.
		SetStamina(FMath::Clamp(GetStamina(), 0.0f, GetMaxStamina()));
	}
}

void UGASAttributeSetBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UGASAttributeSetBase, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGASAttributeSetBase, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGASAttributeSetBase, HealthRegenRate, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGASAttributeSetBase, Mana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGASAttributeSetBase, MaxMana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGASAttributeSetBase, ManaRegenRate, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGASAttributeSetBase, Stamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGASAttributeSetBase, MaxStamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGASAttributeSetBase, StaminaRegenRate, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGASAttributeSetBase, Armor, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGASAttributeSetBase, MoveSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGASAttributeSetBase, CharacterLevel, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGASAttributeSetBase, XP, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGASAttributeSetBase, XPBounty, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGASAttributeSetBase, Gold, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGASAttributeSetBase, GoldBounty, COND_None, REPNOTIFY_Always);
}

void UGASAttributeSetBase::AdjustAttributeForMaxChange(FGameplayAttributeData & AffectedAttribute, const FGameplayAttributeData & MaxAttribute, float NewMaxValue, const FGameplayAttribute & AffectedAttributeProperty)
{
	UAbilitySystemComponent* AbilityComp = GetOwningAbilitySystemComponent();
	const float CurrentMaxValue = MaxAttribute.GetCurrentValue();
	if (!FMath::IsNearlyEqual(CurrentMaxValue, NewMaxValue) && AbilityComp)
	{
		// Change current value to maintain the current Val / Max percent
		const float CurrentValue = AffectedAttribute.GetCurrentValue();
		float NewDelta = (CurrentMaxValue > 0.f) ? (CurrentValue * NewMaxValue / CurrentMaxValue) - CurrentValue : NewMaxValue;

		AbilityComp->ApplyModToAttributeUnsafe(AffectedAttributeProperty, EGameplayModOp::Additive, NewDelta);
	}
}

void UGASAttributeSetBase::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGASAttributeSetBase, Health, OldHealth);
}

void UGASAttributeSetBase::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGASAttributeSetBase, MaxHealth, OldMaxHealth);
}

void UGASAttributeSetBase::OnRep_HealthRegenRate(const FGameplayAttributeData& OldHealthRegenRate)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGASAttributeSetBase, HealthRegenRate, OldHealthRegenRate);
}

void UGASAttributeSetBase::OnRep_Mana(const FGameplayAttributeData& OldMana)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGASAttributeSetBase, Mana, OldMana);
}

void UGASAttributeSetBase::OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGASAttributeSetBase, MaxMana, OldMaxMana);
}

void UGASAttributeSetBase::OnRep_ManaRegenRate(const FGameplayAttributeData& OldManaRegenRate)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGASAttributeSetBase, ManaRegenRate, OldManaRegenRate);
}

void UGASAttributeSetBase::OnRep_Stamina(const FGameplayAttributeData& OldStamina)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGASAttributeSetBase, Stamina, OldStamina);
}

void UGASAttributeSetBase::OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGASAttributeSetBase, MaxStamina, OldMaxStamina);
}

void UGASAttributeSetBase::OnRep_StaminaRegenRate(const FGameplayAttributeData& OldStaminaRegenRate)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGASAttributeSetBase, StaminaRegenRate, OldStaminaRegenRate);
}

void UGASAttributeSetBase::OnRep_Armor(const FGameplayAttributeData& OldArmor)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGASAttributeSetBase, Armor, OldArmor);
}

void UGASAttributeSetBase::OnRep_MoveSpeed(const FGameplayAttributeData& OldMoveSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGASAttributeSetBase, MoveSpeed, OldMoveSpeed);
}

void UGASAttributeSetBase::OnRep_CharacterLevel(const FGameplayAttributeData& OldCharacterLevel)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGASAttributeSetBase, CharacterLevel, OldCharacterLevel);
}

void UGASAttributeSetBase::OnRep_XP(const FGameplayAttributeData& OldXP)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGASAttributeSetBase, XP, OldXP);
}

void UGASAttributeSetBase::OnRep_XPBounty(const FGameplayAttributeData& OldXPBounty)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGASAttributeSetBase, XPBounty, OldXPBounty);
}

void UGASAttributeSetBase::OnRep_Gold(const FGameplayAttributeData& OldGold)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGASAttributeSetBase, Gold, OldGold);
}

void UGASAttributeSetBase::OnRep_GoldBounty(const FGameplayAttributeData& OldGoldBounty)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UGASAttributeSetBase, GoldBounty, OldGoldBounty);
}
