#pragma bank 255

#include <gbdk/platform.h>

#include "interrupts.h"
#include "vm.h"
#include "scroll.h"
#include "parallax.h"
#include "ui.h"
#include "game_time.h"
#include "gbs_types.h"
#include "data_manager.h"

#define LYC_SYNC_VALUE 150u

UBYTE hide_sprites = FALSE;
UBYTE show_actors_on_overlay = FALSE;
UBYTE overlay_cut_scanline = LYC_SYNC_VALUE;
LCD_isr_e tmp_LCD_type;


UBYTE waves[] = { 0, 1, 2, 1};

void remove_LCD_ISRs(void) BANKED {
    CRITICAL {
		remove_LCD(wobble_LCD_isr);
        remove_LCD(parallax_LCD_isr);
        remove_LCD(simple_LCD_isr);
        remove_LCD(fullscreen_LCD_isr);
        LCDC_REG &= ~LCDCF_BG8000;
    }
}

void simple_LCD_isr(void) NONBANKED {
    if (LYC_REG == LYC_SYNC_VALUE) {
        SCX_REG = draw_scroll_x;
        SCY_REG = draw_scroll_y;
        if (WY_REG) {
            if (WY_REG < MENU_CLOSED_Y) LYC_REG = WY_REG - 1;
        } else {
            if ((WX_REG == DEVICE_WINDOW_PX_OFFSET_X) && (show_actors_on_overlay == FALSE)) HIDE_SPRITES;
            LYC_REG = overlay_cut_scanline;
        }
    } else {
        if (LYC_REG < overlay_cut_scanline) {
            if ((WX_REG == DEVICE_WINDOW_PX_OFFSET_X) && (show_actors_on_overlay == FALSE)) {
                while (STAT_REG & STATF_BUSY) ;
                HIDE_SPRITES;
            }
            LYC_REG = overlay_cut_scanline;
        } else {
            while (STAT_REG & STATF_BUSY) ;
            WX_REG = 0, HIDE_WIN;
            if (!hide_sprites) SHOW_SPRITES;
            LYC_REG = LYC_SYNC_VALUE;
            return;
        }
    }
}

void wobble_LCD_isr(void) NONBANKED {	
	while (STAT_REG & STATF_BUSY) ;
	if (LYC_REG == 0){
		LYC_REG = -((game_time) &7);
	}	
	UBYTE a = ((LY_REG + game_time)>>3);	
	SCX_REG = draw_scroll_x +  (waves[a&3]);
	SCY_REG = draw_scroll_y +  (waves[(a + 1)&3]);
	LYC_REG+=8;
	if (LYC_REG < 144){	
		return;
	} 
	LYC_REG = 0;
}

void fullscreen_LCD_isr(void) NONBANKED {
    if (LYC_REG == LYC_SYNC_VALUE) {
        LCDC_REG &= ~LCDCF_BG8000;
        SCX_REG = draw_scroll_x;
        SCY_REG = draw_scroll_y;
        LYC_REG = (9 * 8) - 1;
    } else {
        while (STAT_REG & STATF_BUSY) ;
        LCDC_REG |= LCDCF_BG8000;
        LYC_REG = LYC_SYNC_VALUE;
    }
}

void VBL_isr(void) NONBANKED {
    if ((WY_REG = win_pos_y) < MENU_CLOSED_Y) WX_REG = (win_pos_x + DEVICE_WINDOW_PX_OFFSET_X), SHOW_WIN; else WX_REG = 0, HIDE_WIN;
    if (hide_sprites) HIDE_SPRITES; else SHOW_SPRITES;
    scroll_shadow_update();
}

void set_temp_lcd_effect(SCRIPT_CTX * THIS) OLDCALL BANKED {	
	UBYTE lcd_enum = *(uint8_t *) VM_REF_TO_PTR(FN_ARG0);
	if (lcd_enum){
		if (tmp_LCD_type != lcd_enum){
			tmp_LCD_type = lcd_enum;
			remove_LCD_ISRs();
			CRITICAL {
				switch (tmp_LCD_type) {
					case LCD_wobble:		
						add_LCD(wobble_LCD_isr);
						break;
					case LCD_parallax: 
						add_LCD(parallax_LCD_isr);
						break;
					case LCD_fullscreen:
						add_LCD(fullscreen_LCD_isr);
						break;
					default:
						add_LCD(simple_LCD_isr);
						break;
				}
				LYC_REG = 0u;
            }			
		}
	} else {
		if (tmp_LCD_type){
			tmp_LCD_type = scene_LCD_type;
			remove_LCD_ISRs();
			CRITICAL {
				switch (scene_LCD_type) {
					case LCD_wobble:		
						add_LCD(wobble_LCD_isr);
						break;
					case LCD_parallax: 
						add_LCD(parallax_LCD_isr);
						break;
					case LCD_fullscreen:
						add_LCD(fullscreen_LCD_isr);
						break;
					default:
						add_LCD(simple_LCD_isr);
						break;
				}
				LYC_REG = 0u;
            }			
		}		
	}	
}
