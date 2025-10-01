#include "pch.h"

FString UObject::GetName()
{
    return ObjectName.ToString();
}

FString UObject::GetComparisonName()
{
    return FString();
}


const FString UObject::GetName() const
{
    return ObjectName.ToString();
}