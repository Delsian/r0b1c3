/*
 * ili9341.h
 *
 */

#ifndef ILI9341_H_
#define ILI9341_H_

#define MAX_SPRITE_SIZE 64

// 16- color representation
typedef struct {
        unsigned int p1 : 4 ;
        unsigned int p2 : 4 ;
} __attribute__((packed)) tColor;

typedef struct {
    uint16_t width;
    uint16_t height;
    tColor *img;
    tColor *bg; // mem area to keep background
} __attribute__((packed)) tSprite;

ret_code_t ili9341_init(void);
void ClearDisplay(void);
void SpriteDraw(uint16_t x, uint16_t y, tSprite *s);
void SpriteClear(uint16_t x, uint16_t y, tSprite *s);

void ili9341_rect_draw(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);

void ili9341_set_color_table(uint16_t* buf, uint8_t len);

#endif /* ILI9341_H_ */
