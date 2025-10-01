#pragma once
#include "PrimitiveComponent.h"

class UMeshComponent : public UPrimitiveComponent
{
public:
    DECLARE_CLASS(UMeshComponent, UPrimitiveComponent)
    UMeshComponent();

protected:
    ~UMeshComponent() override;
    virtual void RenderDetail() override;

public:


protected:

};