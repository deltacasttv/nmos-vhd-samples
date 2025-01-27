/*
 * SPDX-FileCopyrightText: Copyright (c) DELTACAST.TV. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at * * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <array>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>
#include <vector>

#include "tools.h"

#if defined(__APPLE__)
#include "VideoMasterHD/VideoMasterHD_Ip_Board.h"
#include "VideoMasterHD/VideoMasterHD_Ip_ST2110_20.h"
#include "VideoMasterHD/VideoMasterHD_PTP.h"
#include "VideoMasterHD/VideoMasterHD_String.h"
#else
#include "VideoMasterHD_Ip_Board.h"
#include "VideoMasterHD_Ip_ST2110_20.h"
#include "VideoMasterHD_PTP.h"
#include "VideoMasterHD_String.h"
#endif

std::string to_string(VHD_PTP_PORT_STATE state) {
   return VHD_PTP_PORT_STATE_ToString(state);
}

std::string to_string(VHD_ERRORCODE error_code)
{
   return VHD_ERRORCODE_ToString(error_code);
}

VHD_ERRORCODE get_nic_mac_address(HANDLE board_handle, std::string& mac_address)
{
   VHD_ERRORCODE result;
   ULONG mac_lsw, mac_msw;

   result = static_cast<VHD_ERRORCODE>(VHD_GetEthernetPortProperty(
       board_handle, VHD_IP_BRD_ETHERNETPORT_ETH_0, VHD_IP_BRD_EP_FACTORY_MAC_ADDR_LSW, &mac_lsw));
   if (result != VHDERR_NOERROR)
   {
      std::cout << "Error getting MAC address LSW: " << to_string(result) << std::endl;
      return result;
   }

   result = static_cast<VHD_ERRORCODE>(VHD_GetEthernetPortProperty(
       board_handle, VHD_IP_BRD_ETHERNETPORT_ETH_0, VHD_IP_BRD_EP_FACTORY_MAC_ADDR_MSW, &mac_msw));
   if (result != VHDERR_NOERROR)
   {
      std::cout << "Error getting MAC address MSW: " << to_string(result) << std::endl;
      return result;
   }

   // Combine MSW and LSW to form the full MAC address
   uint8_t mac[6];
   mac[0] = (mac_msw >> 8) & 0xFF;
   mac[1] = mac_msw & 0xFF;
   mac[2] = (mac_lsw >> 24) & 0xFF;
   mac[3] = (mac_lsw >> 16) & 0xFF;
   mac[4] = (mac_lsw >> 8) & 0xFF;
   mac[5] = mac_lsw & 0xFF;

   // Format the MAC address as a string
   std::stringstream ss;
   ss << std::hex << std::setfill('0');
   for (int i = 0; i < 6; ++i)
   {
      if (i > 0) ss << "-";
      ss << std::setw(2) << static_cast<int>(mac[i]);
   }
   mac_address = ss.str();

   return VHDERR_NOERROR;
}

VHD_ERRORCODE get_video_standard_info(VHD_ST2110_20_VIDEO_STANDARD video_standard,
   uint32_t& frame_width, uint32_t& frame_height, uint32_t& frame_rate, bool& interlaced, bool& is_us)
{
   switch (video_standard)
   {
   case VHD_ST2110_20_VIDEOSTD_720x480i59:
   case VHD_ST2110_20_VIDEOSTD_720x487i59:
      frame_width = 720;
      frame_height = (video_standard == VHD_ST2110_20_VIDEOSTD_720x480i59) ? 480 : 487;
      frame_rate = 60;
      interlaced = true;
      is_us = true;
      break;
   case VHD_ST2110_20_VIDEOSTD_720x576i50:
      frame_width = 720;
      frame_height = 576;
      frame_rate = 50;
      interlaced = true;
      is_us = false;
      break;
   case VHD_ST2110_20_VIDEOSTD_1280x720p50:
   case VHD_ST2110_20_VIDEOSTD_1280x720p59:
   case VHD_ST2110_20_VIDEOSTD_1280x720p60:
      frame_width = 1280;
      frame_height = 720;
      frame_rate = (video_standard == VHD_ST2110_20_VIDEOSTD_1280x720p50) ? 50 : 60;
      interlaced = false;
      is_us = (video_standard == VHD_ST2110_20_VIDEOSTD_1280x720p59);
      break;
   case VHD_ST2110_20_VIDEOSTD_1920x1080i50:
   case VHD_ST2110_20_VIDEOSTD_1920x1080i59:
   case VHD_ST2110_20_VIDEOSTD_1920x1080i60:
      frame_width = 1920;
      frame_height = 1080;

      frame_rate = (video_standard == VHD_ST2110_20_VIDEOSTD_1920x1080i50) ? 50 : 60;
      interlaced = true;
      is_us = (video_standard == VHD_ST2110_20_VIDEOSTD_1920x1080i59);
      break;
   case VHD_ST2110_20_VIDEOSTD_1920x1080p23:
   case VHD_ST2110_20_VIDEOSTD_1920x1080p24:
   case VHD_ST2110_20_VIDEOSTD_1920x1080p25:
   case VHD_ST2110_20_VIDEOSTD_1920x1080p29:
   case VHD_ST2110_20_VIDEOSTD_1920x1080p30:
   case VHD_ST2110_20_VIDEOSTD_1920x1080p50:
   case VHD_ST2110_20_VIDEOSTD_1920x1080p59:
   case VHD_ST2110_20_VIDEOSTD_1920x1080p60:
      frame_width = 1920;
      frame_height = 1080;
      interlaced = false;
      frame_rate = (video_standard == VHD_ST2110_20_VIDEOSTD_1920x1080p23) ? 24 :
         (video_standard == VHD_ST2110_20_VIDEOSTD_1920x1080p24) ? 24 :
         (video_standard == VHD_ST2110_20_VIDEOSTD_1920x1080p25) ? 25 :
         (video_standard == VHD_ST2110_20_VIDEOSTD_1920x1080p29) ? 30 :
         (video_standard == VHD_ST2110_20_VIDEOSTD_1920x1080p30) ? 30 :
         (video_standard == VHD_ST2110_20_VIDEOSTD_1920x1080p50) ? 50 :
         (video_standard == VHD_ST2110_20_VIDEOSTD_1920x1080p59) ? 60 : 60;
      is_us = (video_standard == VHD_ST2110_20_VIDEOSTD_1920x1080p23 ||
         video_standard == VHD_ST2110_20_VIDEOSTD_1920x1080p29 ||
         video_standard == VHD_ST2110_20_VIDEOSTD_1920x1080p59);
      break;
   case VHD_ST2110_20_VIDEOSTD_2048x1080p23:
   case VHD_ST2110_20_VIDEOSTD_2048x1080p24:
   case VHD_ST2110_20_VIDEOSTD_2048x1080p25:
   case VHD_ST2110_20_VIDEOSTD_2048x1080p29:
   case VHD_ST2110_20_VIDEOSTD_2048x1080p30:
   case VHD_ST2110_20_VIDEOSTD_2048x1080p47:
   case VHD_ST2110_20_VIDEOSTD_2048x1080p48:
   case VHD_ST2110_20_VIDEOSTD_2048x1080p50:
   case VHD_ST2110_20_VIDEOSTD_2048x1080p59:
   case VHD_ST2110_20_VIDEOSTD_2048x1080p60:
      frame_width = 2048;
      frame_height = 1080;
      interlaced = false;
      frame_rate = (video_standard == VHD_ST2110_20_VIDEOSTD_2048x1080p23) ? 24 :
         (video_standard == VHD_ST2110_20_VIDEOSTD_2048x1080p24) ? 24 :
         (video_standard == VHD_ST2110_20_VIDEOSTD_2048x1080p25) ? 25 :
         (video_standard == VHD_ST2110_20_VIDEOSTD_2048x1080p29) ? 30 :
         (video_standard == VHD_ST2110_20_VIDEOSTD_2048x1080p30) ? 30 :
         (video_standard == VHD_ST2110_20_VIDEOSTD_2048x1080p47) ? 48 :
         (video_standard == VHD_ST2110_20_VIDEOSTD_2048x1080p48) ? 48 :
         (video_standard == VHD_ST2110_20_VIDEOSTD_2048x1080p50) ? 50 :
         (video_standard == VHD_ST2110_20_VIDEOSTD_2048x1080p59) ? 60 : 60;
      is_us = (video_standard == VHD_ST2110_20_VIDEOSTD_2048x1080p23 ||
         video_standard == VHD_ST2110_20_VIDEOSTD_2048x1080p29 ||
         video_standard == VHD_ST2110_20_VIDEOSTD_2048x1080p47 ||
         video_standard == VHD_ST2110_20_VIDEOSTD_2048x1080p59);
      break;
   case VHD_ST2110_20_VIDEOSTD_3840x2160p23:
   case VHD_ST2110_20_VIDEOSTD_3840x2160p24:
   case VHD_ST2110_20_VIDEOSTD_3840x2160p25:
   case VHD_ST2110_20_VIDEOSTD_3840x2160p29:
   case VHD_ST2110_20_VIDEOSTD_3840x2160p30:
   case VHD_ST2110_20_VIDEOSTD_3840x2160p50:
   case VHD_ST2110_20_VIDEOSTD_3840x2160p59:
   case VHD_ST2110_20_VIDEOSTD_3840x2160p60:
      frame_width = 3840;
      frame_height = 2160;
      interlaced = false;
      frame_rate = (video_standard == VHD_ST2110_20_VIDEOSTD_3840x2160p23) ? 24 :
         (video_standard == VHD_ST2110_20_VIDEOSTD_3840x2160p24) ? 24 :
         (video_standard == VHD_ST2110_20_VIDEOSTD_3840x2160p25) ? 25 :
         (video_standard == VHD_ST2110_20_VIDEOSTD_3840x2160p29) ? 30 :
         (video_standard == VHD_ST2110_20_VIDEOSTD_3840x2160p30) ? 30 :
         (video_standard == VHD_ST2110_20_VIDEOSTD_3840x2160p50) ? 50 :
         (video_standard == VHD_ST2110_20_VIDEOSTD_3840x2160p59) ? 60 : 60;
      is_us = (video_standard == VHD_ST2110_20_VIDEOSTD_3840x2160p23 ||
         video_standard == VHD_ST2110_20_VIDEOSTD_3840x2160p29 ||
         video_standard == VHD_ST2110_20_VIDEOSTD_3840x2160p59);
      break;
   case VHD_ST2110_20_VIDEOSTD_4096x2160p23:
   case VHD_ST2110_20_VIDEOSTD_4096x2160p24:
   case VHD_ST2110_20_VIDEOSTD_4096x2160p25:
   case VHD_ST2110_20_VIDEOSTD_4096x2160p29:
   case VHD_ST2110_20_VIDEOSTD_4096x2160p30:
   case VHD_ST2110_20_VIDEOSTD_4096x2160p47:
   case VHD_ST2110_20_VIDEOSTD_4096x2160p48:
   case VHD_ST2110_20_VIDEOSTD_4096x2160p50:
   case VHD_ST2110_20_VIDEOSTD_4096x2160p59:
   case VHD_ST2110_20_VIDEOSTD_4096x2160p60:
      frame_width = 4096;
      frame_height = 2160;
      interlaced = false;
      frame_rate = (video_standard == VHD_ST2110_20_VIDEOSTD_4096x2160p23) ? 24 :
         (video_standard == VHD_ST2110_20_VIDEOSTD_4096x2160p24) ? 24 :
         (video_standard == VHD_ST2110_20_VIDEOSTD_4096x2160p25) ? 25 :
         (video_standard == VHD_ST2110_20_VIDEOSTD_4096x2160p29) ? 30 :
         (video_standard == VHD_ST2110_20_VIDEOSTD_4096x2160p30) ? 30 :
         (video_standard == VHD_ST2110_20_VIDEOSTD_4096x2160p47) ? 48 :
         (video_standard == VHD_ST2110_20_VIDEOSTD_4096x2160p48) ? 48 :
         (video_standard == VHD_ST2110_20_VIDEOSTD_4096x2160p50) ? 50 :
         (video_standard == VHD_ST2110_20_VIDEOSTD_4096x2160p59) ? 60 : 60;
      is_us = (video_standard == VHD_ST2110_20_VIDEOSTD_4096x2160p23 ||
         video_standard == VHD_ST2110_20_VIDEOSTD_4096x2160p29 ||
         video_standard == VHD_ST2110_20_VIDEOSTD_4096x2160p47 ||
         video_standard == VHD_ST2110_20_VIDEOSTD_4096x2160p59);
      break;
   default:
      return VHDERR_BADARG;
   }
   return VHDERR_NOERROR;
}

VHD_ERRORCODE configure_nic(HANDLE board_handle, uint32_t ip_address, uint32_t subnet_mask, uint32_t gateway,
                            bool is_dhcp_enabled)
{
   VHD_ERRORCODE result;

   if (!is_dhcp_enabled)
   {
      // Disable DHCP
      result = static_cast<VHD_ERRORCODE>(VHD_DisableDHCP(board_handle, VHD_IP_BRD_ETHERNETPORT_ETH_0));
      if (result != VHDERR_NOERROR)
      {
         std::cout << "Error disabling DHCP: " << to_string(result) << std::endl;
         return result;
      }

      // Set IP address
      result = static_cast<VHD_ERRORCODE>(
          VHD_SetEthernetPortProperty(board_handle, VHD_IP_BRD_ETHERNETPORT_ETH_0, VHD_IP_BRD_EP_IP_ADDR, ip_address));
      if (result != VHDERR_NOERROR)
      {
         std::cout << "Error setting IP address: " << to_string(result) << std::endl;
         return result;
      }

      // Set subnet mask
      result = static_cast<VHD_ERRORCODE>(VHD_SetEthernetPortProperty(
          board_handle, VHD_IP_BRD_ETHERNETPORT_ETH_0, VHD_IP_BRD_EP_SUBNET_MASK, subnet_mask));
      if (result != VHDERR_NOERROR)
      {
         std::cout << "Error setting subnet mask: " << to_string(result) << std::endl;
         return result;
      }

      // Set gateway
      result = static_cast<VHD_ERRORCODE>(VHD_SetEthernetPortProperty(
          board_handle, VHD_IP_BRD_ETHERNETPORT_ETH_0, VHD_IP_BRD_EP_GATEWAY_ADDR, gateway));
      if (result != VHDERR_NOERROR)
      {
         std::cout << "Error setting gateway: " << to_string(result) << std::endl;
         return result;
      }
   }
   else
   {
      // Enable DHCP
      result = static_cast<VHD_ERRORCODE>(VHD_EnableDHCP(board_handle, VHD_IP_BRD_ETHERNETPORT_ETH_0));
      if (result != VHDERR_NOERROR)
      {
         std::cout << "Error enabling DHCP: " << to_string(result) << std::endl;
         return result;
      }
   }

   return VHDERR_NOERROR;
}

VHD_ERRORCODE configure_stream(HANDLE board_handle,
                               HANDLE& stream_handle,
                               VHD_STREAMTYPE stream_type,
                               VHD_ST2110_20_VIDEO_STANDARD video_standard,
                               uint32_t destination_ip,
                               uint32_t destination_ssrc,
                               uint16_t destination_udp_port)
{
   VHD_ERRORCODE result;
   uint32_t frame_width;
   uint32_t frame_height;
   uint32_t frame_rate;
   bool interlaced;
   bool is_us;
   result = get_video_standard_info(video_standard, frame_width, frame_height, frame_rate, interlaced, is_us);
   if (result != VHDERR_NOERROR)
   {
      std::cout << "Error getting video standard info: " << to_string(result) << std::endl;
      return result;
   }
   if (is_us)
      VHD_SetBoardProperty(board_handle, VHD_SDI_BP_CLOCK_SYSTEM, VHD_CLOCKDIV_1001);
   else
      VHD_SetBoardProperty(board_handle, VHD_SDI_BP_CLOCK_SYSTEM, VHD_CLOCKDIV_1);

   result = static_cast<VHD_ERRORCODE>(VHD_OpenStreamHandle(
       board_handle, stream_type, VHD_ST2110_STPROC_DISJOINED_VIDEO, nullptr, &stream_handle, nullptr));
   if (result != VHDERR_NOERROR)
   {
      std::cout << "Error opening stream handle: " << to_string(result) << std::endl;
      return result;
   }

   result = static_cast<VHD_ERRORCODE>(
       VHD_SetStreamProperty(stream_handle, VHD_CORE_SP_BUFFER_PACKING, VHD_BUFPACK_VIDEO_YUV422_8));
   if (result != VHDERR_NOERROR)
   {
      std::cout << "Error setting buffer packing: " << to_string(result) << std::endl;
      VHD_CloseStreamHandle(stream_handle);
      return result;
   }

   result = static_cast<VHD_ERRORCODE>(VHD_SetStreamProperty(stream_handle, VHD_CORE_SP_IO_TIMEOUT, 200));
   if (result != VHDERR_NOERROR)
   {
      std::cout << "Error setting IO timeout: " << to_string(result) << std::endl;
      VHD_CloseStreamHandle(stream_handle);
      return result;
   }

   result = static_cast<VHD_ERRORCODE>(
       VHD_SetStreamProperty(stream_handle, VHD_CORE_SP_TRANSFER_SCHEME, VHD_TRANSFER_SLAVED));
   if (result != VHDERR_NOERROR)
   {
      std::cout << "Error setting transfer scheme: " << to_string(result) << std::endl;
      VHD_CloseStreamHandle(stream_handle);
      return result;
   }

   result = static_cast<VHD_ERRORCODE>(VHD_SetStreamProperty(stream_handle, VHD_IP_BRD_BP_ARP_TIMEOUT, 10000));
   if (result != VHDERR_NOERROR)
   {
      std::cout << "Error setting ARP timeout: " << to_string(result) << std::endl;
      VHD_CloseStreamHandle(stream_handle);
      return result;
   }

   result = static_cast<VHD_ERRORCODE>(VHD_SetStreamProperty(stream_handle, VHD_IP_BRD_SP_IP_DST, destination_ip));
   if (result != VHDERR_NOERROR)
   {
      std::cout << "Error setting destination IP: " << to_string(result) << std::endl;
      VHD_CloseStreamHandle(stream_handle);
      return result;
   }

   result = static_cast<VHD_ERRORCODE>(VHD_SetStreamProperty(stream_handle, VHD_IP_BRD_SP_SSRC, destination_ssrc));
   if (result != VHDERR_NOERROR)
   {
      std::cout << "Error setting SSRC: " << to_string(result) << std::endl;
      VHD_CloseStreamHandle(stream_handle);
      return result;
   }

   result = static_cast<VHD_ERRORCODE>(
       VHD_SetStreamProperty(stream_handle, VHD_IP_BRD_SP_UDP_PORT_DST, destination_udp_port));
   if (result != VHDERR_NOERROR)
   {
      std::cout << "Error setting destination UDP port: " << to_string(result) << std::endl;
      VHD_CloseStreamHandle(stream_handle);
      return result;
   }

   result = static_cast<VHD_ERRORCODE>(
       VHD_SetStreamProperty(stream_handle, VHD_ST2110_20_SP_SAMPLING, VHD_ST2110_20_SAMPLING_YUV_422));
   if (result != VHDERR_NOERROR)
   {
      std::cout << "Error setting sampling: " << to_string(result) << std::endl;
      VHD_CloseStreamHandle(stream_handle);
      return result;
   }

   result = static_cast<VHD_ERRORCODE>(
       VHD_SetStreamProperty(stream_handle, VHD_ST2110_20_SP_DEPTH, VHD_ST2110_20_DEPTH_10BIT));
   if (result != VHDERR_NOERROR)
   {
      std::cout << "Error setting depth: " << to_string(result) << std::endl;
      VHD_CloseStreamHandle(stream_handle);
      return result;
   }

   result = static_cast<VHD_ERRORCODE>(VHD_SetStreamProperty(
       stream_handle, VHD_ST2110_20_SP_TRAFFIC_SHAPING_MODE, VHD_ST2110_20_TRAFFIC_SHAPING_MODE_LINEAR));
   if (result != VHDERR_NOERROR)
   {
      std::cout << "Error setting traffic shaping mode: " << to_string(result) << std::endl;
      VHD_CloseStreamHandle(stream_handle);
      return result;
   }

   result = static_cast<VHD_ERRORCODE>(
       VHD_SetStreamProperty(stream_handle, VHD_ST2110_20_SP_VIDEO_STANDARD, video_standard));
   if (result != VHDERR_NOERROR)
   {
      std::cout << "Error setting video standard: " << to_string(result) << std::endl;
      VHD_CloseStreamHandle(stream_handle);
      return result;
   }

   return VHDERR_NOERROR;
}

VHD_ERRORCODE configure_stream_from_sdp(HANDLE board_handle,
                                        std::string sdp,
                                        const uint32_t destination_ip_overrides,
                                        const uint16_t destination_udp_port_overrides,
                                        HANDLE stream_handle,
                                        uint32_t& multicast_group)
{
   VHD_ERRORCODE result;
   HANDLE sdp_parser_handle = NULL;
   ULONG video_standard, sampling, depth, udp_port, ip_address;

   multicast_group = 0u;

   // const-cast
   result = static_cast<VHD_ERRORCODE>(
       VHD_OpenSDPParserHandle(const_cast<char*>(sdp.c_str()), static_cast<ULONG>(sdp.length()), &sdp_parser_handle));
   if (result != VHDERR_NOERROR)
   {
      std::cout << "Error opening SDP parser handle: " << to_string(result) << std::endl;
      return result;
   }

   result = static_cast<VHD_ERRORCODE>(
       VHD_GetSDPStreamProperty(sdp_parser_handle, VHD_SDP_STREAM_PROPERTY_VIDEO_STANDARD, &video_standard));
   if (result != VHDERR_NOERROR)
   {
      std::cout << "Error getting video standard: " << to_string(result) << std::endl;
      VHD_CloseSDPParserHandle(sdp_parser_handle);
      return result;
   }

   result = static_cast<VHD_ERRORCODE>(
       VHD_GetSDPStreamProperty(sdp_parser_handle, VHD_SDP_STREAM_PROPERTY_VIDEO_SAMPLING, &sampling));
   if (result != VHDERR_NOERROR)
   {
      std::cout << "Error getting video sampling: " << to_string(result) << std::endl;
      VHD_CloseSDPParserHandle(sdp_parser_handle);
      return result;
   }

   result = static_cast<VHD_ERRORCODE>(
       VHD_GetSDPStreamProperty(sdp_parser_handle, VHD_SDP_STREAM_PROPERTY_VIDEO_BIT_DEPTH, &depth));
   if (result != VHDERR_NOERROR)
   {
      std::cout << "Error getting video bit depth: " << to_string(result) << std::endl;
      VHD_CloseSDPParserHandle(sdp_parser_handle);
      return result;
   }

   result = static_cast<VHD_ERRORCODE>(
       VHD_GetSDPStreamProperty(sdp_parser_handle, VHD_SDP_STREAM_PROPERTY_VIDEO_DESTINATION_UDP_PORT, &udp_port));
   if (result != VHDERR_NOERROR)
   {
      std::cout << "Error getting UDP port: " << to_string(result) << std::endl;
      VHD_CloseSDPParserHandle(sdp_parser_handle);
      return result;
   }

   result = static_cast<VHD_ERRORCODE>(
       VHD_GetSDPStreamProperty(sdp_parser_handle, VHD_SDP_STREAM_PROPERTY_VIDEO_DESTINATION_IP_ADDRESS, &ip_address));
   if (result != VHDERR_NOERROR)
   {
      std::cout << "Error getting IP address: " << to_string(result) << std::endl;
      VHD_CloseSDPParserHandle(sdp_parser_handle);
      return result;
   }

   // Override destination IP and UDP port if provided
   if (destination_ip_overrides != 0)
   {
      ip_address = destination_ip_overrides;
   }
   if (destination_udp_port_overrides != 0)
   {
      udp_port = destination_udp_port_overrides;
   }

   // Configure board properties
   result = static_cast<VHD_ERRORCODE>(VHD_SetBoardProperty(board_handle, VHD_IP_BRD_BP_RX0_UDP_PORT, udp_port));
   if (result != VHDERR_NOERROR)
   {
      std::cout << "Error setting board UDP port: " << to_string(result) << std::endl;
      VHD_CloseSDPParserHandle(sdp_parser_handle);
      return result;
   }

   if ((ip_address & 0xF0000000) == 0xE0000000)
   { // Check if IP address is multicast
      // Join multicast group
      result =
          static_cast<VHD_ERRORCODE>(VHD_JoinMulticastGroup(board_handle, VHD_IP_BRD_ETHERNETPORT_ETH_0, ip_address));
      if (result != VHDERR_NOERROR)
      {
         std::cout << "Error joining multicast group: " << to_string(result) << std::endl;
         VHD_CloseSDPParserHandle(sdp_parser_handle);
         return result;
      }
      multicast_group = ip_address;
   }

   // Configure stream properties
   result = static_cast<VHD_ERRORCODE>(
       VHD_SetStreamProperty(stream_handle, VHD_CORE_SP_TRANSFER_SCHEME, VHD_TRANSFER_SLAVED));
   if (result != VHDERR_NOERROR)
   {
      std::cout << "Error setting transfer scheme: " << to_string(result) << std::endl;
      VHD_CloseSDPParserHandle(sdp_parser_handle);
      return result;
   }

   result = static_cast<VHD_ERRORCODE>(
       VHD_SetStreamProperty(stream_handle, VHD_ST2110_20_SP_VIDEO_STANDARD, video_standard));
   if (result != VHDERR_NOERROR)
   {
      std::cout << "Error setting video standard: " << to_string(result) << std::endl;
      VHD_CloseSDPParserHandle(sdp_parser_handle);
      return result;
   }

   result = static_cast<VHD_ERRORCODE>(VHD_SetStreamProperty(stream_handle, VHD_ST2110_20_SP_SAMPLING, sampling));
   if (result != VHDERR_NOERROR) {
      std::cout << "Error setting sampling: " << to_string(result) << std::endl;
      VHD_CloseSDPParserHandle(sdp_parser_handle);
      return result;
   }

   result = static_cast<VHD_ERRORCODE>(VHD_SetStreamProperty(stream_handle, VHD_ST2110_20_SP_DEPTH, depth));
   if (result != VHDERR_NOERROR) {
      std::cout << "Error setting depth: " << to_string(result) << std::endl;
      VHD_CloseSDPParserHandle(sdp_parser_handle);
      return result;
   }

   result = static_cast<VHD_ERRORCODE>(VHD_SetStreamProperty(stream_handle, VHD_CORE_SP_BUFFER_PACKING, VHD_BUFPACK_VIDEO_YUV422_10));
   if (result != VHDERR_NOERROR) {
      std::cout << "Error setting buffer packing: " << to_string(result) << std::endl;
      VHD_CloseSDPParserHandle(sdp_parser_handle);
      return result;
   }

   // Set destination IP for filtering
   result = static_cast<VHD_ERRORCODE>(VHD_SetBoardProperty(board_handle, VHD_IP_BRD_SP_IP_DST, ip_address));
   if (result != VHDERR_NOERROR) {
      std::cout << "Error setting VHD_IP_BRD_SP_IP_DST: " << to_string(result) << std::endl;
      VHD_CloseSDPParserHandle(sdp_parser_handle);
      return result;
   }

   // Set destination IP for SPS filtering
   result = static_cast<VHD_ERRORCODE>(VHD_SetBoardProperty(board_handle, VHD_IP_BRD_SP_SPS_IP_DST, ip_address));
   if (result != VHDERR_NOERROR) {
      std::cout << "Error setting VHD_IP_BRD_SP_SPS_IP_DST: " << to_string(result) << std::endl;
      VHD_CloseSDPParserHandle(sdp_parser_handle);
      return result;
   }

   // Set UDP port for filtering
   result = static_cast<VHD_ERRORCODE>(VHD_SetBoardProperty(board_handle, VHD_IP_BRD_SP_UDP_PORT_DST, udp_port));
   if (result != VHDERR_NOERROR) {
      std::cout << "Error setting VHD_IP_BRD_SP_UDP_PORT_DST: " << to_string(result) << std::endl;
      VHD_CloseSDPParserHandle(sdp_parser_handle);
      return result;
   }

   // Set filtering mask
   const ULONG FilteringMask_UL = VHD_IP_FILTER_IP_ADDR_DEST | VHD_IP_FILTER_UDP_PORT_DEST;
   result =
       static_cast<VHD_ERRORCODE>(VHD_SetBoardProperty(board_handle, VHD_IP_BRD_SP_FILTERING_MASK, FilteringMask_UL));
   if (result != VHDERR_NOERROR) {
      std::cout << "Error setting VHD_IP_BRD_SP_FILTERING_MASK: " << to_string(result) << std::endl;
      VHD_CloseSDPParserHandle(sdp_parser_handle);
      return result;
   }

   // Print configuration
   std::cout << "Configuration:" << std::endl;
   std::cout << "\tVideo:" << std::endl;
   std::cout << "\t\tVideo standard: "
             << VHD_ST2110_20_VIDEO_STANDARD_ToPrettyString(static_cast<VHD_ST2110_20_VIDEO_STANDARD>(video_standard))
             << std::endl;
   std::cout << "\t\tVideo sampling: "
             << VHD_ST2110_20_SAMPLING_ToPrettyString(static_cast<VHD_ST2110_20_SAMPLING>(sampling)) << std::endl;
   std::cout << "\t\tDepth: " << VHD_ST2110_20_DEPTH_ToPrettyString(static_cast<VHD_ST2110_20_DEPTH>(depth))
             << std::endl;
   std::cout << "\tUDP port: " << udp_port << std::endl;
   std::cout << "\tIP address: " << ip_address << std::endl;

   VHD_CloseSDPParserHandle(sdp_parser_handle);
   return VHDERR_NOERROR;
}

VHD_ERRORCODE generate_sdp(HANDLE board_handle, HANDLE stream_handle, std::string& sdp)
{
   HANDLE sdp_parser_handle = nullptr;
   std::vector<char> sdp_buffer(4096, 0);
   VHD_ERRORCODE result;
   const char *version = "0";
   const char *originator = "- 30676506 46291097 IN IP4  ";
   const char *description = "VHD Stream";
   const char *colorimetry = "BT709";

   char work_buffer[256]; // VHD don't support const char* buffers

   ULONG ip_address;
   ULONG dest_ip_address;
   ULONG dest_port;
   ULONG video_standard;
   ULONG bit_depth;

   result = static_cast<VHD_ERRORCODE>(
       VHD_GetEthernetPortProperty(board_handle, VHD_IP_BRD_ETHERNETPORT_ETH_0, VHD_IP_BRD_EP_IP_ADDR, &ip_address));
   if (result != VHDERR_NOERROR)
   {
      std::cout << "Error getting IP address: " << to_string(result) << std::endl;
      return result;
   }
   result = static_cast<VHD_ERRORCODE>(
       VHD_GetStreamProperty(stream_handle, VHD_IP_BRD_SP_IP_DST, &dest_ip_address));
   if (result != VHDERR_NOERROR)
   {
      std::cout << "Error getting destination IP address: " << to_string(result) << std::endl;
      return result;
   }
   result = static_cast<VHD_ERRORCODE>(
       VHD_GetStreamProperty(stream_handle, VHD_IP_BRD_SP_UDP_PORT_DST, &dest_port));
   if (result != VHDERR_NOERROR)
   {
      std::cout << "Error getting destination UDP port: " << to_string(result) << std::endl;
      return result;
   }
   result = static_cast<VHD_ERRORCODE>(
       VHD_GetStreamProperty(stream_handle, VHD_ST2110_20_SP_VIDEO_STANDARD, &video_standard));
   if (result != VHDERR_NOERROR)
   {
      std::cout << "Error getting video standard: " << to_string(result) << std::endl;
      return result;
   }
   result = static_cast<VHD_ERRORCODE>(
       VHD_GetStreamProperty(stream_handle, VHD_ST2110_20_SP_DEPTH, &bit_depth));
   if (result != VHDERR_NOERROR)
   {
      std::cout << "Error getting bit depth: " << to_string(result) << std::endl;
      return result;
   }

   result = static_cast<VHD_ERRORCODE>(
       VHD_OpenSDPParserHandle(work_buffer, 0, &sdp_parser_handle));
   if (result != VHDERR_NOERROR)
   {
      std::cout << "Error opening SDP parser handle: " << to_string(result) << std::endl;
      return result;
   }

   std::strcpy(work_buffer, version);
   VHD_AddSDPField(sdp_parser_handle,
                   VHD_SDP_DESCRIPTION_TYPE_SESSION,
                   0,
                   VHD_SDP_SESSION_FIELD_VERSION,
                   work_buffer,
                   static_cast<ULONG>(strlen(work_buffer)));

   std::strcpy(work_buffer, originator);
   VHD_AddSDPField(sdp_parser_handle,
                   VHD_SDP_DESCRIPTION_TYPE_SESSION,
                   0,
                   VHD_SDP_SESSION_FIELD_ORIGINATOR,
                   work_buffer,
                   static_cast<ULONG>(strlen(work_buffer)));

   std::strcpy(work_buffer, description);
   VHD_AddSDPField(sdp_parser_handle,
                   VHD_SDP_DESCRIPTION_TYPE_SESSION,
                   0,
                   VHD_SDP_SESSION_FIELD_NAME,
                   work_buffer,
                   static_cast<ULONG>(strlen(work_buffer)));

   VHD_SetSDPStreamProperty(sdp_parser_handle, VHD_SDP_STREAM_PROPERTY_SOURCE_IP_ADDRESS, ip_address);
   VHD_SetSDPStreamProperty(sdp_parser_handle, VHD_SDP_STREAM_PROPERTY_START_TIME, 0);
   VHD_SetSDPStreamProperty(sdp_parser_handle, VHD_SDP_STREAM_PROPERTY_END_TIME, 0);

   VHD_SetSDPStreamProperty(sdp_parser_handle, VHD_SDP_STREAM_PROPERTY_VIDEO_RTP_PAYLOAD_ID, 98);
   VHD_SetSDPStreamProperty(sdp_parser_handle, VHD_SDP_STREAM_PROPERTY_VIDEO_DESTINATION_UDP_PORT, dest_port);
   VHD_SetSDPStreamProperty(sdp_parser_handle, VHD_SDP_STREAM_PROPERTY_VIDEO_STANDARD, video_standard);
   VHD_SetSDPStreamProperty(sdp_parser_handle, VHD_SDP_STREAM_PROPERTY_VIDEO_SAMPLING, VHD_ST2110_20_SAMPLING_YUV_422);
   VHD_SetSDPStreamProperty(sdp_parser_handle, VHD_SDP_STREAM_PROPERTY_VIDEO_BIT_DEPTH, bit_depth);
   std::strcpy(work_buffer, colorimetry);
   VHD_SetSDPStringStreamProperty(sdp_parser_handle,
                                  VHD_SDP_STREAM_PROPERTY_VIDEO_COLORIMETRY,
                                  work_buffer,
                                  static_cast<ULONG>(strlen(work_buffer)));
   VHD_SetSDPStreamProperty(sdp_parser_handle, VHD_SDP_STREAM_PROPERTY_VIDEO_DESTINATION_IP_ADDRESS, dest_ip_address);
   VHD_SetSDPStreamProperty(sdp_parser_handle,
                            VHD_SDP_STREAM_PROPERTY_VIDEO_TRAFFIC_SHAPING_SENDER_TYPE,
                            VHD_ST2110_20_TRAFFIC_SHAPING_MODE_LINEAR);

   ULONG buffer_size = sdp_buffer.size();
   result = static_cast<VHD_ERRORCODE>(VHD_GetSDPBuffer(sdp_parser_handle, sdp_buffer.data(), &buffer_size));
   if (result != VHDERR_NOERROR || buffer_size > sdp_buffer.size())
   {
      VHD_CloseSDPParserHandle(sdp_parser_handle);
      std::cout << "Error getting SDP buffer: " << to_string(result) << std::endl;
      return result;
   }
   VHD_CloseSDPParserHandle(sdp_parser_handle);

   sdp = std::string(sdp_buffer.data(), buffer_size);

   return result;
}

VHD_ERRORCODE leave_multicast(HANDLE board, uint32_t& multicast_group)
{
    VHD_ERRORCODE result = VHDERR_NOERROR;
    if (multicast_group != 0u)
    {
        result = static_cast<VHD_ERRORCODE>(VHD_LeaveMulticastGroup(board, VHD_IP_BRD_ETHERNETPORT_ETH_0, multicast_group));
        if (result != VHDERR_NOERROR)
            std::cout << "Error when leaving the multicast group" << " [" << to_string(result) << "]" << std::endl;
        multicast_group = 0u;
    }
    return result;
}

VHD_ERRORCODE apply_ptp_parameters(HANDLE board_handle, uint8_t domain_number, uint8_t announce_receipt_timeout)
{
   VHD_ERRORCODE result;
   VHD_PTP_CONFIGURATION ptp_config;
   VHD_PTP_DEFAULTDS default_ds;
   VHD_PTP_PORTDS port_ds;

   // Create PTP datasets based on ST2059-2 profile
   result = static_cast<VHD_ERRORCODE>(VHD_CreatePTPDSFromProfile(VHD_PTP_PROFILE_ST2059_2, &default_ds, &port_ds));
   if (result != VHDERR_NOERROR)
   {
      std::cout << std::endl << "Error when creating PTP datasets from ST2059-2 profile" << " [" << to_string(result) << "]" << std::endl;
      return result;
   }

   // Get the current PTP configuration
   result = static_cast<VHD_ERRORCODE>(VHD_GetPTPConfiguration(board_handle, &ptp_config));
   if (result != VHDERR_NOERROR)
   {
      std::cout << std::endl << "Error when getting the PTP config" << " [" << to_string(result) << "]" << std::endl;
      return result;
   }

   // Update the PTP configuration with the ST2059-2 profile and user-specified parameters
   ptp_config.DefaultDS = default_ds;
   ptp_config.PortDS = port_ds;
   ptp_config.DefaultDS.DomainNumber = domain_number;
   ptp_config.PortDS.AnnounceReceiptTimeout = announce_receipt_timeout;

   // Set the updated PTP configuration
   result = static_cast<VHD_ERRORCODE>(VHD_SetPTPConfiguration(board_handle, ptp_config));
   if (result != VHDERR_NOERROR)
   {
      std::cout << std::endl << "Error when setting the PTP config" << " [" << to_string(result) << "]" << std::endl;
   }

   return result;
}

VHD_ERRORCODE print_ptp_status(HANDLE board_handle, uint8_t domain_number, uint8_t announce_receipt_timeout)
{
    VHD_ERRORCODE result;
    VHD_PTP_PORT_STATE ptp_state;
    BOOL32 locked;
    LONG offset_sec = 0u, offset_nsec = 0u;

    result = static_cast<VHD_ERRORCODE>(VHD_GetPTPPortState(board_handle, &ptp_state, &locked));
    if (result != VHDERR_NOERROR)
    {
        std::cout << std::endl << "Error when getting the PTP port state" << " [" << to_string(result) << "]" << std::endl;
        return result;
    }

    result = static_cast<VHD_ERRORCODE>(VHD_GetPTPOffset(board_handle, &offset_sec, &offset_nsec));
    if (result != VHDERR_NOERROR)
    {
        std::cout << std::endl << "Error when getting the PTP offset" << " [" << to_string(result) << "]" << std::endl;
        return result;
    }

    // Calculate total offset in seconds
    double total_offset = offset_sec + (offset_nsec / 1e9);

    std::cout << "PTP : Domain Number = " << static_cast<int>(domain_number)
              << " Announce Receipt Timeout = " << static_cast<int>(announce_receipt_timeout)
              << " State = " << to_string(ptp_state)
              << " (Offset : " << total_offset << " seconds)                 \r" << std::flush;

    return VHDERR_NOERROR;
}

using namespace std::chrono_literals;
void monitor_rx_stream_status(HANDLE stream_handle, bool* request_stop, uint32_t* timeout)
{
   ULONG SlotsCount = 0;
   ULONG SlotsDropped = 0;
   ULONG JitterMax = 0;
   ULONG DatagramCount = 0;

   while (!*request_stop)
   {
      VHD_GetStreamProperty(stream_handle, VHD_CORE_SP_SLOTS_COUNT, &SlotsCount);
      VHD_GetStreamProperty(stream_handle, VHD_CORE_SP_SLOTS_DROPPED, &SlotsDropped);
      VHD_GetStreamProperty(stream_handle, VHD_IP_BRD_SP_JITTER_MAX, &JitterMax);
      VHD_GetStreamProperty(stream_handle, VHD_IP_BRD_SP_DATAGRAM_COUNT, &DatagramCount);

      std::cout << "SlotCount: " << SlotsCount << " - SlotDropped: " << SlotsDropped
                << " - JitterMax: " << JitterMax << " - DatagramCount: " << DatagramCount
                << " - Timeout: " << *timeout << "                      \r" << std::flush;

      std::this_thread::sleep_for(100ms);
   }
}

void monitor_tx_stream_status(HANDLE stream_handle, bool* request_stop)
{
   ULONG SlotsCount = 0;
   ULONG SlotsDropped = 0;
   ULONG JitterMax = 0;
   ULONG DatagramCount = 0;

   while (!*request_stop)
   {
      VHD_GetStreamProperty(stream_handle, VHD_CORE_SP_SLOTS_COUNT, &SlotsCount);
      VHD_GetStreamProperty(stream_handle, VHD_CORE_SP_SLOTS_DROPPED, &SlotsDropped);
      VHD_GetStreamProperty(stream_handle, VHD_IP_BRD_SP_JITTER_MAX, &JitterMax);
      VHD_GetStreamProperty(stream_handle, VHD_IP_BRD_SP_DATAGRAM_COUNT, &DatagramCount);

      std::cout << "SlotCount: " << SlotsCount
                << " - SlotDropped: " << SlotsDropped
                << " - JitterMax: " << JitterMax
                << " - DatagramCount: " << DatagramCount << "                      \r" << std::flush;

      std::this_thread::sleep_for(100ms);
   }
}
