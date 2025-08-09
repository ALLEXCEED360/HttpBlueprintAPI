# HttpBlueprintAPI

An Unreal Engine 5 plugin providing Blueprint nodes to make HTTP API requests.

## Features

- Easy-to-use BlueprintCallable node to perform HTTP GET requests.
- Prints API response to screen and output log.
- Designed for UE5.5 with C++ and Blueprint integration.

## Installation

1. Copy the `HttpBlueprintAPI` folder into your project’s `Plugins/` directory.
2. Open your project in Unreal Editor.
3. Go to **Edit → Plugins**, find **HttpBlueprintAPI** under **Installed**.
4. Enable the plugin and restart the editor.

## Usage

- In any Blueprint, call the **Make API Request** node.
- Provide a valid URL, for example:

  ```
  https://jsonplaceholder.typicode.com/todos/1
  ```

- The response will print on screen and in the log.

## License

This project is licensed under the MIT License - see the LICENSE file for details.
