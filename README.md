# HttpBlueprintFunctionLibrary Plugin

This plugin provides Blueprint-accessible HTTP functionality for Unreal Engine 5.

## Features

- Make HTTP GET and POST requests with custom headers.
- Receive asynchronous callbacks in Blueprints with response data.
- Utility functions to validate URLs and check HTTP response status codes.

## How to Use

### Installation

1. Copy the `HttpBlueprintFunctionLibrary` folder into your Unreal project’s `Plugins` folder.
2. Enable the plugin in your project settings (Edit → Plugins).
3. Restart the Unreal Editor to compile the plugin.

### Blueprint Usage

- Use the `Make HTTP GET Request` node to send GET requests.
- Use the `Make HTTP POST Request` node to send POST requests with a request body.
- Use the `Make HTTP Request with Headers` node for full control over HTTP method and headers.
- Bind your Blueprint event to the callback delegate to handle the response.

### Sample GET Request

```blueprint
MakeHttpGetRequest("https://api.example.com/data", OnResponseReceived);

