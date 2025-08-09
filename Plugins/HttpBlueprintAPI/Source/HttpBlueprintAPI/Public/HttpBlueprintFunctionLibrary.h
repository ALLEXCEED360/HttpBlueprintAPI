#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Http.h"
#include "HttpBlueprintFunctionLibrary.generated.h"

/**
 * HTTP Blueprint API Library
 * Provides functions to make HTTP GET requests and return results to Blueprints.
 */
UCLASS()
class HTTPBLUEPRINTAPI_API UHttpBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    /**
     * Make a GET request to the given URL.
     * @param URL - The API endpoint URL.
     */
    UFUNCTION(BlueprintCallable, Category = "HTTP")
    static void MakeAPIRequest(const FString& URL, UObject* WorldContextObject);

private:
    static void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
};
