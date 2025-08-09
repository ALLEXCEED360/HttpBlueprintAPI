#include "HttpBlueprintFunctionLibrary.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/DateTime.h"

// Define logging category
DEFINE_LOG_CATEGORY_STATIC(LogHttpBlueprintAPI, Log, All);

// =============================================================================
// PUBLIC HTTP REQUEST FUNCTIONS
// =============================================================================

void UHttpBlueprintFunctionLibrary::MakeHttpGetRequest(
    const FString& URL,
    const FOnHttpResponseReceived& OnResponseReceived,
    UObject* WorldContextObject)
{
    // Use the generic request function with GET method and no body
    TMap<FString, FString> DefaultHeaders;
    DefaultHeaders.Add(TEXT("Content-Type"), TEXT("application/json"));

    MakeHttpRequestWithHeaders(
        URL,
        TEXT("GET"),
        TEXT(""), // Empty body for GET requests
        DefaultHeaders,
        OnResponseReceived,
        WorldContextObject
    );
}

void UHttpBlueprintFunctionLibrary::MakeHttpPostRequest(
    const FString& URL,
    const FString& RequestBody,
    const FString& ContentType,
    const FOnHttpResponseReceived& OnResponseReceived,
    UObject* WorldContextObject)
{
    // Set up headers with the specified content type
    TMap<FString, FString> Headers;
    Headers.Add(TEXT("Content-Type"), ContentType.IsEmpty() ? TEXT("application/json") : ContentType);

    MakeHttpRequestWithHeaders(
        URL,
        TEXT("POST"),
        RequestBody,
        Headers,
        OnResponseReceived,
        WorldContextObject
    );
}

void UHttpBlueprintFunctionLibrary::MakeHttpRequestWithHeaders(
    const FString& URL,
    const FString& Method,
    const FString& RequestBody,
    const TMap<FString, FString>& Headers,
    const FOnHttpResponseReceived& OnResponseReceived,
    UObject* WorldContextObject)
{
    // Validate input parameters
    FString ErrorMessage;
    if (!ValidateHttpRequest(URL, Method, ErrorMessage))
    {
        UE_LOG(LogHttpBlueprintAPI, Error, TEXT("HTTP Request validation failed: %s"), *ErrorMessage);

        // Call delegate immediately with error
        if (OnResponseReceived.IsBound())
        {
            // Execute on game thread to ensure Blueprint safety
            AsyncTask(ENamedThreads::GameThread, [OnResponseReceived, ErrorMessage]()
                {
                    OnResponseReceived.ExecuteIfBound(
                        false,          // bWasSuccessful
                        0,              // ResponseCode
                        TEXT(""),       // ResponseBody
                        ErrorMessage    // ErrorMessage
                    );
                });
        }
        return;
    }

    // Check if HTTP module is available
    FHttpModule* Http = &FHttpModule::Get();
    if (!Http)
    {
        UE_LOG(LogHttpBlueprintAPI, Error, TEXT("HTTP module not available"));

        if (OnResponseReceived.IsBound())
        {
            AsyncTask(ENamedThreads::GameThread, [OnResponseReceived]()
                {
                    OnResponseReceived.ExecuteIfBound(
                        false, 0, TEXT(""), TEXT("HTTP module not available")
                    );
                });
        }
        return;
    }

    // Create and configure the HTTP request
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = CreateHttpRequest(URL, Method, RequestBody, Headers);

    // Set up the completion callback
    // This is the key part - when the request completes, our internal callback will be called,
    // which will then call the user's Blueprint delegate
    Request->OnProcessRequestComplete().BindStatic(
        &UHttpBlueprintFunctionLibrary::OnHttpRequestComplete,
        OnResponseReceived
    );

    // Log the request details
    UE_LOG(LogHttpBlueprintAPI, Log, TEXT("Starting HTTP %s request to: %s"), *Method, *URL);
    if (!RequestBody.IsEmpty())
    {
        UE_LOG(LogHttpBlueprintAPI, Verbose, TEXT("Request body: %s"), *RequestBody);
    }

    // Start the HTTP request
    bool bRequestStarted = Request->ProcessRequest();
    if (!bRequestStarted)
    {
        UE_LOG(LogHttpBlueprintAPI, Error, TEXT("Failed to start HTTP request"));

        if (OnResponseReceived.IsBound())
        {
            AsyncTask(ENamedThreads::GameThread, [OnResponseReceived]()
                {
                    OnResponseReceived.ExecuteIfBound(
                        false, 0, TEXT(""), TEXT("Failed to start HTTP request")
                    );
                });
        }
    }
}

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

bool UHttpBlueprintFunctionLibrary::IsHttpResponseSuccessful(int32 ResponseCode)
{
    // HTTP status codes 200-299 indicate success
    return ResponseCode >= 200 && ResponseCode < 300;
}

