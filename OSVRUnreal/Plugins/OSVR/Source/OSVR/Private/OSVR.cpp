// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "OSVRPrivatePCH.h"
#include "InputCoreTypes.h"
#include "GameFramework/InputSettings.h"

#include "OSVREntryPoint.h"

#include "OSVRHMD.h"

DEFINE_LOG_CATEGORY(OSVRLog);

class FOSVR : public IOSVR
{
private:
    TSharedPtr<FOSVRHMD, ESPMode::ThreadSafe> hmd;
    FCriticalSection mModuleMutex;
    TSharedPtr< class OSVREntryPoint > EntryPoint;
    bool mModulesLoaded = false;
public:
    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    /** IHeadMountedDisplayModule implementation */
    virtual TSharedPtr< class IHeadMountedDisplay, ESPMode::ThreadSafe > CreateHeadMountedDisplay() override;

    // Pre-init the HMD module (optional).
    //virtual void PreInit() override;

    virtual OSVREntryPoint* GetEntryPoint() override;
    virtual TSharedPtr<FOSVRHMD, ESPMode::ThreadSafe> GetHMD() override;
    virtual void LoadOSVRClientKitModule() override;
};

IMPLEMENT_MODULE(FOSVR, OSVR)

OSVREntryPoint* FOSVR::GetEntryPoint()
{
    return EntryPoint.Get();
}

TSharedPtr<FOSVRHMD, ESPMode::ThreadSafe> FOSVR::GetHMD()
{
    return hmd;
}

void FOSVR::LoadOSVRClientKitModule()
{
    FScopeLock lock(&mModuleMutex);
    if (!mModulesLoaded) {
#if PLATFORM_WINDOWS
        const std::vector<std::string> osvrDlls = {
            "osvrClientKit.dll",
            "osvrClient.dll",
            "osvrCommon.dll",
            "osvrUtil.dll",
            "osvrRenderManager.dll",
            "d3dcompiler_47.dll",
            "glew32.dll",
            "SDL2.dll"
        };
#if PLATFORM_64BITS
        std::vector<FString> pathsToTry = {
            FPaths::GamePluginsDir() / "OSVR/Source/OSVRClientKit/bin/Win64/",
            FPaths::EngineDir() / "Plugins/Runtime/OSVR/Source/OSVRClientKit/bin/Win64/",
            FPaths::EngineDir() / "Binaries/ThirdParty/OSVRClientKit/bin/Win64/",
            FPaths::EngineDir() / "Source/ThirdParty/OSVRClientKit/bin/Win64/"
        };

#else
        std::vector<FString> pathsToTry = {
            FPaths::GamePluginsDir() / "OSVR/Source/OSVRClientKit/bin/Win32/",
            FPaths::EngineDir() / "Plugins/Runtime/OSVR/Source/OSVRClientKit/bin/Win32/",
            FPaths::EngineDir() / "Binaries/ThirdParty/OSVRClientKit/bin/Win32/",
            FPaths::EngineDir() / "Source/ThirdParty/OSVRClientKit/bin/Win32/"
        };
#endif

        FString osvrClientKitLibPath;
        for (size_t i = 0; i < pathsToTry.size(); i++) {
            if (FPaths::DirectoryExists(pathsToTry[i])) {
                osvrClientKitLibPath = pathsToTry[i];
                break;
            }
        }
        if (osvrClientKitLibPath.Len() == 0) {
            UE_LOG(OSVRLog, Warning, TEXT("Could not find OSVRClientKit module binaries in either the engine plugins or game plugins folder."));
            return;
        }
        FPlatformProcess::PushDllDirectory(*osvrClientKitLibPath);
        for (size_t i = 0; i < osvrDlls.size(); i++) {
            void* libHandle = nullptr;
            auto path = osvrClientKitLibPath + osvrDlls[i].c_str();
            libHandle = FPlatformProcess::GetDllHandle(*path);
            if (!libHandle) {
                UE_LOG(OSVRLog, Warning, TEXT("FAILED to load %s"), *path);
            }
        }
        FPlatformProcess::PopDllDirectory(*osvrClientKitLibPath);
#endif
        mModulesLoaded = true;
    }
}

TSharedPtr< class IHeadMountedDisplay, ESPMode::ThreadSafe > FOSVR::CreateHeadMountedDisplay()
{
    if (EntryPoint->IsOSVRConnected()) {
        TSharedPtr< FOSVRHMD, ESPMode::ThreadSafe > OSVRHMD(new FOSVRHMD());
        if (OSVRHMD->IsInitialized() && OSVRHMD->IsHMDConnected())
        {
            hmd = OSVRHMD;
            return OSVRHMD;
        }
    }
    return nullptr;
}

void FOSVR::StartupModule()
{
    LoadOSVRClientKitModule();
    IHeadMountedDisplayModule::StartupModule();

    EntryPoint = MakeShareable(new OSVREntryPoint());
}

void FOSVR::ShutdownModule()
{
    EntryPoint = nullptr;

    IHeadMountedDisplayModule::ShutdownModule();
}
