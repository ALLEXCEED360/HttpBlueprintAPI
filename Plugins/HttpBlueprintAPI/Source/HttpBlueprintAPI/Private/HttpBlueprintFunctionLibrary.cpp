#include "HttpBlueprintFunctionLibrary.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Misc/Paths.h"
#include "Engine/Engine.h"

void UHttpBlueprintFunctionLibrary::MakeAPIRequest(const FString& URL, UObject* WorldContextObject)
{
    if (URL.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("API Request failed: Empty URL"));
        return;
    }

    FHttpModule* Http = &FHttpModule::Get();
    if (!Http) return;

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http->CreateRequest();
    Request->SetURL(URL);
    Request->SetVerb(TEXT("GET"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->OnProcessRequestComplete().BindStatic(&UHttpBlueprintFunctionLibrary::OnResponseReceived);
    Request->ProcessRequest();
}

void UHttpBlueprintFunctionLibrary::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (!bWasSuccessful || !Response.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("API Request failed"));
        return;
    }

    FString ResponseString = Response->GetContentAsString();
    UE_LOG(LogTemp, Log, TEXT("API Response: %s"), *ResponseString);

    // Print to screen for demo
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, ResponseString);
    }
}