FString UHttpBlueprintFunctionLibrary::GetHttpResponseCodeDescription(int32 ResponseCode)
{
    // Return human-readable descriptions for common HTTP status codes
    switch (ResponseCode)
    {
        // 2xx Success
    case 200: return TEXT("OK");
    case 201: return TEXT("Created");
    case 202: return TEXT("Accepted");
    case 204: return TEXT("No Content");

        // 3xx Redirection
    case 301: return TEXT("Moved Permanently");
    case 302: return TEXT("Found");
    case 304: return TEXT("Not Modified");

        // 4xx Client Error
    case 400: return TEXT("Bad Request");
    case 401: return TEXT("Unauthorized");
    case 403: return TEXT("Forbidden");
    case 404: return TEXT("Not Found");
    case 405: return TEXT("Method Not Allowed");
    case 408: return TEXT("Request Timeout");
    case 409: return TEXT("Conflict");
    case 422: return TEXT("Unprocessable Entity");
    case 429: return TEXT("Too Many Requests");

        // 5xx Server Error
    case 500: return TEXT("Internal Server Error");
    case 501: return TEXT("Not Implemented");
    case 502: return TEXT("Bad Gateway");
    case 503: return TEXT("Service Unavailable");
    case 504: return TEXT("Gateway Timeout");

        // Default case
    default: return FString::Printf(TEXT("HTTP %d"), ResponseCode);
    }
}

FString UHttpBlueprintFunctionLibrary::GetDomainFromURL(const FString& URL)
{
    FString Domain = URL;

    // Remove protocol (http:// or https://)
    if (Domain.StartsWith(TEXT("https://")))
    {
        Domain = Domain.RightChop(8);
    }
    else if (Domain.StartsWith(TEXT("http://")))
    {
        Domain = Domain.RightChop(7);
    }

    // Find the first slash or question mark and cut there
    int32 SlashIndex = Domain.Find(TEXT("/"));
    int32 QueryIndex = Domain.Find(TEXT("?"));

    int32 CutIndex = -1;
    if (SlashIndex >= 0 && QueryIndex >= 0)
    {
        CutIndex = FMath::Min(SlashIndex, QueryIndex);
    }
    else if (SlashIndex >= 0)
    {
        CutIndex = SlashIndex;
    }
    else if (QueryIndex >= 0)
    {
        CutIndex = QueryIndex;
    }

    if (CutIndex >= 0)
    {
        Domain = Domain.Left(CutIndex);
    }

    return Domain;
}

bool UHttpBlueprintFunctionLibrary::IsValidURL(const FString& URL)
{
    // Basic URL validation
    if (URL.IsEmpty())
    {
        return false;
    }

    // Must start with http:// or https://
    if (!URL.StartsWith(TEXT("http://")) && !URL.StartsWith(TEXT("https://")))
    {
        return false;
    }

    // Must have something after the protocol
    FString WithoutProtocol = URL.StartsWith(TEXT("https://")) ?
        URL.RightChop(8) : URL.RightChop(7);

    if (WithoutProtocol.IsEmpty())
    {
        return false;
    }

    // Basic check for invalid characters
    if (WithoutProtocol.Contains(TEXT(" ")) || WithoutProtocol.Contains(TEXT("<")) ||
        WithoutProtocol.Contains(TEXT(">")))
    {
        return false;
    }

    return true;
}

// =============================================================================
// PRIVATE HELPER FUNCTIONS
// =============================================================================

void UHttpBlueprintFunctionLibrary::OnHttpRequestComplete(
    FHttpRequestPtr Request,
    FHttpResponsePtr Response,
    bool bWasSuccessful,
    FOnHttpResponseReceived UserCallback)
{
    // Process the HTTP response into our Blueprint-friendly format
    FHttpResponseData ResponseData = ProcessHttpResponse(Request, Response, bWasSuccessful);

    // Log the response details
    UE_LOG(LogHttpBlueprintAPI, Log, TEXT("HTTP request completed. Success: %s, Code: %d"),
        ResponseData.bWasSuccessful ? TEXT("true") : TEXT("false"),
        ResponseData.ResponseCode);

    if (!ResponseData.bWasSuccessful)
    {
        UE_LOG(LogHttpBlueprintAPI, Warning, TEXT("HTTP request failed: %s"), *ResponseData.ErrorMessage);
    }

    // CRITICAL: Execute the Blueprint delegate on the Game Thread
    // HTTP callbacks happen on background threads, but Blueprint code must run on the main thread
    AsyncTask(ENamedThreads::GameThread, [UserCallback, ResponseData]()
        {
            if (UserCallback.IsBound())
            {
                UserCallback.ExecuteIfBound(
                    ResponseData.bWasSuccessful,
                    ResponseData.ResponseCode,
                    ResponseData.ResponseBody,
                    ResponseData.ErrorMessage
                );
            }
            else
            {
                UE_LOG(LogHttpBlueprintAPI, Warning, TEXT("HTTP response received but no callback delegate was bound"));
            }
        });
}

