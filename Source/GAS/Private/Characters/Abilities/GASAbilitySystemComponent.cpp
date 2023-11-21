// Copyright 2020 Dan Kestranek.


#include "Characters/Abilities/GASAbilitySystemComponent.h"

void Agas::ReceiveDamage(Agas * SourceASC, float UnmitigatedDamage, float MitigatedDamage)
{
	ReceivedDamage.Broadcast(SourceASC, UnmitigatedDamage, MitigatedDamage);
}
