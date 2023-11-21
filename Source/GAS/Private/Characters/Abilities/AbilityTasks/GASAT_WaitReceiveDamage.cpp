// Copyright 2020 Dan Kestranek.


#include "..\..\..\..\Public\Characters\Abilities\AbilityTasks\GASAT_WaitReceiveDamage.h"
#include "Characters/Abilities/GASAbilitySystemComponent.h"

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
	UGASAbilitySystemComponent* GASASC = Cast<UGASAbilitySystemComponent>(AbilitySystemComponent);

	if (GASASC)
	{
		GASASC->ReceivedDamage.AddDynamic(this, &UGASAT_WaitReceiveDamage::OnDamageReceived);
	}
}

void UGASAT_WaitReceiveDamage::OnDestroy(bool AbilityIsEnding)
{
	UGASAbilitySystemComponent* GASASC = Cast<UGASAbilitySystemComponent>(AbilitySystemComponent);

	if (GASASC)
	{
		GASASC->ReceivedDamage.RemoveDynamic(this, &UGASAT_WaitReceiveDamage::OnDamageReceived);
	}

	Super::OnDestroy(AbilityIsEnding);
}

void UGASAT_WaitReceiveDamage::OnDamageReceived(UGASAbilitySystemComponent* SourceASC, float UnmitigatedDamage, float MitigatedDamage)
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
