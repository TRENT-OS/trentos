# Configuration Provisioning Tool

## General

The Configuration Provisioning Tool (**`cpt`**) creates provisioned
binary files that contain configuration data that can be managed by the
[ConfigService API](../api/config-service_api.md). The configuration data is first
parsed from an XML file and in turn written to the files. A TRENTOS system that
makes use of a ConfigService component, can interpret these files and serve the
clients connected to the ConfigService component with their requested
configuration parameters.

## Settings

As the tool can also create provisioned images, there is one setting for
this option regarding the output file size, that might need to be
adjusted. In case the total size of the configuration data exceeds the
default setting of 128 KiB, this can be reconfigured with the
below-mentioned defines in the **`config.h`** that can be found in the
directory containing the sources of the tool.

```c
// Set the max. size of the output image
# define HOSTSTORAGE_SIZE ((size_t)(128 * 1024))
```

## Configuration Data Layout

The configuration data for a TRENTOS system requires a specific XML
layout so that the tool can parse the input settings and write them to
the provisioned image. To demonstrate more clearly how the configuration
data can be worked into the XML layout required by the tool, the
following configuration data from the Cloud Connector component of
the [IoT Demo App for QEMU](../demos/demo-iot_qemu.md) is taken as an example.

```xml
<domain name = 'Domain-CloudConnector'>
  <param_name>ServerPort</param_name>
    <type>int32</type>
    <access_policy>
        <read>true</read>
        <write>false</write>
    </access_policy>
    <value>8883</value>

  <param_name>SharedAccessSignature</param_name>
    <type>blob</type>
    <access_policy>
        <read>true</read>
        <write>false</write>
    </access_policy>
    <value>/cloudConnector_SharedAccessSignature</value>

  <param_name>IoT-Hub</param_name>
    <type>blob</type>
    <access_policy>
        <read>true</read>
        <write>false</write>
    </access_policy>
    <value>/cloudConnector_cloudDomain</value>

  <param_name>CloudServiceIP</param_name>
    <type>string</type>
    <access_policy>
        <read>true</read>
        <write>false</write>
    </access_policy>
    <value>10.0.0.1</value>

  <param_name>IoT-Device</param_name>
    <type>string</type>
    <access_policy>
        <read>true</read>
        <write>false</write>
    </access_policy>
    <value>TempSensor_01</value>

  <param_name>ServerCaCert</param_name>
    <type>blob</type>
    <access_policy>
        <read>true</read>
        <write>false</write>
    </access_policy>
    <value>/cloudConnector_ServerCACert.pem</value>
</domain>
```

As described in the chapter about the [ConfigService API](../api/config-service_api.md),
the configuration parameters can be grouped in domains, and in the
presented configuration excerpt, the Cloud Connector component was set
to be its own domain with a specific set of parameters. In general, the
layout to describe the parameters always follows the same pattern but
the value entry differs based on the type of the parameter. The current
implementation of the  [ConfigService API](../api/config-service_api.md) supports
storing parameter values either as integers, strings, or blobs.

### Setting Integer Parameters

The values for parameters that are set to be treated as integer values
can be written directly to the **`<value>`** element of the respective
configuration parameter in the configuration XML file.

```xml
<param_name>ServerPort</param_name>
    <type>int32</type>
    <access_policy>
        <read>true</read>
        <write>false</write>
    </access_policy>
    <value>8883</value>
```

**Warning:** Values set for parameters of type \"**`int32`**\" or
\"**`int64`**\" in the XML file will be treated as unsigned integers by the
current implementation of the ConfigService library. Native support for
signed integers is currently not implemented but it can still be
utilized by casting the unsigned integer value retrieved from the
[ConfigService API](../api/config-service_api.md) to a signed integer.

Integer values can be entered in the following formats:

- Decimal positive values, e.g. \"194827\" or \"+194827\"
- Decimal negative values, e.g. \"-194827\" (as mentioned above, this
    will require the value to be casted to a signed integer when
    retrieved through the [ConfigService API](../api/config-service_api.md)).
