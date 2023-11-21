// Copyright 2020 Dan Kestranek.


#include "..\..\Public\Player\GASPlayerState.h"
#include "Characters/Abilities/AttributeSets/GASAttributeSetBase.h"
#include "Characters/Abilities/GASAbilitySystemComponent.h"
#include "Characters/Heroes/GASHeroCharacter.h"
#include "..\..\Public\Player\GASPlayerController.h"
#include "..\..\Public\UI\GASFloatingStatusBarWidget.h"
#include "..\..\Public\UI\GASHUDWidget.h"

AGASPlayerState::AGASPlayerState()
{
	// Create ability system component, and set it to be explicitly replicated
	AbilitySystemComponent = CreateDefaultSubobject<UGASAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);

	// Mixed mode means we only are replicated the GEs to ourself, not the GEs to simulated proxies. If another GDPlayerState (Hero) receives a GE,
	// we won't be told about it by the Server. Attributes, GameplayTags, and GameplayCues will still replicate to us.
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// Create the attribute set, this replicates by default
	// Adding it as a subobject of the owning actor of an AbilitySystemComponent
	// automatically registers the AttributeSet with the AbilitySystemComponent
	AttributeSetBase = CreateDefaultSubobject<UGASAttributeSetBase>(TEXT("AttributeSetBase"));

	// Set PlayerState's NetUpdateFrequency to the same as the Character.
	// Default is very low for PlayerStates and introduces perceived lag in the ability system.
	// 100 is probably way too high for a shipping game, you can adjust to fit your needs.
	NetUpdateFrequency = 100.0f;

	// Cache tags
	DeadTag = FGameplayTag::RequestGameplayTag(FName("State.Dead"));
}

UAbilitySystemComponent * AGASPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UGASAttributeSetBase * AGASPlayerState::GetAttributeSetBase() const
{
	return AttributeSetBase;
}

bool AGASPlayerState::IsAlive() const
{
	return GetHealth() > 0.0f;
}

void AGASPlayerState::ShowAbilityConfirmCancelText(bool ShowText)
{
	AGASPlayerController* PC = Cast<AGASPlayerController>(GetOwner());
	if (PC)
	{
		UGASHUDWidget* HUD = PC->GetHUD();
		if (HUD)
		{
			HUD->ShowAbilityConfirmCancelText(ShowText);
		}
	}
}

float AGASPlayerState::GetHealth() const
{
	return AttributeSetBase->GetHealth();
}

float AGASPlayerState::GetMaxHealth() const
{
	return AttributeSetBase->GetMaxHealth();
}

float AGASPlayerState::GetHealthRegenRate() const
{
	return AttributeSetBase->GetHealthRegenRate();
}

float AGASPlayerState::GetMana() const
{
	return AttributeSetBase->GetMana();
}

float AGASPlayerState::GetMaxMana() const
{
	return AttributeSetBase->GetMaxMana();
}

float AGASPlayerState::GetManaRegenRate() const
{
	return AttributeSetBase->GetManaRegenRate();
}

float AGASPlayerState::GetStamina() const
{
	return AttributeSetBase->GetStamina();
}

float AGASPlayerState::GetMaxStamina() const
{
	return AttributeSetBase->GetMaxStamina();
}

float AGASPlayerState::GetStaminaRegenRate() const
{
	return AttributeSetBase->GetStaminaRegenRate();
}

float AGASPlayerState::GetArmor() const
{
	return AttributeSetBase->GetArmor();
}

float AGASPlayerState::GetMoveSpeed() const
{
	return AttributeSetBase->GetMoveSpeed();
}

int32 AGASPlayerState::GetCharacterLevel() const
{
	return AttributeSetBase->GetCharacterLevel();
}

int32 AGASPlayerState::GetXP() const
{
	return AttributeSetBase->GetXP();
}

int32 AGASPlayerState::GetXPBounty() const
{
	return AttributeSetBase->GetXPBounty();
}

int32 AGASPlayerState::GetGold() const
{
	return AttributeSetBase->GetGold();
}

int32 AGASPlayerState::GetGoldBounty() const
{
	return AttributeSetBase->GetGoldBounty();
}

