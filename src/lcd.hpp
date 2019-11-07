#pragma once
#ifndef LCD_HPP__
#define LCD_HPP__
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>


#define LCD_ADDR (int)0x27
#define LCD_ROW (int)2
#define LCD_COL (int)16

#ifdef USE_TEMPLATE

template <typename T>
void lcd_err_clr_pr(LiquidCrystal_I2C &lcd, T content)

#else

void lcd_err_clr_pr(LiquidCrystal_I2C &lcd, const char *content);
void lcd_err_clr_pr(LiquidCrystal_I2C &lcd, const String &content);
void lcd_err_clr_pr(LiquidCrystal_I2C &lcd, int err_code);

#endif


#endif