- Hexadecimal lower case, e.g. \"0x2f90b\"
- Hexadecimal upper case, e.g. \"0x2F90B\"

As can be seen in the examples provided, values entered in hex format
need to be prefixed with \"**`0x`**\".

### Setting String Parameters

Similar to integer values, string values can also be written directly to
the **`<value>`** element of the respective configuration parameter in
the configuration XML file.

```xml
<param_name>CloudServiceIP</param_name>
    <type>string</type>
    <access_policy>
        <read>true</read>
        <write>false</write>
    </access_policy>
    <value>10.0.0.1</value>
```

**Warning:** The [ConfigService API](../api/config-service_api.md) allows for a
maximum string size of 32 bytes (including NUL terminator). Every parameter
entry that goes beyond that size should be stored as a blob parameter.

### Setting Blob Parameters

The only difference that needs to be handled correctly when setting blob
parameters is the way the value of the parameter is set in the XML file.
Instead of quite inconveniently having to directly paste the value of
the blob into the XML file, the path to the file containing the blob
value is set in the **`<value>`** element. As seen in the example below,
the **`<value>`** element contains the path to a file called
**`cloudConnector_ServerCACert.pem`** that is stored in the same
directory as the configuration XML file (file paths have to be passed
relative to the location of the XML file).

```xml
<param_name>ServerCaCert</param_name>
    <type>blob</type>
    <access_policy>
        <read>true</read>
        <write>false</write>
    </access_policy>
    <value>/cloudConnector_ServerCACert.pem</value>
```

To get a better understanding of how this can be applied to a TRENTOS
system, make sure to take a look at the
[IoT Demo App for QEMU](../demos/demo-iot_qemu.md) included in the SDK.

## Build

The tool with its default settings is already contained pre-built in the
**`bin`** folder as part of the delivered SDK.

If the settings of the tools need to be changed or recompilation is
required for some other reason, the tool can be built standalone by
running the provided **`build.sh`** script that can be found in the root
directory of the tool and passing it the SDK path to build against.

```shell
cd <sdk_root_directory>

# run sdk/tools/cpt/build.sh in the container,
# parameter is the relative path to the SDK (within the container)
sdk/scripts/open_trentos_build_env.sh sdk/tools/cpt/build.sh sdk
```

This will create a separate folder **`build_cpt`**, which includes the application
binary.

## Run

Run the tool and provide it with the path to the XML file containing the
configuration parameters. The tool will create the binary files
**`DOMAIN.BIN`**, **`PARAMETER.BIN`**, **`STRING.BIN`**, **`BLOB.BIN`**
from the specified input file.

If the tool should create a provisioned image file instead of creating
the binary files directly, this will require additionally passing it the
name for the output image file and specifying what type of file system
is used in the TRENTOS system that the image is intended for.

```shell
sdk/bin/cpt -i [<path-to-xml_file>] -o [<output_nvm_file_name>] -t [filesystem_type]
```

### Options

**-i** *input-path*

Specify the path to the XML file containing the configuration parameters
that should be written to the configuration files.

**-o** *output-file-name*

Specify the output image name. This option is only required if the tool
should produce a provisioned image instead of the binary files.

**-t** *filesystem-type*

Select the file system type, *filesystem_type*  can be either \"FAT\",
\"SPIFFS\" or \"LITTLEFS\". This option is only required if the tool
should produce a provisioned image instead of the binary files.

**Note:** To use the created image in combination with the
[Proxy Application](proxy-application.md), it is important to note that the
image file name needs to match the convention used by the Proxy. For example, if
the TRENTOS system is using the first NVM channel of the Proxy, which has the
channel ID 6 in the proxy, then the image file name needs to be set to
**`nvm_06`**, so the Proxy can find this file.

### Example

The following example creates a provisioned image file that will be named
**`nvm_06`** (to comply with the conventions of the
[Proxy Application](proxy-application.md)) and formatted to a FAT file system:

```shell
./cpt -i [<path-to-xml_file>] -o nvm_06 -t FAT
```