void AGASPlayerState::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystemComponent)
	{
		// Attribute change callbacks
		HealthChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSetBase->GetHealthAttribute()).AddUObject(this, &AGASPlayerState::HealthChanged);
		MaxHealthChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSetBase->GetMaxHealthAttribute()).AddUObject(this, &AGASPlayerState::MaxHealthChanged);
		HealthRegenRateChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSetBase->GetHealthRegenRateAttribute()).AddUObject(this, &AGASPlayerState::HealthRegenRateChanged);
		ManaChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSetBase->GetManaAttribute()).AddUObject(this, &AGASPlayerState::ManaChanged);
		MaxManaChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSetBase->GetMaxManaAttribute()).AddUObject(this, &AGASPlayerState::MaxManaChanged);
		ManaRegenRateChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSetBase->GetManaRegenRateAttribute()).AddUObject(this, &AGASPlayerState::ManaRegenRateChanged);
		StaminaChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSetBase->GetStaminaAttribute()).AddUObject(this, &AGASPlayerState::StaminaChanged);
		MaxStaminaChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSetBase->GetMaxStaminaAttribute()).AddUObject(this, &AGASPlayerState::MaxStaminaChanged);
		StaminaRegenRateChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSetBase->GetStaminaRegenRateAttribute()).AddUObject(this, &AGASPlayerState::StaminaRegenRateChanged);
		XPChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSetBase->GetXPAttribute()).AddUObject(this, &AGASPlayerState::XPChanged);
		GoldChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSetBase->GetGoldAttribute()).AddUObject(this, &AGASPlayerState::GoldChanged);
		CharacterLevelChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSetBase->GetCharacterLevelAttribute()).AddUObject(this, &AGASPlayerState::CharacterLevelChanged);

		// Tag change callbacks
		AbilitySystemComponent->RegisterGameplayTagEvent(FGameplayTag::RequestGameplayTag(FName("State.Debuff.Stun")), EGameplayTagEventType::NewOrRemoved).AddUObject(this, &AGASPlayerState::StunTagChanged);
	}
}

void AGASPlayerState::HealthChanged(const FOnAttributeChangeData & Data)
{
	float Health = Data.NewValue;

	// Update floating status bar
	AGASHeroCharacter* Hero = Cast<AGASHeroCharacter>(GetPawn());
	if (Hero)
	{
		UGASFloatingStatusBarWidget* HeroFloatingStatusBar = Hero->GetFloatingStatusBar();
		if (HeroFloatingStatusBar)
		{
			HeroFloatingStatusBar->SetHealthPercentage(Health / GetMaxHealth());
		}
	}

	// Update the HUD
	// Handled in the UI itself using the AsyncTaskAttributeChanged node as an example how to do it in Blueprint

	// If the player died, handle death
	if (!IsAlive() && !AbilitySystemComponent->HasMatchingGameplayTag(DeadTag))
	{
		if (Hero)
		{
			Hero->Die();
		}
	}
}

void AGASPlayerState::MaxHealthChanged(const FOnAttributeChangeData & Data)
{
	float MaxHealth = Data.NewValue;

	// Update floating status bar
	AGASHeroCharacter* Hero = Cast<AGASHeroCharacter>(GetPawn());
	if (Hero)
	{
		UGASFloatingStatusBarWidget* HeroFloatingStatusBar = Hero->GetFloatingStatusBar();
		if (HeroFloatingStatusBar)
		{
			HeroFloatingStatusBar->SetHealthPercentage(GetHealth() / MaxHealth);
		}
	}

	// Update the HUD
	AGASPlayerController* PC = Cast<AGASPlayerController>(GetOwner());
	if (PC)
	{
		UGASHUDWidget* HUD = PC->GetHUD();
		if (HUD)
		{
			HUD->SetMaxHealth(MaxHealth);
		}
	}
}

void AGASPlayerState::HealthRegenRateChanged(const FOnAttributeChangeData & Data)
{
	float HealthRegenRate = Data.NewValue;

	// Update the HUD
	AGASPlayerController* PC = Cast<AGASPlayerController>(GetOwner());
	if (PC)
	{
		UGASHUDWidget* HUD = PC->GetHUD();
		if (HUD)
		{
			HUD->SetHealthRegenRate(HealthRegenRate);
		}
	}
}

void AGASPlayerState::ManaChanged(const FOnAttributeChangeData & Data)
{
	float Mana = Data.NewValue;

	// Update floating status bar
	AGASHeroCharacter* Hero = Cast<AGASHeroCharacter>(GetPawn());
	if (Hero)
	{
		UGASFloatingStatusBarWidget* HeroFloatingStatusBar = Hero->GetFloatingStatusBar();
		if (HeroFloatingStatusBar)
		{
			HeroFloatingStatusBar->SetManaPercentage(Mana / GetMaxMana());
		}
	}

	// Update the HUD
	// Handled in the UI itself using the AsyncTaskAttributeChanged node as an example how to do it in Blueprint
}

