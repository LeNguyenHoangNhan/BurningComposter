#include "lcd.hpp"

#ifdef USE_TEMPLATE
template <typename T>
void lcd_err_clr_pr(LiquidCrystal_I2C &lcd, T content) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Error:");
  lcd.setCursor(0, 1);
  lcd.print(content);
}
template <typename T>
void lcd_clr_pr(LiquidCrystal_I2C &lcd, T content) {}
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