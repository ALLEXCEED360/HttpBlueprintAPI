# HTTP Blueprint API Plugin for Unreal Engine 5

[![Unreal Engine](https://img.shields.io/badge/Unreal%20Engine-5.0%2B-blue.svg)](https://www.unrealengine.com/)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/Platform-Win64%20%7C%20Mac%20%7C%20Linux-lightgrey.svg)]()

A comprehensive Unreal Engine 5 plugin that enables Blueprint developers to make HTTP requests with proper callback handling, error management, and thread-safe operations.

## 🚀 Features

- ✅ **Blueprint Delegate Callbacks** - Asynchronous response handling without freezing gameplay
- ✅ **Multiple HTTP Methods** - GET, POST support with custom headers
- ✅ **Comprehensive Error Handling** - Detailed error messages and HTTP status code interpretation
- ✅ **Thread-Safe Operations** - Proper async task management for Blueprint safety
- ✅ **JSON Ready** - Built-in support for JSON APIs and content types
- ✅ **Input Validation** - URL and parameter validation with meaningful error feedback
- ✅ **Production Ready** - Timeout handling, logging, and robust error management
- ✅ **Blueprint Utilities** - Helper functions for common HTTP operations

## 📦 Installation

### Method 1: Manual Installation
1. Download or clone this repository
2. Copy the `HttpBlueprintAPI` folder to your project's `Plugins/` directory
3. If `Plugins/` doesn't exist, create it in your project root
4. Regenerate project files (right-click `.uproject` → "Generate Visual Studio project files")
5. Compile your project
6. Enable the plugin in Unreal Editor (`Edit` → `Plugins` → Search "HTTP Blueprint API")

### Method 2: Git Submodule
```bash
cd YourUnrealProject
git submodule add https://github.com/ALLEXCEED360/HttpBlueprintAPI.git Plugins/HttpBlueprintAPI
```

## 🎯 Quick Start

### 1. Basic GET Request

In your Blueprint:

1. **Create a Custom Event** named `OnAPIResponse` with these parameters:
   - `Was Successful` (Boolean)
   - `Response Code` (Integer)
   - `Response Body` (String)
   - `Error Message` (String)

2. **Add HTTP Request**:
   ```
   [Event] → [Make HTTP GET Request]
       • URL: "https://jsonplaceholder.typicode.com/posts/1"
       • On Response Received: OnAPIResponse
       • World Context Object: Self
   ```

3. **Handle Response**:
   ```
   [OnAPIResponse] → [Branch: Was Successful?]
       ├── True: Display Response Body
       └── False: Display Error Message
   ```

### 2. POST Request with JSON

```
[Event] → [Make HTTP POST Request]
    • URL: "https://api.example.com/data"
    • Request Body: '{"name": "John", "age": 30}'
    • Content Type: "application/json"
    • On Response Received: OnAPIResponse
    • World Context Object: Self
```

## 📚 API Reference

### Core Functions

#### `Make HTTP GET Request`
Simple GET request with callback support.
- **URL** (String): The API endpoint URL
- **On Response Received** (Delegate): Blueprint callback function
- **World Context Object** (Object): Usually "Self"

#### `Make HTTP POST Request`
POST request with request body support.
- **URL** (String): The API endpoint URL
- **Request Body** (String): Data to send (usually JSON)
- **Content Type** (String): Content type header (e.g., "application/json")
- **On Response Received** (Delegate): Blueprint callback function
- **World Context Object** (Object): Usually "Self"

#### `Make HTTP Request with Headers`
Advanced request with full header customization.
- **URL** (String): The API endpoint URL
- **Method** (String): HTTP method (GET, POST, PUT, DELETE, etc.)
- **Request Body** (String): Data to send
- **Headers** (Map): Custom headers as key-value pairs
- **On Response Received** (Delegate): Blueprint callback function
- **World Context Object** (Object): Usually "Self"

### Utility Functions

#### `Is HTTP Response Successful`
- **Input**: Response Code (Integer)
- **Output**: Boolean (true for codes 200-299)

#### `Get HTTP Response Code Description`
- **Input**: Response Code (Integer)
- **Output**: String ("OK", "Not Found", etc.)

#### `Is Valid URL`
- **Input**: URL (String)
- **Output**: Boolean (basic URL format validation)

#### `Get Domain from URL`
- **Input**: URL (String)
- **Output**: String (extracted domain name)

## 🧪 Testing

### Test URLs for Different Scenarios

**✅ Successful Response (200):**
```
https://jsonplaceholder.typicode.com/posts/1
```

**❌ Not Found Error (404):**
```
https://jsonplaceholder.typicode.com/posts/999999
```

**❌ Invalid URL Format:**
```
not-a-valid-url
```

**❌ Network Error:**
```
https://this-domain-definitely-does-not-exist-12345.com/test
```

### Example Test Blueprint

1. Create an Actor Blueprint
2. Add a Static Mesh Component (Cube) for visibility
3. Implement the Quick Start example above
4. Place in level and test during Play mode

## 🔧 Configuration

### Build Dependencies
The plugin requires these Unreal Engine modules:
- Core
- CoreUObject
- Engine
- HTTP
- Json
- JsonUtilities

### Project Settings
No additional project settings required. The plugin works out of the box.

## 📖 Examples

### Weather API Integration
```cpp
// Custom Event: OnWeatherReceived (Same 4 parameters)
URL: "https://api.openweathermap.org/data/2.5/weather?q=London&appid=YOUR_API_KEY"
```

### REST API with Authentication
```cpp
// Use Make HTTP Request with Headers
Headers: 
  - "Authorization": "Bearer YOUR_TOKEN"
  - "Content-Type": "application/json"
```

### File Upload Simulation
```cpp
// POST request with form data
Request Body: '{"file_name": "image.jpg", "file_data": "base64_encoded_data"}'
Content Type: "application/json"
```

## 🛠️ Development

### Project Structure
```
HttpBlueprintAPI/
├── Source/
│   └── HttpBlueprintAPI/
│       ├── HttpBlueprintAPI.cpp          # Module implementation
│       ├── HttpBlueprintAPI.h            # Module header
│       ├── HttpBlueprintFunctionLibrary.cpp  # Main plugin logic
│       ├── HttpBlueprintFunctionLibrary.h    # Blueprint interface
│       └── HttpBlueprintAPI.Build.cs     # Build configuration
├── HttpBlueprintAPI.uplugin              # Plugin metadata
└── README.md
```

### Key Technical Improvements

**Blueprint Delegate System:**
- `DECLARE_DYNAMIC_DELEGATE_FourParams` for Blueprint callbacks
- Thread-safe execution using `AsyncTask(ENamedThreads::GameThread)`
- Proper delegate binding and execution

**Enhanced Error Handling:**
- Input validation before request execution
- HTTP status code interpretation
- Network error detection and reporting
- Timeout and failure management

**Production Features:**
- Request timeout handling (30 seconds)
- Response time measurement
- Response header extraction
- Comprehensive logging system
- Memory-safe request lifecycle management

## 🐛 Troubleshooting

### Plugin Not Appearing in Blueprint
1. Ensure plugin is enabled: `Edit` → `Plugins` → Search "HTTP Blueprint API"
2. Restart Unreal Editor after enabling
3. Compile your Blueprint project

### Build Errors
1. Close Unreal Editor completely before building
2. Disable Live Coding: `Ctrl+Alt+F11` in editor
3. Clean and rebuild solution in Visual Studio

### Requests Not Working
1. Check Output Log for detailed error messages (`Window` → `Developer Tools` → `Output Log`)
2. Verify internet connection and firewall settings
3. Test with known working URLs (see Testing section)

### Response Not Showing in Blueprint
1. Ensure Custom Event parameters match exactly:
   - `Was Successful` (Boolean)
   - `Response Code` (Integer)
   - `Response Body` (String)
   - `Error Message` (String)
2. Check that `Print to Screen` is enabled on Print String nodes
3. Verify World Context Object is connected to "Self"

## 📋 Requirements

- **Unreal Engine**: 5.0 or higher
- **Platform**: Windows, Mac, Linux
- **Build Tools**: Visual Studio 2019/2022 (Windows), Xcode (Mac)
- **Dependencies**: No external dependencies required

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## 📝 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- Built on Unreal Engine's HTTP module
- Inspired by the need for better Blueprint HTTP integration
- Thanks to the Unreal Engine community for feedback and suggestions

## 📞 Support

- **Issues**: [GitHub Issues](https://github.com/ALLEXCEED360/HttpBlueprintAPI/issues)
- **Discussions**: [GitHub Discussions](https://github.com/ALLEXCEED360/HttpBlueprintAPI/discussions)
- **Documentation**: [Wiki](https://github.com/ALLEXCEED360/HttpBlueprintAPI/wiki)

---

**Made with ❤️ for the Unreal Engine Blueprint community**
