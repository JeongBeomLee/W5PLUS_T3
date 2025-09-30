#pragma once
#include "Object.h"

class AActor;

class ULevel : public UObject
{
public:
    DECLARE_CLASS(ULevel, UObject)
    ULevel();

protected:
    ~ULevel() override;

public:
    void AddActor(AActor* Actor);
    void RemoveActor(AActor* Actor);

    const TArray<AActor*>& GetActors() const { return Actors; }
    TArray<AActor*>& GetActors() { return Actors; }

protected:
    TArray<AActor*> Actors;
};