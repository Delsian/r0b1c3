/*
 * ili9341.h
 *
 */

#ifndef ILI9341_H_
#define ILI9341_H_

typedef struct {
    uint8_t width;
    uint8_t height;
    uint16_t *img;
} tSprite;

ret_code_t ili9341_init(void);
void ili9341_rect_draw(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);
void ili9341_sprite_draw(uint16_t x, uint16_t y, tSprite *s);


#endif /* ILI9341_H_ */
