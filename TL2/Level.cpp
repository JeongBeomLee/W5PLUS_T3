#include "pch.h"
#include "Level.h"
#include "Actor.h"

ULevel::ULevel()
{
}

ULevel::~ULevel()
{
    Actors.clear();
}

void ULevel::AddActor(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }
    Actors.Add(Actor);
}

void ULevel::RemoveActor(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }

    auto it = std::find(Actors.begin(), Actors.end(), Actor);
    if (it != Actors.end())
    {
        Actors.erase(it);
    }
}