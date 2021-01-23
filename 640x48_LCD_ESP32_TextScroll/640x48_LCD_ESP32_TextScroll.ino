// ####################  LCD Pin Assignments  ####################
//  
//  0.5mm Pitch - 22Pin
//  
//  1   - GND   [to GND]
//  2   - RST   [to ESP32 - 32]
//  3   - GND   [to GND]
//  4   - D0    [to ESP32 - 12]
//  5   - D1    [to ESP32 - 13]
//  6   - D2    [to ESP32 - 26]
//  7   - D3    [to ESP32 - 25]
//  8   - D4    [to ESP32 - 17]
//  9   - D5    [to ESP32 - 16]
//  10  - D6    [to ESP32 - 27]
//  11  - D7    [to ESP32 - 14]
//  12  - GND   [to GND]
//  13  - RD    [to ESP32 - 2]
//  14  - WR    [to ESP32 - 4]
//  15  - CS    [to ESP32 - 33]
//  16  - DC    [to ESP32 - 15]
//  17  - TE    [OPEN]
//  18  - VDD   [to 3.3V]
//  19  - VDDIO [to 3.3V]
//  20  - LED+  [to 5.5V~]
//  21  - LED-  [to GND]
//  22  - GND   [to GND]
//
// ※ PSRAMを使用する場合は GPIO 16 , 17 は使用できないので注意。

#include <LovyanGFX.hpp>


struct LGFX_Config {
  static constexpr int gpio_wr  =  4; //to LCD WR(14)
  static constexpr int gpio_rd  =  2; //to LCD RD(13)
  static constexpr int gpio_rs  = 15; //to LCD DC(16)
  static constexpr int gpio_d0  = 12; //to LCD D0(4)
  static constexpr int gpio_d1  = 13; //to LCD D1(5)
  static constexpr int gpio_d2  = 26; //to LCD D2(6)
  static constexpr int gpio_d3  = 25; //to LCD D3(7)
  static constexpr int gpio_d4  = 17; //to LCD D4(8)
  static constexpr int gpio_d5  = 16; //to LCD D5(9)
  static constexpr int gpio_d6  = 27; //to LCD D6(10)
  static constexpr int gpio_d7  = 14; //to LCD D7(11)
};

constexpr char text[] = "マジョカアイリス！マジマジョマジカル！あなたも変身！魔法×戦士 マジマジョピュアーズ！きらめく魔法でピュアライズ！ホンモノガーネット！ビッグアクアマリン！レインボーミルキーファンタジー！遊びがいっぱい！マジョカアイリス！";
constexpr uint32_t textlen = sizeof(text) / sizeof(text[0]);
constexpr int32_t panel_width = 640;
constexpr int32_t scroll_step = 1;

static lgfx::LGFX_PARALLEL<LGFX_Config> lcd;

static lgfx::Panel_ILI9342 panel;

static lgfx::LGFX_Sprite buf; // [(640 + 40) * 48] Buffer


inline void lcd_buffer_write()
{
  lcd.startWrite();
  lcd.setAddrWindow(0, 0, 320, 96);
  auto b = (lgfx::swap565_t*)buf.getBuffer();
  auto bufwidth = buf.width();
  // バッファ幅が640より1文字分長いので、まとめて出力するとズレるため、1ラインずつループで出力
  for (int i = 0; i < 48; i++)
  {
    lcd.writePixels(&b[bufwidth * i], panel_width);
  }
  lcd.endWrite();
}


void setup(void)
{
// 設定できるクロックは20MHz～6.7MHzです。また、80MHzを整数で割った値に調整されます。
// 20MHz, 16MHz, 13.4MHz, 11.5MHz, 10MHz, 88.9MHz, 8MHz, 7.3MHz, 6.7MHz
  panel.freq_write = 16000000;
  panel.len_dummy_read_pixel = 8;

  panel.spi_cs = 33;  //to LCD CS(15)
  panel.spi_dc = -1;
  panel.gpio_rst = 32;  //to LCD RST(2)

  panel.panel_width  = 320;
  panel.panel_height = 96;

  panel.offset_x = 0;
  panel.offset_y = 144;

  lcd.setPanel(&panel);

  lcd.init();
  lcd.setColorDepth(16);

  lcd.setRotation(0); // 0 or 2

  buf.setColorDepth(16);
  buf.createSprite(panel_width + 40, 48);  // バッファ幅はパネル幅より1文字分広くしておく
  buf.setFont(&fonts::lgfxJapanMinchoP_40);
  buf.setTextWrap(false);
}


void loop(void)
{
  static size_t textpos = 0;
  int32_t cursor_x = buf.getCursorX() - scroll_step;   // 現在のカーソル位置を取得し、スクロール量の分だけ左に移動
  if (cursor_x <= 0) // カーソル位置が左端に到達している場合は一周したと判断
  {
    textpos = 0;            // 文字列の読取り位置をリセット
    cursor_x = panel_width; // 新たな文字が画面右端に描画されるようにカーソル位置を変更
  }

  buf.setCursor(cursor_x, 0);  // カーソル位置を更新
  buf.scroll(-scroll_step, 0); // バッファ内容を左にスクロール
  while (textpos < textlen && cursor_x <= panel_width) // 画面右端に文字が書けるか判定
  {
    buf.print(text[textpos++]);   // 1バイトずつ出力 (マルチバイト文字でもこの処理で動作します)
    cursor_x = buf.getCursorX();  // 出力後のカーソル位置を取得
  }

  lcd_buffer_write();
}

