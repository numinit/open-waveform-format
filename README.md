# Open Waveform Format

## Introduction

The Open Waveform Format (OWF) is a new format for transmitting captured waveforms, alarms, and events from devices.

### Features

* HTTP transport (with MIME types)
* JSON format for development and prototyping
* Binary format for speed
* Designed to transmit measurements
* Designed to optionally transmist device alarms and events in free text
* Support for batch updates for multiple data sources, multiple data acquisition devices, and multiple acquisition channels

Read more on our [wiki](https://github.com/medicalinformaticscorp/open-waveform-format/wiki)

### Supported languages

|Language|JSON|Binary|
|:-------|:---|:-----|
|C|Planned|**Deserialize only (serializer in progress)**|
|C++|Planned|Planned|
|C#|Planned|**Complete**|
|JS (NodeJS)|Planned|Planned|
|Ruby|Planned|**Convert from JSON only**|
|Elixir|Planned|Planned|

## Contributing

### Workflow:

1. Open an issue.
2. Fork the repository on Github.
3. Commit work.
4. Submit pull request, referencing the Github issue.
5. We'll review and merge the pull request, perhaps including discussion and changes.

### Licensing

Please explicitly enumerate any 3rd-party libraries you depend on in your code changes, either opening a new dependencies file or updating an existing one.

*We are unable to accept any code licensed under or requiring copyleft licenses (GPL, AGPL, etc.).*

*All code submitted (excluding supporting libraries) is considered as being relicensable under the Apache License 2.0.* If this is not acceptable, please do not submit code.

## Scripting utilities

* ruby/json2owf.rb: Simple utility to convert an OWF JSON object into an OWF binary object. Use `ruby json2owf.rb <input JSON> <output OWF>` to create an OWF object from a JSON file.

## License

This work is all free software under the Apache License Version 2.0.

Supporting libraries are licensed under their original licenses.
