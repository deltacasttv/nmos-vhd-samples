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
#include "nmos/node_api.h"
#include "nmos/node_server.h"
#include "nmos/server.h"
#include "nmos/mutex.h"

namespace nmos_tools
{
   struct NmosPtpSystemParameters{
      int announce_receipt_timeout /*! Announce receipt timeout in seconds */ = 0;
      int domain_number            /*! Domain number */ = 0;

      bool operator==(const NmosPtpSystemParameters& other) const {
         return std::tie(announce_receipt_timeout, domain_number) ==
                std::tie(other.announce_receipt_timeout, other.domain_number);
      }

      bool operator!=(const NmosPtpSystemParameters& other) const {
         return !(*this == other);
      }
   }; /*! PTP parameters as defined in PTP standard IEEE 1588-2008 */;

   class NodeServer
   {
   public:

      bool is_enabled;

      NodeServer(nmos::node_model& node_model, nmos::experimental::node_implementation node_implementation, nmos::experimental::log_model& log_model,
         slog::base_gate& gate, const std::string device_name, const std::string device_description, std::string media_nic_name,
         std::string media_nic_mac_address);

      bool get_ptp_system_parameters(NmosPtpSystemParameters& ptp_system_parameters);

      void start();
      void stop();

   protected:

      //parameters
      nmos::node_model& node_model;
      slog::base_gate& gate;
      std::string media_nic_name;
      std::string media_nic_mac_address;
      std::string sdp;
      const std::string device_name;
      const std::string device_description;

      //members
      nmos::server node_server;
      nmos::id node_id;
      nmos::id device_id;

      virtual bool node_implementation_init();

      //callbacks
      void system_params_handler(const web::uri& system_uri, const web::json::value& system_global);
      virtual void resolve_auto(const nmos::resource& resource, const nmos::resource& connection_resource, web::json::value& transport_params) = 0;
      virtual void patch_validator(const nmos::resource& resource, const nmos::resource& connection_resource, const web::json::value& endpoint_staged) = 0;
      virtual void connection_activation(const nmos::resource& resource, const nmos::resource& connection_resource) = 0;

      //helpers
      bool insert_resource_after(nmos::node_model &node_model, nmos::write_lock &lock, unsigned int milliseconds, nmos::resources& resources, nmos::resource&& resource, slog::base_gate& gate);
      bool is_field_auto(const web::json::value& object, const web::json::field_as_value_or& field_name);

      class node_implementation_init_exception : public std::exception
      {
      public:
         node_implementation_init_exception(const std::string& message) : m_message(message) {}

         const char* what() const noexcept override
         {
            return m_message.c_str();
         }

      private:
         std::string m_message;
      };
   private:
      //system parameters
      NmosPtpSystemParameters m_ptp_system_parameters;
      std::mutex m_ptp_system_parameters_mutex;
      bool m_are_system_parameters_valid = false;
   };

   class NodeServerReceiver : public nmos_tools::NodeServer
   {
   public:

      struct TransportParams{
         uint32_t ip_interface /*! Receiver interface ip. */ = 0u;
         uint32_t ip_multicast /*! Multicast destination ip : can be equal to 0 if unicast used. */ = 0u;
         uint32_t ip_src /*! Source ip : filtering, if equals to 0 disable filtering. */ = 0u;
         uint16_t port_dst /*! Destination port. */ = 0u;

         bool operator==(const TransportParams& other) const {
            return std::tie(ip_interface, ip_multicast, ip_src, port_dst) ==
                   std::tie(other.ip_interface, other.ip_multicast, other.ip_src, other.port_dst);
         }

         bool operator!=(const TransportParams& other) const {
            return !(*this == other);
         }
      };

      TransportParams& m_active_transport_params;

      NodeServerReceiver(nmos::node_model& node_model, nmos::experimental::log_model& log_model, slog::base_gate& gate, const std::string device_name,
         const   std::string device_description, TransportParams& resolve_auto_transport_params, TransportParams& active_transport_params,
         std::string media_nic_name, std::string media_nic_mac_address);

      bool node_implementation_init() override;

      std::string get_sdp();

   private:

      TransportParams& m_resolve_auto_transport_params;

      nmos::experimental::node_implementation make_node_implementation();

      void resolve_auto(const nmos::resource& resource, const nmos::resource& connection_resource, web::json::value& transport_params) override;
      void patch_validator(const nmos::resource& resource, const nmos::resource& connection_resource, const web::json::value& endpoint_staged) override;
      void connection_activation(const nmos::resource& resource, const nmos::resource& connection_resource) override;
      web::json::value transportfile_parser(const nmos::resource& resource, const nmos::resource& connection_resource, const utility::string_t& transportfile_type, const utility::string_t& transportfile_data);
   
      web::json::value generate_constraints();
   };

   // convert the given ipv4 address in network byte order to a string representation of the form "a.b.c.d"
   utility::string_t ipv4_to_string(uint32_t ipv4_network_byte_order);

   // convert the given string representation of an ipv4 address of the form "a.b.c.d" to an ipv4 address in network byte order
   uint32_t string_to_ipv4(const utility::string_t& ipv4_string);
}

