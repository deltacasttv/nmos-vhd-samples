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

#include "pattern.h"

#include <iostream>

#include <vector>

/*!
   @brief Structure representing yuv color object.
*/

struct Yuv8Bit
{
   uint8_t y;
   uint8_t u;
   uint8_t v;
};

void create_color_bar_pattern(uint8_t* user_buffer, uint32_t frame_height, uint32_t frame_width)
{
   const Yuv8Bit white = {0xb4, 0x80, 0x80};
   const Yuv8Bit yellow = {0xa8, 0x2c, 0x88};
   const Yuv8Bit cyan = {0x91, 0x93, 0x2c};
   const Yuv8Bit green = {0x85, 0x3f, 0x34};
   const Yuv8Bit magenta = {0x3f, 0xc1, 0xcc};
   const Yuv8Bit red = {0x33, 0x61, 0xd4};
   const Yuv8Bit blue = {0x1c, 0xd4, 0x78};
   const Yuv8Bit black = {0x10, 0x80, 0x80};
   std::vector<Yuv8Bit> color_list = {white, yellow, cyan, green, magenta, red, blue, black};

   if (user_buffer == nullptr)
   {
      std::cout << std::endl << "The  user_buffer pointer is invalid" << std::endl;
      return;
   }

   if (frame_width % 2 != 0)
   {
      std::cout << std::endl << "The frame width must be a multiple of 2" << std::endl;
      return;
   }

   uint32_t* yuyv_ptr = reinterpret_cast<uint32_t*>(user_buffer);
   for (uint32_t pixel_y = 0; pixel_y < frame_height; pixel_y++)
   {
      for (uint32_t pixel_x = 0; pixel_x < frame_width; pixel_x += 2)
      {
         const auto color = color_list[pixel_x / (frame_width / static_cast<uint32_t>(color_list.size()))];
         *yuyv_ptr++ = color.y << 24 | color.u << 16 | color.y << 8 | color.v;
      }
   }
}

void draw_white_line(uint8_t* buffer, uint32_t line, uint32_t frame_height, uint32_t frame_width,
                     bool interlaced)
{
   const Yuv8Bit white = {0xeb, 0x80, 0x80};
   const uint32_t white_yuyv = white.y << 24 | white.u << 16 | white.y << 8 | white.v;

   if (buffer == nullptr)
   {
      std::cout << std::endl << "The  buffer pointer is invalid" << std::endl;
      return;
   }

   if (frame_width % 2 != 0)
   {
      std::cout << std::endl << "The frame width must be a multiple of 2" << std::endl;
      return;
   }

   uint8_t* temp_ptr;
   if (interlaced)
   {
      if (line % 2 == 0)
         temp_ptr = (buffer + (line / 2) * frame_width * PIXELSIZE_8BIT);
      else
         temp_ptr = (buffer + (((frame_height + 1) / 2) + (line / 2)) * frame_width * PIXELSIZE_8BIT);
   }
   else
      temp_ptr = (buffer + line * frame_width * PIXELSIZE_8BIT);

   uint32_t* yuyv_ptr = reinterpret_cast<uint32_t*>(temp_ptr);
   for (uint32_t pixel_x = 0; pixel_x < frame_width; pixel_x += 2)
      *yuyv_ptr++ = white_yuyv;
}
