// Copyright 2020 Dan Kestranek.


#include "Characters/Abilities/GASAbilitySystemComponent.h"

void UGASAbilitySystemComponent::ReceiveDamage(UGASAbilitySystemComponent * SourceASC, float UnmitigatedDamage, float MitigatedDamage)
{
	ReceivedDamage.Broadcast(SourceASC, UnmitigatedDamage, MitigatedDamage);
}
