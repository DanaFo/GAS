// Copyright 2020 Dan Kestranek.


#include "..\..\..\..\Public\Characters\Abilities\AbilityTasks\GASAT_WaitReceiveDamage.h"
#include "Characters/Abilities/GDAbilitySystemComponent.h"

UGASAT_WaitReceiveDamage::UGASAT_WaitReceiveDamage(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TriggerOnce = false;
}

UGASAT_WaitReceiveDamage* UGASAT_WaitReceiveDamage::WaitReceiveDamage(UGameplayAbility* OwningAbility, bool InTriggerOnce)
{
	UGASAT_WaitReceiveDamage* MyObj = NewAbilityTask<UGASAT_WaitReceiveDamage>(OwningAbility);
	MyObj->TriggerOnce = InTriggerOnce;
	return MyObj;
}

void UGASAT_WaitReceiveDamage::Activate()
{
	UGDAbilitySystemComponent* GDASC = Cast<UGDAbilitySystemComponent>(AbilitySystemComponent);

	if (GDASC)
	{
		GDASC->ReceivedDamage.AddDynamic(this, &UGASAT_WaitReceiveDamage::OnDamageReceived);
	}
}

void UGASAT_WaitReceiveDamage::OnDestroy(bool AbilityIsEnding)
{
	UGDAbilitySystemComponent* GDASC = Cast<UGDAbilitySystemComponent>(AbilitySystemComponent);

	if (GDASC)
	{
		GDASC->ReceivedDamage.RemoveDynamic(this, &UGASAT_WaitReceiveDamage::OnDamageReceived);
	}

	Super::OnDestroy(AbilityIsEnding);
}

void UGASAT_WaitReceiveDamage::OnDamageReceived(UGDAbilitySystemComponent* SourceASC, float UnmitigatedDamage, float MitigatedDamage)
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnDamage.Broadcast(SourceASC, UnmitigatedDamage, MitigatedDamage);
	}

	if (TriggerOnce)
	{
		EndTask();
	}
}
