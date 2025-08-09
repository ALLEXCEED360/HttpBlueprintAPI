#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Http.h"
#include "HttpBlueprintFunctionLibrary.generated.h"

/**
 * Blueprint delegate that gets called when an HTTP request completes
 * This is the "callback function" that Blueprints can hook into
 *
 * Parameters:
 * - bWasSuccessful: True if request completed successfully
 * - ResponseCode: HTTP status code (200, 404, 500, etc.)
 * - ResponseBody: The actual content returned from the server
 * - ErrorMessage: Description of any error that occurred
 */
DECLARE_DYNAMIC_DELEGATE_FourParams(
    FOnHttpResponseReceived,
    bool, bWasSuccessful,
    int32, ResponseCode,
    FString, ResponseBody,
    FString, ErrorMessage
);

/**
 * Structure to hold HTTP response data in a Blueprint-friendly format
 */
USTRUCT(BlueprintType)
struct HTTPBLUEPRINTAPI_API FHttpResponseData
{
    GENERATED_BODY()

    /** Whether the request was successful */
    UPROPERTY(BlueprintReadOnly, Category = "HTTP Response")
    bool bWasSuccessful = false;

    /** HTTP response code (200 = OK, 404 = Not Found, etc.) */
    UPROPERTY(BlueprintReadOnly, Category = "HTTP Response")
    int32 ResponseCode = 0;

    /** The response content as a string */
    UPROPERTY(BlueprintReadOnly, Category = "HTTP Response")
    FString ResponseBody;

    /** Error message if the request failed */
    UPROPERTY(BlueprintReadOnly, Category = "HTTP Response")
    FString ErrorMessage;

    /** How long the request took to complete (in seconds) */
    UPROPERTY(BlueprintReadOnly, Category = "HTTP Response")
    float ResponseTimeSeconds = 0.0f;

    /** HTTP response headers */
    UPROPERTY(BlueprintReadOnly, Category = "HTTP Response")
    TMap<FString, FString> ResponseHeaders;
};

/**
 * HTTP Blueprint Function Library with delegate support
 * Provides functions to make HTTP requests and return results via Blueprint delegates
 */
UCLASS()
class HTTPBLUEPRINTAPI_API UHttpBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    /**
     * Make an HTTP GET request with a Blueprint callback delegate
     *
     * This function starts an HTTP request and immediately returns.
     * When the response arrives (could be seconds later), your delegate function will be called.
     *
     * @param URL - The web address to request from (e.g., "https://api.example.com/data")
     * @param OnResponseReceived - Blueprint delegate/event that gets called when response arrives
     * @param WorldContextObject - Reference to the game world (usually pass 'self')
     */
    UFUNCTION(BlueprintCallable, Category = "HTTP",
        Meta = (DisplayName = "Make HTTP GET Request",
            CallInEditor = true,
            Keywords = "http get request api web"))
    static void MakeHttpGetRequest(
        const FString& URL,
        const FOnHttpResponseReceived& OnResponseReceived,
        UObject* WorldContextObject = nullptr
    );

    /**
     * Make an HTTP POST request with request body and callback delegate
     *
     * @param URL - The web address to post to
     * @param RequestBody - Data to send in the POST body (usually JSON)
     * @param ContentType - Content type header (e.g., "application/json")
     * @param OnResponseReceived - Blueprint delegate that gets called when response arrives
     * @param WorldContextObject - Reference to the game world
     */
    UFUNCTION(BlueprintCallable, Category = "HTTP",
        Meta = (DisplayName = "Make HTTP POST Request",
            CallInEditor = true,
            Keywords = "http post request api web json"))
    static void MakeHttpPostRequest(
        const FString& URL,
        const FString& RequestBody,
        const FString& ContentType,
        const FOnHttpResponseReceived& OnResponseReceived,
        UObject* WorldContextObject = nullptr
    );

    /**
     * Make an HTTP request with custom headers
     *
     * @param URL - The web address to request from
     * @param Method - HTTP method (GET, POST, PUT, DELETE)
     * @param RequestBody - Data to send (empty for GET requests)
     * @param Headers - Custom headers to include with the request
     * @param OnResponseReceived - Blueprint delegate that gets called when response arrives
     * @param WorldContextObject - Reference to the game world
     */
    UFUNCTION(BlueprintCallable, Category = "HTTP",
        Meta = (DisplayName = "Make HTTP Request with Headers",
            CallInEditor = true,
            Keywords = "http request api web headers auth"))
    static void MakeHttpRequestWithHeaders(
        const FString& URL,
        const FString& Method,
        const FString& RequestBody,
        const TMap<FString, FString>& Headers,
        const FOnHttpResponseReceived& OnResponseReceived,
        UObject* WorldContextObject = nullptr
    );

    // =============================================================================
    // UTILITY FUNCTIONS
    // =============================================================================

    /**
     * Check if an HTTP response code indicates success
     * HTTP codes 200-299 are considered successful
     *
     * @param ResponseCode - The HTTP response code to check
     * @return True if the response code indicates success
     */
    UFUNCTION(BlueprintCallable, Category = "HTTP|Utilities", BlueprintPure,
        Meta = (DisplayName = "Is HTTP Response Successful"))
    static bool IsHttpResponseSuccessful(int32 ResponseCode);

    /**
     * Get a human-readable description of an HTTP response code
     *
     * @param ResponseCode - The HTTP response code
     * @return Description of the response code (e.g., "OK", "Not Found", etc.)
     */
    UFUNCTION(BlueprintCallable, Category = "HTTP|Utilities", BlueprintPure,
        Meta = (DisplayName = "Get HTTP Response Description"))
    static FString GetHttpResponseCodeDescription(int32 ResponseCode);

    /**
     * Parse a URL to extract the base domain
     *
     * @param URL - Full URL to parse
     * @return The domain part of the URL
     */
    UFUNCTION(BlueprintCallable, Category = "HTTP|Utilities", BlueprintPure,
        Meta = (DisplayName = "Get Domain from URL"))
    static FString GetDomainFromURL(const FString& URL);

    /**
     * Validate if a string is a proper URL format
     *
     * @param URL - String to validate
     * @return True if the string appears to be a valid URL
     */
    UFUNCTION(BlueprintCallable, Category = "HTTP|Utilities", BlueprintPure,
        Meta = (DisplayName = "Is Valid URL"))
    static bool IsValidURL(const FString& URL);

private:

    // =============================================================================
    // INTERNAL CALLBACK HANDLERS
    // These functions handle the raw HTTP responses and convert them to Blueprint format
    // =============================================================================

    /**
     * Internal callback that gets called when HTTP request completes
     * This processes the raw HTTP response and calls the user's Blueprint delegate
     */
    static void OnHttpRequestComplete(
        FHttpRequestPtr Request,
        FHttpResponsePtr Response,
        bool bWasSuccessful,
        FOnHttpResponseReceived UserCallback
    );

    /**
     * Helper function to process HTTP response into our Blueprint-friendly structure
     */
    static FHttpResponseData ProcessHttpResponse(
        FHttpRequestPtr Request,
        FHttpResponsePtr Response,
        bool bWasSuccessful
    );

    /**
     * Helper function to create and configure an HTTP request
     */
    static TSharedRef<IHttpRequest, ESPMode::ThreadSafe> CreateHttpRequest(
        const FString& URL,
        const FString& Method,
        const FString& RequestBody,
        const TMap<FString, FString>& Headers
    );

    /**
     * Validate request parameters before sending
     */
    static bool ValidateHttpRequest(
        const FString& URL,
        const FString& Method,
        FString& OutErrorMessage
    );
};