/*
    This file is part of Burning Composter - A compost monitoring device
    based on ESP32 and Arduino Core.

        Copyright (c) 2019 Le Nguyen Hoang Nhan

    Burning Composter  is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Burning Composter  is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Burning Composter.  If not, see <https://www.gnu.org/licenses/>.
*/
#pragma once
#ifndef LCD_HPP__
#define LCD_HPP__
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

#define LCD_ADDR (int)0x27
#define LCD_ROW (int)2
#define LCD_COL (int)16

#ifdef USE_TEMPLATE

template <typename T> void lcd_err_clr_pr(LiquidCrystal_I2C &lcd, T content)

#else

void lcd_err_clr_pr(LiquidCrystal_I2C &lcd, const char *content);
void lcd_err_clr_pr(LiquidCrystal_I2C &lcd, const String &content);
void lcd_err_clr_pr(LiquidCrystal_I2C &lcd, int err_code);

#endif

#endif