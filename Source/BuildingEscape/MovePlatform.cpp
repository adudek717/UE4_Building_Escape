// Copyright Aleksander Dudek 2021


#include "MovePlatform.h"
#include "Components/AudioComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

#define OUT

// Sets default values for this component's properties
UMovePlatform::UMovePlatform()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts
void UMovePlatform::BeginPlay()
{
	Super::BeginPlay();

	YInitialPosition = GetOwner()->GetActorLocation().Y;
	YCurrentPosition = YInitialPosition;
	YMoveBy += YInitialPosition;

	FindPressurePlate();
	FindAudioComponent();
}

void UMovePlatform::FindPressurePlate() const
{
	if (!PressurePlate) {
		UE_LOG(LogTemp, Error, TEXT("%s has the open door component on it, but no pressureplate set!"), *GetOwner()->GetName());
	}
}

void UMovePlatform::FindAudioComponent()
{
	AudioComponent = GetOwner()->FindComponentByClass<UAudioComponent>();
	if (!AudioComponent) {
		UE_LOG(LogTemp, Error, TEXT("No UAudioComponent found on object: %s"), *GetOwner()->GetName());
	}
}

// Called every frame
void UMovePlatform::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (TotalMassOfActors() > MassToOpenDoors) {
		OpenDoor(DeltaTime);
		DoorLastOpened = GetWorld()->GetTimeSeconds();
	}
	else if (PressurePlate) {
		if (GetWorld()->GetTimeSeconds() > DoorLastOpened + DoorCloseDelay) {
			CloseDoor(DeltaTime);
		}
	}
	// Use FInterpConstantTo instead of Lerp to make the door close in linear time
	// OpenDoor.Yaw = FMath::FInterpConstantTo(CurrentYaw, OpenAngle, DeltaTime, 45.f);
}

void UMovePlatform::OpenDoor(float DeltaTime)
{

	// DEBUG CODE
	// UE_LOG(LogTemp, Warning, TEXT("%s is the door rotation"), *GetOwner()->GetActorRotation().ToString()); 
	// UE_LOG(LogTemp, Warning, TEXT("%f is the door YAW(Z) value"), GetOwner()->GetActorRotation().Yaw);
	YCurrentPosition = FMath::Lerp(YCurrentPosition, YMoveBy, DeltaTime * DoorOpenSpeed);
	FVector FloorPos = GetOwner()->GetActorLocation();
	FloorPos.Y = YCurrentPosition;
	GetOwner()->SetActorLocation(FloorPos);

	CloseDoorSound = false;
	if (!AudioComponent) { return; }
	if (!OpenDoorSound) {
		AudioComponent->Play();
		OpenDoorSound = true;
	}
}

void UMovePlatform::CloseDoor(float DeltaTime)
{
	YCurrentPosition = FMath::Lerp(YCurrentPosition, YInitialPosition, DeltaTime * DoorCloseSpeed);
	FVector FloorPos = GetOwner()->GetActorLocation();
	FloorPos.Y = YCurrentPosition;
	GetOwner()->SetActorLocation(FloorPos);

	OpenDoorSound = false;
	if (!AudioComponent) { return; }
	if (!CloseDoorSound) {
		AudioComponent->Play();
		CloseDoorSound = true;
	}
}

float UMovePlatform::TotalMassOfActors() const
{
	float TotalMass = 0.f;

	// Find all overlapping actors
	TArray<AActor*> OverlappingActors{ nullptr };
	if (!PressurePlate) { return TotalMass; }
	PressurePlate->GetOverlappingActors(OUT OverlappingActors);

	// Add up their masses
	for (AActor* Actor : OverlappingActors)
	{
		TotalMass += Actor->FindComponentByClass<UPrimitiveComponent>()->GetMass();
	}
	//UE_LOG(LogTemp, Warning, TEXT("Total mass of overlapping components: %f"), TotalMass);

	return TotalMass;
}