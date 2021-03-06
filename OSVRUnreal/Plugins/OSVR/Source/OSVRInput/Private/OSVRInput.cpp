// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "OSVRInputPrivatePCH.h"
#include "IOSVR.h"
#include "IOSVRInput.h"
#include "OSVREntryPoint.h"
#include "OSVRInputDevice.h"

#include "InputCoreTypes.h"
#include "GameFramework/InputSettings.h"

class FOSVRInput : public IOSVRInput
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	virtual TSharedPtr< class IInputDevice > CreateInputDevice(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler) override;

	TSharedPtr< class FOSVRInputDevice > InputDevice;
};

IMPLEMENT_MODULE(FOSVRInput, OSVRInput)

TSharedPtr< class IInputDevice > FOSVRInput::CreateInputDevice(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler)
{
    if (IOSVR::Get().GetEntryPoint()->IsOSVRConnected()) {
        FOSVRInputDevice::RegisterNewKeys();

        InputDevice = MakeShareable(new FOSVRInputDevice(InMessageHandler));
        return InputDevice;
    }
    return nullptr;
}

void FOSVRInput::StartupModule()
{
	IInputDeviceModule::StartupModule();
}

void FOSVRInput::ShutdownModule()
{
	IInputDeviceModule::ShutdownModule();
}
