
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
#include "lcd.hpp"

#ifdef USE_TEMPLATE
template <typename T> void lcd_err_clr_pr(LiquidCrystal_I2C &lcd, T content) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Error:");
  lcd.setCursor(0, 1);
  lcd.print(content);
}
template <typename T> void lcd_clr_pr(LiquidCrystal_I2C &lcd, T content) {}
#endif

#ifndef USE_TEMPLATE
void lcd_err_clr_pr(LiquidCrystal_I2C &lcd, const char *content) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Error");
  lcd.setCursor(0, 1);
  lcd.print(content);
}
void lcd_err_clr_pr(LiquidCrystal_I2C &lcd, const String &content) {
  lcd_err_clr_pr(lcd, content.c_str());
}
void lcd_err_clr_pr(LiquidCrystal_I2C &lcd, int err_code) {
  char buff[4];
  snprintf(buff, 3, "%d", err_code);
  lcd_err_clr_pr(lcd, buff);
}
#endif /* USE_TEMPLATE */