FHttpResponseData UHttpBlueprintFunctionLibrary::ProcessHttpResponse(
    FHttpRequestPtr Request,
    FHttpResponsePtr Response,
    bool bWasSuccessful)
{
    FHttpResponseData ResponseData;

    // Calculate response time
    if (Request.IsValid())
    {
        ResponseData.ResponseTimeSeconds = Request->GetElapsedTime();
    }

    if (bWasSuccessful && Response.IsValid())
    {
        // Get basic response info
        ResponseData.ResponseCode = Response->GetResponseCode();
        ResponseData.ResponseBody = Response->GetContentAsString();

        // Extract response headers
        TArray<FString> AllHeaders = Response->GetAllHeaders();
        for (const FString& HeaderLine : AllHeaders)
        {
            FString HeaderName, HeaderValue;
            if (HeaderLine.Split(TEXT(": "), &HeaderName, &HeaderValue))
            {
                ResponseData.ResponseHeaders.Add(HeaderName, HeaderValue);
            }
        }

        // Determine if this is considered a successful response
        ResponseData.bWasSuccessful = IsHttpResponseSuccessful(ResponseData.ResponseCode);

        if (!ResponseData.bWasSuccessful)
        {
            ResponseData.ErrorMessage = FString::Printf(
                TEXT("HTTP Error %d: %s"),
                ResponseData.ResponseCode,
                *GetHttpResponseCodeDescription(ResponseData.ResponseCode)
            );
        }
    }
    else
    {
        // Request failed at the network level
        ResponseData.bWasSuccessful = false;
        ResponseData.ResponseCode = 0;
        ResponseData.ErrorMessage = TEXT("Network error: Request failed to complete");

        // Try to get more specific error info if available
        if (Request.IsValid())
        {
            FString URL = Request->GetURL();
            ResponseData.ErrorMessage += FString::Printf(TEXT(" (URL: %s)"), *URL);
        }
    }

    return ResponseData;
}

TSharedRef<IHttpRequest, ESPMode::ThreadSafe> UHttpBlueprintFunctionLibrary::CreateHttpRequest(
    const FString& URL,
    const FString& Method,
    const FString& RequestBody,
    const TMap<FString, FString>& Headers)
{
    // Get the HTTP module and create a new request
    FHttpModule* Http = &FHttpModule::Get();
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http->CreateRequest();

    // Configure the basic request parameters
    Request->SetURL(URL);
    Request->SetVerb(Method.ToUpper());

    // Set the request body if provided
    if (!RequestBody.IsEmpty())
    {
        Request->SetContentAsString(RequestBody);
    }

    // Apply all headers
    for (const auto& HeaderPair : Headers)
    {
        Request->SetHeader(HeaderPair.Key, HeaderPair.Value);
    }

    // Set some default headers if they weren't provided
    if (!Headers.Contains(TEXT("User-Agent")))
    {
        Request->SetHeader(TEXT("User-Agent"), TEXT("UnrealEngine/5.0 HttpBlueprintAPI/1.0"));
    }

    // Set timeout (30 seconds default)
    Request->SetTimeout(30.0f);

    return Request;
}

bool UHttpBlueprintFunctionLibrary::ValidateHttpRequest(
    const FString& URL,
    const FString& Method,
    FString& OutErrorMessage)
{
    // Validate URL
    if (URL.IsEmpty())
    {
        OutErrorMessage = TEXT("URL cannot be empty");
        return false;
    }

    if (!IsValidURL(URL))
    {
        OutErrorMessage = TEXT("Invalid URL format. URL must start with http:// or https://");
        return false;
    }

    // Validate HTTP method
    if (Method.IsEmpty())
    {
        OutErrorMessage = TEXT("HTTP method cannot be empty");
        return false;
    }

    FString UpperMethod = Method.ToUpper();
    if (UpperMethod != TEXT("GET") && UpperMethod != TEXT("POST") &&
        UpperMethod != TEXT("PUT") && UpperMethod != TEXT("DELETE") &&
        UpperMethod != TEXT("PATCH") && UpperMethod != TEXT("HEAD") &&
        UpperMethod != TEXT("OPTIONS"))
    {
        OutErrorMessage = FString::Printf(TEXT("Unsupported HTTP method: %s"), *Method);
        return false;
    }

    OutErrorMessage = TEXT("");
    return true;
}