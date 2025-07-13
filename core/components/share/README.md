
# Sample configuration files

This directory contains sub-directories with sample configuration files (JSON)

  * I2RIS  Configuration file for JHU eye snake robot

The JSON file contains the following fields:

| Keyword       | Default   | Description                                           |
|:--------------|:----------|:------------------------------------------------------|
| file_version  |           | Version of JSON file format                           |
| name          |           | Robot name                                            |
| device_name   |           | Controller device name (e.g., "EPOS2")                |
| protocol_stack_name  |    | Protocol name                                         |
| interface_name |          | Name of interface (e.g., "USB")                       |
| port_name     |           | Name of port used                                     |
| timeout       |           | Timeout for communications (msec)                     |
|               |           |
| axes          |           | Array of robot axis configuration data (see below)    |
|  - nodeid     |           |  - Node id for controller                             |
