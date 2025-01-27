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

#pragma once
/*!
   @file Tools.h
   @brief This file contains some helping functions used in the samples.
*/

#if defined(__GNUC__) && !(defined(__APPLE__))
#include <stdint-gcc.h>
#else
#include <stdint.h>
#endif

#include <iostream>
#include <string>
#include <vector>

#if defined(__APPLE__)
#include "VideoMasterHD/VideoMasterHD_Core.h"
#include "VideoMasterHD/VideoMasterHD_PTP.h"
#include "VideoMasterHD/VideoMasterHD_Ip_ST2110_20.h"
#else
#include "VideoMasterHD_Core.h"
#include "VideoMasterHD_PTP.h"
#include "VideoMasterHD_Ip_ST2110_20.h"
#endif

/*!
   @brief Convert VHD_PTP_PORT_STATE to string

   @returns String representation of the state
*/
std::string to_string(VHD_PTP_PORT_STATE state /*!< [in] VHD_PTP_PORT_STATE to convert*/);

/*!
   @brief Convert VHD_ERRORCODE to string
   
   @returns String representation of the error code
*/
std::string to_string(VHD_ERRORCODE error_code /*!< [in] VHD_ERRORCODE to convert*/);

/*!
   @brief Get the MAC address of the NIC

   @returns VHDERR_NOERROR if the MAC address is successfully retrieved
*/
VHD_ERRORCODE get_nic_mac_address(HANDLE board_handle /*!< [in] Board handle*/,
    std::string& mac_address /*!< [out] MAC address of the NIC*/);

/*!
   @brief Get video standard info

   @returns VHDERR_BADARG if the video standard is not supported, VHDERR_NOERROR otherwise
*/
VHD_ERRORCODE get_video_standard_info(
   VHD_ST2110_20_VIDEO_STANDARD video_standard /*!< [in] Video standard to get information for  */,
   uint32_t& frame_width /*!< [out] Width of the video frame in pixels */,
   uint32_t& frame_height /*!< [out] Height of the video frame in pixels */,
   uint32_t& frame_rate /*!< [out] Frame rate of the video in frames per second */,
   bool& interlaced /*!< [out] True if the video is interlaced, false if progressive */,
   bool& is_us /*!< [out] True if the video standard uses US frame rates (e.g. 29.97, 59.94 fps) */
);

/*!
   @brief This function allows to configure a network interface and provides the corresponding ID.

   @detail The call to this function is optional. If it is not called, the OS configuration will not be modified.

   @returns The function returns the status of its execution as VHD_ERRORCODE
*/
VHD_ERRORCODE configure_nic(HANDLE board_handle /*!< [in] Board handle.*/,
   uint32_t ip_address /*!< [in] Ip address that will be configured on the NIC.*/,
   uint32_t subnet_mask /*!< [in] Subnet mask that will be configured on the NIC.*/,
   uint32_t gateway /*!< [in] Gateway that will be configured on the NIC.*/,
   bool     is_dhcp_enabled /*!< [in] If true, the NIC will be configured to use DHCP.*/
);

/*!
   @brief This function configures a TX stream based on the given parameters.

   @returns The function returns the status of its execution as VHD_ERRORCODE
*/
VHD_ERRORCODE configure_stream(HANDLE board_handle /*!< [in] Board handle*/,
                               HANDLE& stream_handle /*!< [out] Handle of the created stream*/,
                               VHD_STREAMTYPE stream_type /*!< [in] Type of the stream to configure*/,
                               VHD_ST2110_20_VIDEO_STANDARD video_standard /*!< [in] Video standard of the stream*/,
                               uint32_t destination_ip /*!< [in] Destination IP of the stream*/,
                               uint32_t destination_ssrc /*!< [in] Destination SSRC of the stream*/,
                               uint16_t destination_udp_port /*!< [in] Destination UDP port of the stream*/
);

/*!
   @brief This function manages stream creation and configuration based on a SDP

   @returns The function returns the status of its execution as VHD_ERRORCODE
*/
VHD_ERRORCODE configure_stream_from_sdp(HANDLE board /*!< [in] Board handle*/,
                                        std::string sdp /*!< [in] SDP from which informations will be extracted*/,
                                        const uint32_t destination_ip_overrides /*!< [in] To override the destination IP contained in the SDP.*/,
                                        const uint16_t destination_udp_port_ovverrides /*!< [in] To override the destination UDP port contained in the SDP.*/,
                                        HANDLE stream /*!< [in] Handle of the created stream */,
                                        uint32_t& multicast_group /*!< [out] Multicast group the has been joined, 0 if none */
);

/*!
   @brief This function generates an SDP based on a stream configuration.

   @returns The function returns the status of its execution as VMIP_ERRORCODE
*/
VHD_ERRORCODE generate_sdp(HANDLE board_handle  /*!< [in] VCS context used by the stream */,
                           HANDLE stream_handle /*!< [in] stream used for the SDP generation */,
                           std::string& sdp     /*!< [out] SDP generated*/
);

/*!
   @brief This function leaves a multicast group

   @returns The function returns the status of its execution as VHD_ERRORCODE
*/
VHD_ERRORCODE leave_multicast(HANDLE board /*!< [in] Board handle*/,
                              uint32_t& multicast_group /*!< [inout] Multicast group to leave*/);

/*! 
   @brief This function applies the PTP parameters.

   @returns The function returns the status of its execution as VHD_ERRORCODE

*/
VHD_ERRORCODE apply_ptp_parameters(HANDLE board_handle /*!< [in] Board handle.*/,
                                   uint8_t domain_number /*!< [in] Domain number of the PTP service*/,
                                   uint8_t announce_receipt_timeout /*!< [in] Announce receipt timeout in seconds*/
);

/*!
   @brief This function prints the status of the PTP service

   @returns The function returns the status of its execution as VHD_ERRORCODE
*/
VHD_ERRORCODE print_ptp_status(HANDLE board_handle /*!< [in] Board handle.*/,
                               uint8_t domain_number /*!< [in] Domain number of the PTP service*/,
                               uint8_t announce_receipt_timeout /*!< [in] Announce receipt timeout in seconds*/

);

/*!
   @brief This function monitor RX stream status
*/
void monitor_rx_stream_status(HANDLE stream_handle, bool* request_stop, uint32_t* timeout);

/*!
   @brief This function monitor TX stream status
*/
void monitor_tx_stream_status(HANDLE stream_handle, bool* request_stop);