void AGASPlayerState::MaxManaChanged(const FOnAttributeChangeData & Data)
{
	float MaxMana = Data.NewValue;

	// Update floating status bar
	AGASHeroCharacter* Hero = Cast<AGASHeroCharacter>(GetPawn());
	if (Hero)
	{
		UGASFloatingStatusBarWidget* HeroFloatingStatusBar = Hero->GetFloatingStatusBar();
		if (HeroFloatingStatusBar)
		{
			HeroFloatingStatusBar->SetManaPercentage(GetMana() / MaxMana);
		}
	}

	// Update the HUD
	AGASPlayerController* PC = Cast<AGASPlayerController>(GetOwner());
	if (PC)
	{
		UGASHUDWidget* HUD = PC->GetHUD();
		if (HUD)
		{
			HUD->SetMaxMana(MaxMana);
		}
	}
}

void AGASPlayerState::ManaRegenRateChanged(const FOnAttributeChangeData & Data)
{
	float ManaRegenRate = Data.NewValue;

	// Update the HUD
	AGASPlayerController* PC = Cast<AGASPlayerController>(GetOwner());
	if (PC)
	{
		UGASHUDWidget* HUD = PC->GetHUD();
		if (HUD)
		{
			HUD->SetManaRegenRate(ManaRegenRate);
		}
	}
}

void AGASPlayerState::StaminaChanged(const FOnAttributeChangeData & Data)
{
	float Stamina = Data.NewValue;

	// Update the HUD
	// Handled in the UI itself using the AsyncTaskAttributeChanged node as an example how to do it in Blueprint
}

void AGASPlayerState::MaxStaminaChanged(const FOnAttributeChangeData & Data)
{
	float MaxStamina = Data.NewValue;

	// Update the HUD
	AGASPlayerController* PC = Cast<AGASPlayerController>(GetOwner());
	if (PC)
	{
		UGASHUDWidget* HUD = PC->GetHUD();
		if (HUD)
		{
			HUD->SetMaxStamina(MaxStamina);
		}
	}
}

void AGASPlayerState::StaminaRegenRateChanged(const FOnAttributeChangeData & Data)
{
	float StaminaRegenRate = Data.NewValue;

	// Update the HUD
	AGASPlayerController* PC = Cast<AGASPlayerController>(GetOwner());
	if (PC)
	{
		UGASHUDWidget* HUD = PC->GetHUD();
		if (HUD)
		{
			HUD->SetStaminaRegenRate(StaminaRegenRate);
		}
	}
}

void AGASPlayerState::XPChanged(const FOnAttributeChangeData & Data)
{
	float XP = Data.NewValue;

	// Update the HUD
	AGASPlayerController* PC = Cast<AGASPlayerController>(GetOwner());
	if (PC)
	{
		UGASHUDWidget* HUD = PC->GetHUD();
		if (HUD)
		{
			HUD->SetExperience(XP);
		}
	}
}

void AGASPlayerState::GoldChanged(const FOnAttributeChangeData & Data)
{
	float Gold = Data.NewValue;

	// Update the HUD
	AGASPlayerController* PC = Cast<AGASPlayerController>(GetOwner());
	if (PC)
	{
		UGASHUDWidget* HUD = PC->GetHUD();
		if (HUD)
		{
			HUD->SetGold(Gold);
		}
	}
}

void AGASPlayerState::CharacterLevelChanged(const FOnAttributeChangeData & Data)
{
	float CharacterLevel = Data.NewValue;

	// Update the HUD
	AGASPlayerController* PC = Cast<AGASPlayerController>(GetOwner());
	if (PC)
	{
		UGASHUDWidget* HUD = PC->GetHUD();
		if (HUD)
		{
			HUD->SetHeroLevel(CharacterLevel);
		}
	}
}

void AGASPlayerState::StunTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	if (NewCount > 0)
	{
		FGameplayTagContainer AbilityTagsToCancel;
		AbilityTagsToCancel.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability")));

		FGameplayTagContainer AbilityTagsToIgnore;
		AbilityTagsToIgnore.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.NotCanceledByStun")));

		AbilitySystemComponent->CancelAbilities(&AbilityTagsToCancel, &AbilityTagsToIgnore);
	}
}
