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
    uint16_t width; // !!! width in two-pixel size !!!
    uint16_t height;
    tColor *img;
    tColor *bg; // mem area to keep background
} __attribute__((packed)) tSprite;

ret_code_t ili9341_init(void);
void ClearDisplayWithBg(void);
void SpriteDraw(uint16_t xdiv2, uint16_t y, tSprite *s);
void SpriteClear(uint16_t xdiv2, uint16_t y, tSprite *s);
void SetColorTable(const uint16_t* buf);

#endif /* ILI9341_H_ */
