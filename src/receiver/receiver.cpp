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

#include <iostream>
#include <vector>
#include <string>
#include <thread>

#if defined(__GNUC__) && !(defined(__APPLE__))
#include <stdint-gcc.h>
#else
#include <stdint.h>
#endif

#if defined (__linux__) || defined (__APPLE__)
#include "../keyboard.h"
#else
#include <conio.h>
#define init_keyboard()
#define close_keyboard()
#endif

#include "nmos/model.h"
#include "nmos/log_model.h"
#include "nmos/log_gate.h"
#include "nmos/server.h"
#include "nmos/node_server.h"

#include "../tools.h"
#include "../nmos_tools.h"

#if defined(__APPLE__)
#include "VideoMasterHD/VideoMasterHD_Core.h"
#include "VideoMasterHD/VideoMasterHD_Ip_ST2110_Board.h"
#include "VideoMasterHD/VideoMasterHD_Ip_ST2110_20.h"
#include "cpprest/host_utils.h"
#else
#include "VideoMasterHD_Core.h"
#include "VideoMasterHD_Ip_ST2110_Board.h"
#include "VideoMasterHD_Ip_ST2110_20.h"
#endif

int main(int argc, char* argv[])
{
   //VHD parameters
   const uint32_t board_id = 0;

   //Media NIC
   const std::string media_nic_name = "delta" + std::to_string(board_id);  //Streaming network interface controller
   const uint32_t media_nic_ip = 0xc0a80002; //Streaming network interface controller IP address
   const uint32_t media_nic_gateway = 0x00000000; // Streaming network interface controller gateway
   const uint32_t media_nic_subnet_mask = 0xffffff00; // Streaming network interface controller subnet mask
   const bool media_nic_dhcp = false; // Streaming network interface controller DHCP enabled
   
   //NMOS parameters
   const std::string management_nic_ip = "192.168.0.10"; //Management network interface controller
   const uint32_t default_destination_address = 0xef0a0a01; //default IP destination address used for resolving "auto" nmos parameter
   const uint16_t default_destination_udp_port = 1025; //default UDP destination port used for resolving "auto" nmos parameter IP address

   //Node parameters
   const std::string node_label = "VHD Rx Node";
   const std::string node_description = "Deltacast IP Card NMOS RX Demonstration Sample";
   const std::string device_name = "VHD Rx Device";
   const std::string device_description = "Deltacast IP Card RX Device";
   const std::string node_domain = "local.domain."; //domain name where your machine is located
   const int node_api_port = 3212; //port used by the node to expose its registry API
   const int connection_api_port = 3215; //port used by the node to expose its connection API


   HANDLE board = nullptr, stream = nullptr, slot = nullptr;
   VHD_STREAMTYPE stream_type = VHD_ST_RX0;
   VHD_ERRORCODE result = VHDERR_NOERROR;

   uint32_t frame_width;
   uint32_t frame_height;
   uint32_t frame_rate;
   bool     interlaced;
   bool     is_us;

   std::string media_nic_mac_address;

   //Buffer that will be created and filled by the API
   uint8_t* buffer = nullptr;
   ULONG buffer_size = 0, index = 0;
   uint32_t multicast_group = 0u;

   bool exit = false;

   init_keyboard();

   std::cout << "DELTA-IP NMOS ST2110-20 RECEPTION SAMPLE APPLICATION\n(c) DELTACAST\n--------------------------------------------------------"
      << std::endl << std::endl;

   nmos_tools::NodeServerReceiver::TransportParams resolve_auto_transport_params;

   result = static_cast<VHD_ERRORCODE>(VHD_OpenBoardHandle(board_id, &board, nullptr, 0ul));
   if (result != VHDERR_NOERROR)
   {
      std::cout << "Error when opening the board handle" << " [" << to_string(result) << "]" << std::endl;
   }

   if(result == VHDERR_NOERROR)
   {
      result = configure_nic(board, media_nic_ip, media_nic_subnet_mask, media_nic_gateway, media_nic_dhcp);
      if(result != VHDERR_NOERROR)
      {
         std::cout << "Error when configuring the NIC" << " [" << to_string(result) << "]" << std::endl;
      }
      resolve_auto_transport_params.ip_interface = media_nic_ip;
      resolve_auto_transport_params.ip_multicast = default_destination_address;
      resolve_auto_transport_params.ip_src = 0; //no filtering on source ip
      resolve_auto_transport_params.port_dst = default_destination_udp_port;
   }
   if (result == VHDERR_NOERROR)
   {
      result = get_nic_mac_address(board, media_nic_mac_address);
      if(result != VHDERR_NOERROR)
      {
         std::cout << "Error when getting the MAC address" << " [" << to_string(result) << "]" << std::endl;
      }
   }

   nmos::node_model node_model;
   nmos::experimental::log_model log_model;
   nmos::experimental::node_implementation node_implementation;

   std::filebuf error_log_buf;
   std::ostream error_log(std::cerr.rdbuf());
   std::filebuf access_log_buf;
   std::ostream access_log(&access_log_buf);

   nmos::experimental::log_gate gate(error_log, access_log, log_model);

   web::json::value hostAddresses = web::json::value::array();
   hostAddresses[0] = web::json::value::string(utility::conversions::to_string_t(management_nic_ip));
   node_model.settings[nmos::fields::host_addresses] = hostAddresses;
   nmos::insert_node_default_settings(node_model.settings);
#if defined(__APPLE__)
   std::string host_name = utility::conversions::to_utf8string(web::hosts::experimental::host_name());
   node_model.settings[nmos::fields::host_name] = web::json::value::string(utility::conversions::to_string_t(host_name));
#endif
   node_model.settings[nmos::experimental::fields::seed_id] = web::json::value::string(U("f885a687-71e9-4b7b-a8da-bfd730cd2839"));
   node_model.settings[nmos::fields::label] = web::json::value::string(utility::conversions::to_string_t(node_label));
   node_model.settings[nmos::fields::description] = web::json::value::string(utility::conversions::to_string_t(node_description));
   node_model.settings[nmos::fields::domain] = web::json::value::string(utility::conversions::to_string_t(node_domain));
   node_model.settings[nmos::fields::node_port] = web::json::value::number(node_api_port);
   node_model.settings[nmos::fields::connection_port] = web::json::value::number(connection_api_port);
   node_model.settings[nmos::fields::logging_level] = slog::severities::warning; //change this to change the logging level

   log_model.settings = node_model.settings;
   log_model.level = nmos::fields::logging_level(log_model.settings);

   nmos_tools::NodeServerReceiver::TransportParams active_transport_params = resolve_auto_transport_params;
   nmos_tools::NodeServerReceiver node_server(node_model, log_model, gate, device_name, device_description,
      resolve_auto_transport_params, active_transport_params, media_nic_name, media_nic_mac_address);
   
   if(!node_server.node_implementation_init())
   {
      result = VHDERR_OPERATIONFAILED;
      std::cout << "Error when initializing the node server" << " [" << to_string(result) << "]" << std::endl;
   }

   if (result == VHDERR_NOERROR)
   {
      node_server.start();
      std::cout << "NMOS: node ready for connections" << std::endl;
   }

   nmos_tools::NodeServerReceiver::TransportParams previous_transport_params = resolve_auto_transport_params;
   std::string previous_sdp = "INVALID SDP";

   //Get the system parameters and apply new PTP parameters
   nmos_tools::NmosPtpSystemParameters ptp_system_parameters;
   nmos_tools::NmosPtpSystemParameters previous_ptp_system_parameters;

   while(result == VHDERR_NOERROR && !exit){

      //Wait for the stream to be enabled
      while(node_server.is_enabled == false)
      {
         if (_kbhit())
         { 
            _getch();
            exit = true;
            break;
         }

         //While the stream is disabled, we have to react to PTP changes
         if(node_server.get_ptp_system_parameters(ptp_system_parameters)) //if get_ptp_system_parameters returns false,
                                                                          //it means that the PTP system parameters are not available
         {
            if(ptp_system_parameters != previous_ptp_system_parameters)
            {
               std::cout << "Applying new PTP parameters: domain=" << ptp_system_parameters.domain_number << std::endl;
               //ptp_system_parameters were changed, we need to update the ptp configuration
               result = apply_ptp_parameters(board, static_cast<uint8_t>(ptp_system_parameters.domain_number),
                  static_cast<uint8_t>(ptp_system_parameters.announce_receipt_timeout));
               if(result != VHDERR_NOERROR)
               {
                  exit = true;
                  break;
               }

               previous_ptp_system_parameters = ptp_system_parameters;
            }
            print_ptp_status(board, static_cast<uint8_t>(ptp_system_parameters.domain_number),
               static_cast<uint8_t>(ptp_system_parameters.announce_receipt_timeout));
         }

         std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }

      //to not start and stop the transmission
      if(exit)
         break;

      std::string sdp = node_server.get_sdp();
      if(previous_transport_params != active_transport_params || sdp != previous_sdp)
      {
         std::cout << "New transport parameters received or SDP changed" << std::endl;

         //active parameters were changed, we need to update the stream
         if (stream != nullptr) {
            result = static_cast<VHD_ERRORCODE>(VHD_CloseStreamHandle(stream));
            if (result != VHDERR_NOERROR)
               std::cout << "Error when destroying the stream" << " [" << to_string(result) << "]" << std::endl;
         }
         if (result == VHDERR_NOERROR)
         {
            result = static_cast<VHD_ERRORCODE>(VHD_OpenStreamHandle(board, stream_type, VHD_ST2110_STPROC_DISJOINED_VIDEO, nullptr, &stream, nullptr));
            if (result != VHDERR_NOERROR)
               std::cout << "Error when creating stream" << " [" << to_string(result) << "]" << std::endl;
         }
         leave_multicast(board, multicast_group);
         if(result == VHDERR_NOERROR)
         {
            result = configure_stream_from_sdp(board, sdp, active_transport_params.ip_multicast, active_transport_params.port_dst, stream, multicast_group);
            previous_transport_params = active_transport_params;
            previous_sdp = sdp;
            if (result != VHDERR_NOERROR)
               std::cout << "Error when configuring stream" << " [" << to_string(result) << "]" << std::endl;
         }
         if (result == VHDERR_NOERROR)
         {
            ULONG video_standard;
            result = static_cast<VHD_ERRORCODE>(VHD_GetStreamProperty(stream, VHD_ST2110_20_SP_VIDEO_STANDARD, &video_standard));
            if (result == VHDERR_NOERROR)
            {
               result = get_video_standard_info(static_cast<VHD_ST2110_20_VIDEO_STANDARD>(video_standard), frame_width, frame_height, frame_rate, interlaced, is_us);
               if (result != VHDERR_NOERROR)
                  std::cout << "Error when getting video standard info" << std::endl;
            }
         }

      }

      if(result == VHDERR_NOERROR)
      {
         result = static_cast<VHD_ERRORCODE>(VHD_StartStream(stream));
         if (result != VHDERR_NOERROR)
            std::cout << "Error when starting stream" << " [" << to_string(result) << "]" << std::endl;
      }

      if(result == VHDERR_NOERROR)
      {
         std::cout << std::endl << "Received Sdp : " << std::endl << sdp << std::endl;
         std::cout << std::endl << "Reception started, press any key to stop..." << std::endl;

         uint32_t slot_timeout = 0;

         bool stop_monitoring = false;
         std::thread monitoring_thread (monitor_rx_stream_status, stream, &stop_monitoring, &slot_timeout);
         
         //Reception loop
         while (1)
         {
            if (_kbhit())
            {
               _getch();
               exit = true;
               break;
            }

            sdp = node_server.get_sdp();
            if(!node_server.is_enabled)
            {
               std::cout << "node server disabled; exit reception loop" << std::endl;
               break;
            }
            if (previous_transport_params != active_transport_params)
            {
               std::cout << "active transport params changed; exit reception loop" << std::endl;
               break;
            }
            if (sdp != previous_sdp)
            {
               std::cout << "sdp changed; exit reception loop" << std::endl;
               break;
            }

            //Try to lock the next slot.
            result = static_cast<VHD_ERRORCODE>(VHD_LockSlotHandle(stream, &slot));
            if (result != VHDERR_NOERROR)
            {
               if (result == VHDERR_TIMEOUT)
               {
                  slot_timeout++;
                  result = VHDERR_NOERROR; //After the above print message, timeout error is considered as handled
                  continue;
               }
               std::cout << "Error when locking slot " << index << " [" << to_string(result) << "]" << std::endl;
               break;
            }

            //Get the video buffer associated to the slot.
            result = static_cast<VHD_ERRORCODE>(VHD_GetSlotBuffer(slot, VHD_ST2110_BT_VIDEO, &buffer, &buffer_size));
            if (result != VHDERR_NOERROR)
            {
               std::cout << "Error when getting slot buffer at slot " << index << " [" << to_string(result) << "]" << std::endl;
            }

            //Unlock the slot. buffer wont be available anymore
            result = static_cast<VHD_ERRORCODE>(VHD_UnlockSlotHandle(slot));
            if (result != VHDERR_NOERROR)
            {
               std::cout << "Error when unlocking slot at slot " << index << " [" << to_string(result) << "]" << std::endl;
               break;
            }

            index++;
         }

         stop_monitoring = true;
         monitoring_thread.join();

         VHD_ERRORCODE result_stop_stream; //temporary variable to not overwrite result if an error occured in the transmission loop

         result_stop_stream = static_cast<VHD_ERRORCODE>(VHD_StopStream(stream));
         if (result_stop_stream != VHDERR_NOERROR)
         {
            std::cout << "Error when stopping the stream" << " [" << to_string(result_stop_stream) << "]" << std::endl;
            result = result_stop_stream;
         }
      }
   }
   
   if(stream)
   {
      result = static_cast<VHD_ERRORCODE>(VHD_CloseStreamHandle(stream));
      if (result != VHDERR_NOERROR)
         std::cout << "Error when closing the stream" << " [" << to_string(result) << "]" << std::endl;
   }

   if (board)
   {
      leave_multicast(board, multicast_group);
      result = static_cast<VHD_ERRORCODE>(VHD_CloseBoardHandle(board));
      if (result != VHDERR_NOERROR)
         std::cout << "Error when closing the board handle" << " [" << to_string(result) << "]" << std::endl;
   }

   close_keyboard();

   node_server.stop();

   return 0;
}