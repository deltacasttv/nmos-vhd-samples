# NMOS VHD Samples

The VideoMaster HD NMOS samples demonstrate the integration of several NMOS standards with the VideoMasterHD API and a [DELTACAST IP Card](https://www.deltacast.tv/products/developer-products/ip-cards/smpte-st-2110-capture-card).

NMOS, as defined by AMWA (Advanced Media Workflow Association), is a set of specifications for IP-based media networks. It enables interoperability between compliant devices and systems, and provides a framework for the development of IP-based media solutions.

The NMOS VHD samples showcase the basic structure and usage of the NMOS APIs in conjunction with [DELTACAST](https://www.deltacast.tv/) VideoMasterHD API. It can be used as a reference for implementing VHD solutions in media production and distribution workflows using NMOS.

The sample include a Receiver Node Sample, which demonstrates the implementation of a receiver node capable of discovering, connecting to, and displaying streams from NMOS-compliant sender nodes. This sample utilizes the NMOS IS-04 and IS-05 specifications for discovery and connection management. It showcases the ability to dynamically discover available sender nodes on the network, establish connections with them, and receive and display the video streams they send. By providing a practical example of NMOS functionality integration into a receiver application, this sample serves as a valuable reference for developers.

This sample serve as reference implementation for developers looking to build NMOS-compliant receiver nodes. It demonstrates the usage of the NMOS APIs, including discovery, connection management, and streaming. By studying and modifying the sample, developers can gain a better understanding of how to integrate NMOS functionality into their own applications with the [DELTACAST](https://www.deltacast.tv/) VideoMaster SDK.

## Features
- Supports the following NMOS specifications:
  - [IS-04](https://specs.amwa.tv/is-04/): Discovery and Registration of NMOS Devices and Resources
  - [IS-05](https://specs.amwa.tv/is-05/): Connection Management
  - [IS-09](https://specs.amwa.tv/is-09/): System API
- Supports real-time media transport, discovery, and control over IP networks. NMOS VHD Samples only demonstrate ST2110-20 video essence.
- Enables interoperability with other NMOS-compliant devices and systems.
- Supports PTP synchronization.
- Supports 1 stream per instance.

## Dependencies installation
### VideoMaster SDK

The VideoMaster SDK is required to run the NMOS VHD Samples. It is available for download at https://www.deltacast.tv/support/download-center. These samples have been developed and tested with VideoMaster SDK 6.25.2.

### Submodules

To run the NMOS VHD Samples, this project directly uses the [nmos-cpp](https://github.com/sony/nmos-cpp) library, which is a C++ open source implementation of the NMOS specifications. To clone this repository as a submodule, use the following commands:

```bash
git submodule init
git submodule update
```

nmos-cpp has its own dependencies, which are listed below.

### Dependencies

#### [cmake](https://cmake.org/)
Install cmake with a method of your choice. For example, on Ubuntu, you can use the following command: `sudo apt-get install cmake`. CMake version 3.19 or higher is required.
#### [conan](https://conan.io/)
Install conan 2.X with a method of your choice.

#### DNS Service Discovery
##### Windows

1. Download the [Bonjour SDK for Windows v3.0](https://developer.apple.com/download/more/?=Bonjour%20SDK%20for%20Windows) from Apple Developer Downloads
2. Run bonjoursdksetup.exe, which will install the necessary files in e.g. *C:\Program Files\Bonjour SDK*
3. The critical files for building this software are:
   - *Bonjour SDK\Include\dns_sd.h*
   - *Bonjour SDK\Lib\x64\dnssd.lib*
4. The Bonjour service itself needs to be installed on the system where this software is run.  
   The installer is included in the SDK:
   - *Bonjour SDK\Installer\Bonjour64.msi*

##### Linux

The [Avahi](https://www.avahi.org/) project provides a DNS-SD daemon for Linux, and the *avahi-compat-libdns_sd* library which enables applications to use the original Bonjour *dns_sd.h* API to communicate with the Avahi daemon.

For example, to install the Bonjour compatibility library, Name Service Switch module, and Avahi tools on Ubuntu:
```sh
sudo apt-get install -f libavahi-compat-libdnssd-dev libnss-mdns avahi-utils
```

##### MacOS

No need to install anything.

For more details, refer to the [nmos-cpp documentation](https://github.com/sony/nmos-cpp/blob/6d64db87f133dbb91bf1d10ba09053543884390a/Documents/Dependencies.md)

## Parameters

The NMOS VHD Samples do not take any command line parameter.
You have to edit [receiver.cpp](src/receiver/receiver.cpp) to change the parameters. They are located at the beginning of the main function. Some parameters must be changed according to your environment to make the sample work. Those parameters are :
  | Parameter | Description |
  |-----------|-------------|
  | `board_id` | The board ID of the DELTACAST IP Card. |
  | `media_nic_ip` | The IP address which will be programmed on the DELTACAST IP Card. |
  | `media_nic_gateway` | The gateway which will be programmed on the DELTACAST IP Card. |
  | `media_nic_subnet_mask` | The subnet mask which will be programmed on the DELTACAST IP Card. |
  | `media_nic_dhcp` | If true, the DELTACAST IP Card will be configured with DHCP (in that case, the other IP parameters are ignored). |
  | `management_nic_ip` | The IP address of the management network interface. |
  | `node_domain` | The DNS search domain (use `"local."` for multicast DNS). |

The other parameters can be changed accordingly to your needs.

## Build and Execution

After installing the dependencies and having configured the required parameters, you can build the NMOS VHD Samples using the following commands:
```shell
cd /path/to/nmos-vhd-sample
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

The executable will be compiled in the following directory: `/build/src/receiver/`

 ### Firewall configuration
 If you experience troubles connecting the NMOS VHD Samples to your NMOS infrastructure, you may need to configure your machine firewall to allow the following ports:
 - registration_port: 3210
 - node_port: 3212
 - connection_port: 3215
 - events_port: 3216
 - events_ws_port: 3217
 - channelmapping_port: 3215
 - system_port: 10641

 The node and connection port are parameters of the NMOS VHD Samples. If you change them, you will need to change the firewall configuration accordingly.

 On windows you can use the following script:
  ```powershell
  New-NetFirewallRule -DisplayName "NMOS VHD Samples" -Direction Inbound -Protocol TCP -LocalPort 3210,3212,3215,3216,3217,10641 -Action Allow
  ```

## Testing
Those samples have been tested with VideoMaster SDK 6.25.2 on:

 - Mac OS Ventura 13.6.8 (Apple M2 Ultra)
 - Windows Server 2022
 - Ubuntu Linux 20.04 LTS

## Support
For any questions or issues related to the NMOS VHD Samples, please refer to the [project's issue tracker](https://github.com/deltacasttv/nmos-vhd-samples/issues).

## Contributing
We welcome contributions from the community to help improve the nmos-vhd-sample project. If you would like to contribute, please follow these guidelines:

1. Fork the repository and create a new branch for your contribution.
2. Submit a pull request with a clear description of your changes.
3. Your pull request will be reviewed by the project maintainers, and any necessary feedback will be provided.
5. Once your contribution is approved, it will be merged into the main branch.

By contributing to the NMOS VHD Samples project, you agree to abide by the licensing terms specified in the repository.

We appreciate your contributions and look forward to working with you to make the NMOS VHD Samples project even better!

## License
This NMOS VHD Samples are licensed under the [Apache License](http://www.apache.org/licenses/).
