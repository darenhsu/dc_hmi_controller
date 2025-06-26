#include "dc_hmi_controller.h"

// ============================================================================
// 畫面控制
// ============================================================================

int hmi_switch_screen(hmi_controller_t *hmi, uint16_t screen_id) {
    hmi->current_screen = screen_id;
    uint8_t frame[16];
    uint16_t frame_len = 0;
    
    frame[frame_len++] = FRAME_HEADER;
    frame[frame_len++] = CMD_CONFIG_BASE;
    frame[frame_len++] = CMD_SWITCH_SCREEN;
    frame[frame_len++] = screen_id >> 8;
    frame[frame_len++] = screen_id & 0xFF;
    
    uint8_t tail[] = FRAME_TAIL;
    memcpy(frame + frame_len, tail, FRAME_TAIL_SIZE);
    frame_len += FRAME_TAIL_SIZE;
    
    return hmi_send_command(hmi, frame, frame_len);
}

int hmi_switch_screen_with_effect(hmi_controller_t *hmi, uint16_t screen_id, uint8_t effect, 
                                  uint8_t area_en, uint16_t left, uint16_t right, uint16_t top, uint16_t bottom) {
    hmi->current_screen = screen_id;
    uint8_t data[12] = {
        screen_id >> 8, screen_id & 0xFF,
        effect, area_en,
        left >> 8, left & 0xFF,
        right >> 8, right & 0xFF,
        top >> 8, top & 0xFF,
        bottom >> 8, bottom & 0xFF
    };
    uint8_t frame[32];
    uint16_t frame_len = 0;
    
    frame[frame_len++] = FRAME_HEADER;
    frame[frame_len++] = CMD_CONFIG_BASE;
    frame[frame_len++] = CMD_ANIM_SWITCH;
    
    memcpy(frame + frame_len, data, sizeof(data));
    frame_len += sizeof(data);
    
    uint8_t tail[] = FRAME_TAIL;
    memcpy(frame + frame_len, tail, FRAME_TAIL_SIZE);
    frame_len += FRAME_TAIL_SIZE;
    
    return hmi_send_command(hmi, frame, frame_len);
}

int hmi_read_screen(hmi_controller_t *hmi, uint16_t *screen_id) {
    uint8_t frame[16];
    uint16_t frame_len = 0;
    
    frame[frame_len++] = FRAME_HEADER;
    frame[frame_len++] = CMD_CONFIG_BASE;
    frame[frame_len++] = CMD_READ_SCREEN;
    
    uint8_t tail[] = FRAME_TAIL;
    memcpy(frame + frame_len, tail, FRAME_TAIL_SIZE);
    frame_len += FRAME_TAIL_SIZE;
    
    if (hmi_send_command(hmi, frame, frame_len) < 0) {
        return -1;
    }
    
    hmi_response_t response;
    if (hmi_receive_response(hmi, &response, 1000) == 0) {
        if (response.data && response.length >= 3) {
            *screen_id = (response.data[1] << 8) | response.data[2];
            free(response.data);
            return 0;
        }
        if (response.data) free(response.data);
    }
    
    return -1;
}

// ============================================================================
// 觸摸屏配置
// ============================================================================

int hmi_config_touch(hmi_controller_t *hmi, touch_config_t *config) {
    uint8_t cmd = 0;
    cmd |= (config->enable & 0x01);
    cmd |= (config->beep & 0x01) << 1;
    cmd |= (config->upload_mode & 0x07) << 2;
    cmd |= (config->calibrate_disable & 0x01) << 5;
    
    return hmi_send_data(hmi, CMD_TOUCH_CONFIG, &cmd, 1);
}

int hmi_calibrate_touch(hmi_controller_t *hmi) {
    return hmi_send_data(hmi, CMD_TOUCH_CALIBRATE, NULL, 0);
}

int hmi_test_touch(hmi_controller_t *hmi, uint8_t enable) {
    return hmi_send_data(hmi, CMD_TOUCH_TEST, &enable, 1);
}

// ============================================================================
// 文本控制
// ============================================================================

int hmi_update_text(hmi_controller_t *hmi, uint16_t screen_id, uint16_t control_id, const char *text) {
    uint16_t text_len = strlen(text);
    uint8_t frame[1024];
    uint16_t frame_len = 0;
    
    frame[frame_len++] = FRAME_HEADER;
    frame[frame_len++] = CMD_CONFIG_BASE;
    frame[frame_len++] = CMD_UPDATE_CONTROL;
    frame[frame_len++] = screen_id >> 8;
    frame[frame_len++] = screen_id & 0xFF;
    frame[frame_len++] = control_id >> 8;
    frame[frame_len++] = control_id & 0xFF;
    
    memcpy(frame + frame_len, text, text_len);
    frame_len += text_len;
    
    uint8_t tail[] = FRAME_TAIL;
    memcpy(frame + frame_len, tail, FRAME_TAIL_SIZE);
    frame_len += FRAME_TAIL_SIZE;
    
    return hmi_send_command(hmi, frame, frame_len);
}

int hmi_clear_text(hmi_controller_t *hmi, uint16_t screen_id, uint16_t control_id) {
    uint8_t frame[32];
    uint16_t frame_len = 0;
    
    frame[frame_len++] = FRAME_HEADER;
    frame[frame_len++] = CMD_CONFIG_BASE;
    frame[frame_len++] = CMD_UPDATE_CONTROL;
    frame[frame_len++] = screen_id >> 8;
    frame[frame_len++] = screen_id & 0xFF;
    frame[frame_len++] = control_id >> 8;
    frame[frame_len++] = control_id & 0xFF;
    
    uint8_t tail[] = FRAME_TAIL;
    memcpy(frame + frame_len, tail, FRAME_TAIL_SIZE);
    frame_len += FRAME_TAIL_SIZE;
    
    return hmi_send_command(hmi, frame, frame_len);
}

int hmi_read_text(hmi_controller_t *hmi, uint16_t screen_id, uint16_t control_id, char *text, uint16_t max_len) {
    uint8_t frame[32];
    uint16_t frame_len = 0;
    
    frame[frame_len++] = FRAME_HEADER;
    frame[frame_len++] = CMD_CONFIG_BASE;
    frame[frame_len++] = CMD_READ_CONTROL;
    frame[frame_len++] = screen_id >> 8;
    frame[frame_len++] = screen_id & 0xFF;
    frame[frame_len++] = control_id >> 8;
    frame[frame_len++] = control_id & 0xFF;
    
    uint8_t tail[] = FRAME_TAIL;
    memcpy(frame + frame_len, tail, FRAME_TAIL_SIZE);
    frame_len += FRAME_TAIL_SIZE;
    
    if (hmi_send_command(hmi, frame, frame_len) < 0) {
        return -1;
    }
    
    hmi_response_t response;
    if (hmi_receive_response(hmi, &response, 1000) == 0) {
        if (response.data && response.length > 5) {
            uint16_t copy_len = (response.length - 5 < max_len - 1) ? response.length - 5 : max_len - 1;
            memcpy(text, response.data + 5, copy_len);
            text[copy_len] = '\0';
            free(response.data);
            return 0;
        }
        if (response.data) free(response.data);
    }
    
    return -1;
}

int hmi_set_text_blink(hmi_controller_t *hmi, uint16_t screen_id, uint16_t control_id, uint16_t cycle_10ms) {
    uint8_t frame[32];
    uint16_t frame_len = 0;
    
    frame[frame_len++] = FRAME_HEADER;
    frame[frame_len++] = CMD_CONFIG_BASE;
    frame[frame_len++] = CMD_SET_BLINK;
    frame[frame_len++] = screen_id >> 8;
    frame[frame_len++] = screen_id & 0xFF;
    frame[frame_len++] = control_id >> 8;
    frame[frame_len++] = control_id & 0xFF;
    frame[frame_len++] = cycle_10ms >> 8;
    frame[frame_len++] = cycle_10ms & 0xFF;
    
    uint8_t tail[] = FRAME_TAIL;
    memcpy(frame + frame_len, tail, FRAME_TAIL_SIZE);
    frame_len += FRAME_TAIL_SIZE;
    
    return hmi_send_command(hmi, frame, frame_len);
}

int hmi_set_text_scroll(hmi_controller_t *hmi, uint16_t screen_id, uint16_t control_id, uint16_t speed) {
    uint8_t frame[32];
    uint16_t frame_len = 0;
    
    frame[frame_len++] = FRAME_HEADER;
    frame[frame_len++] = CMD_CONFIG_BASE;
    frame[frame_len++] = CMD_SET_SCROLL;
    frame[frame_len++] = screen_id >> 8;
    frame[frame_len++] = screen_id & 0xFF;
    frame[frame_len++] = control_id >> 8;
    frame[frame_len++] = control_id & 0xFF;
    frame[frame_len++] = speed >> 8;
    frame[frame_len++] = speed & 0xFF;
    
    uint8_t tail[] = FRAME_TAIL;
    memcpy(frame + frame_len, tail, FRAME_TAIL_SIZE);
    frame_len += FRAME_TAIL_SIZE;
    
    return hmi_send_command(hmi, frame, frame_len);
}

int hmi_set_text_color(hmi_controller_t *hmi, uint16_t screen_id, uint16_t control_id, uint16_t fg_color, uint16_t bg_color) {
    // 設置背景色
    uint8_t frame[32];
    uint16_t frame_len = 0;
    
    frame[frame_len++] = FRAME_HEADER;
    frame[frame_len++] = CMD_CONFIG_BASE;
    frame[frame_len++] = CMD_SET_BK_COLOR;
    frame[frame_len++] = screen_id >> 8;
    frame[frame_len++] = screen_id & 0xFF;
    frame[frame_len++] = control_id >> 8;
    frame[frame_len++] = control_id & 0xFF;
    frame[frame_len++] = bg_color >> 8;
    frame[frame_len++] = bg_color & 0xFF;
    
    uint8_t tail[] = FRAME_TAIL;
    memcpy(frame + frame_len, tail, FRAME_TAIL_SIZE);
    frame_len += FRAME_TAIL_SIZE;
    
    hmi_send_command(hmi, frame, frame_len);
    
    // 設置前景色
    frame_len = 0;
    frame[frame_len++] = FRAME_HEADER;
    frame[frame_len++] = CMD_CONFIG_BASE;
    frame[frame_len++] = CMD_SET_FG_COLOR;
    frame[frame_len++] = screen_id >> 8;
    frame[frame_len++] = screen_id & 0xFF;
    frame[frame_len++] = control_id >> 8;
    frame[frame_len++] = control_id & 0xFF;
    frame[frame_len++] = fg_color >> 8;
    frame[frame_len++] = fg_color & 0xFF;
    
    memcpy(frame + frame_len, tail, FRAME_TAIL_SIZE);
    frame_len += FRAME_TAIL_SIZE;
    
    return hmi_send_command(hmi, frame, frame_len);
}

int hmi_format_text(hmi_controller_t *hmi, uint16_t screen_id, uint16_t control_id, 
                    data_type_t type, uint8_t decimal, uint32_t value) {
    uint8_t frame[32];
    uint16_t frame_len = 0;
    
    frame[frame_len++] = FRAME_HEADER;
    frame[frame_len++] = CMD_CONFIG_BASE;
    frame[frame_len++] = CMD_FORMAT_TEXT;
    frame[frame_len++] = screen_id >> 8;
    frame[frame_len++] = screen_id & 0xFF;
    frame[frame_len++] = control_id >> 8;
    frame[frame_len++] = control_id & 0xFF;
    frame[frame_len++] = (uint8_t)type;
    frame[frame_len++] = decimal;
    frame[frame_len++] = (value >> 24) & 0xFF;
    frame[frame_len++] = (value >> 16) & 0xFF;
    frame[frame_len++] = (value >> 8) & 0xFF;
    frame[frame_len++] = value & 0xFF;
    
    uint8_t tail[] = FRAME_TAIL;
    memcpy(frame + frame_len, tail, FRAME_TAIL_SIZE);
    frame_len += FRAME_TAIL_SIZE;
    
    return hmi_send_command(hmi, frame, frame_len);
}

// ============================================================================
// 按鈕控制
// ============================================================================

int hmi_set_button_state(hmi_controller_t *hmi, uint16_t screen_id, uint16_t control_id, uint8_t state) {
    uint8_t frame[32];
    uint16_t frame_len = 0;
    
    frame[frame_len++] = FRAME_HEADER;
    frame[frame_len++] = CMD_CONFIG_BASE;
    frame[frame_len++] = CMD_UPDATE_CONTROL;
    frame[frame_len++] = screen_id >> 8;
    frame[frame_len++] = screen_id & 0xFF;
    frame[frame_len++] = control_id >> 8;
    frame[frame_len++] = control_id & 0xFF;
    frame[frame_len++] = state;
    
    uint8_t tail[] = FRAME_TAIL;
    memcpy(frame + frame_len, tail, FRAME_TAIL_SIZE);
    frame_len += FRAME_TAIL_SIZE;
    
    return hmi_send_command(hmi, frame, frame_len);
}

int hmi_read_button_state(hmi_controller_t *hmi, uint16_t screen_id, uint16_t control_id, uint8_t *state) {
    uint8_t frame[32];
    uint16_t frame_len = 0;
    
    frame[frame_len++] = FRAME_HEADER;
    frame[frame_len++] = CMD_CONFIG_BASE;
    frame[frame_len++] = CMD_READ_CONTROL;
    frame[frame_len++] = screen_id >> 8;
    frame[frame_len++] = screen_id & 0xFF;
    frame[frame_len++] = control_id >> 8;
    frame[frame_len++] = control_id & 0xFF;
    
    uint8_t tail[] = FRAME_TAIL;
    memcpy(frame + frame_len, tail, FRAME_TAIL_SIZE);
    frame_len += FRAME_TAIL_SIZE;
    
    if (hmi_send_command(hmi, frame, frame_len) < 0) {
        return -1;
    }
    
    hmi_response_t response;
    if (hmi_receive_response(hmi, &response, 1000) == 0) {
        if (response.data && response.length >= 7) {
            *state = response.data[6];
            free(response.data);
            return 0;
        }
        if (response.data) free(response.data);
    }
    
    return -1;
}

// ============================================================================
// 進度條和滑動條控制
// ============================================================================

int hmi_update_progress(hmi_controller_t *hmi, uint16_t screen_id, uint16_t control_id, uint32_t value) {
    uint8_t frame[32];
    uint16_t frame_len = 0;
    
    frame[frame_len++] = FRAME_HEADER;
    frame[frame_len++] = CMD_CONFIG_BASE;
    frame[frame_len++] = CMD_UPDATE_CONTROL;
    frame[frame_len++] = screen_id >> 8;
    frame[frame_len++] = screen_id & 0xFF;
    frame[frame_len++] = control_id >> 8;
    frame[frame_len++] = control_id & 0xFF;
    frame[frame_len++] = (value >> 24) & 0xFF;
    frame[frame_len++] = (value >> 16) & 0xFF;
    frame[frame_len++] = (value >> 8) & 0xFF;
    frame[frame_len++] = value & 0xFF;
    
    uint8_t tail[] = FRAME_TAIL;
    memcpy(frame + frame_len, tail, FRAME_TAIL_SIZE);
    frame_len += FRAME_TAIL_SIZE;
    
    return hmi_send_command(hmi, frame, frame_len);
}

int hmi_read_progress(hmi_controller_t *hmi, uint16_t screen_id, uint16_t control_id, uint32_t *value) {
    uint8_t frame[32];
    uint16_t frame_len = 0;
    
    frame[frame_len++] = FRAME_HEADER;
    frame[frame_len++] = CMD_CONFIG_BASE;
    frame[frame_len++] = CMD_READ_CONTROL;
    frame[frame_len++] = screen_id >> 8;
    frame[frame_len++] = screen_id & 0xFF;
    frame[frame_len++] = control_id >> 8;
    frame[frame_len++] = control_id & 0xFF;
    
    uint8_t tail[] = FRAME_TAIL;
    memcpy(frame + frame_len, tail, FRAME_TAIL_SIZE);
    frame_len += FRAME_TAIL_SIZE;
    
    if (hmi_send_command(hmi, frame, frame_len) < 0) {
        return -1;
    }
    
    hmi_response_t response;
    if (hmi_receive_response(hmi, &response, 1000) == 0) {
        if (response.data && response.length >= 9) {
            *value = (response.data[5] << 24) | (response.data[6] << 16) | 
                     (response.data[7] << 8) | response.data[8];
            free(response.data);
            return 0;
        }
        if (response.data) free(response.data);
    }
    
    return -1;
}

int hmi_update_slider(hmi_controller_t *hmi, uint16_t screen_id, uint16_t control_id, uint32_t value) {
    return hmi_update_progress(hmi, screen_id, control_id, value);
}

int hmi_read_slider(hmi_controller_t *hmi, uint16_t screen_id, uint16_t control_id, uint32_t *value) {
    return hmi_read_progress(hmi, screen_id, control_id, value);
}

// ============================================================================
// 儀表控制
// ============================================================================

int hmi_update_meter(hmi_controller_t *hmi, uint16_t screen_id, uint16_t control_id, uint32_t value) {
    return hmi_update_progress(hmi, screen_id, control_id, value);
}

int hmi_read_meter(hmi_controller_t *hmi, uint16_t screen_id, uint16_t control_id, uint32_t *value) {
    return hmi_read_progress(hmi, screen_id, control_id, value);
}

// ============================================================================
// 圖標控制
// ============================================================================

int hmi_show_icon(hmi_controller_t *hmi, uint16_t screen_id, uint16_t control_id, uint8_t frame_id) {
    uint8_t frame[32];
    uint16_t frame_len = 0;
    
    frame[frame_len++] = FRAME_HEADER;
    frame[frame_len++] = CMD_CONFIG_BASE;
    frame[frame_len++] = CMD_ANIM_FRAME;
    frame[frame_len++] = screen_id >> 8;
    frame[frame_len++] = screen_id & 0xFF;
    frame[frame_len++] = control_id >> 8;
    frame[frame_len++] = control_id & 0xFF;
    frame[frame_len++] = frame_id;
    
    uint8_t tail[] = FRAME_TAIL;
    memcpy(frame + frame_len, tail, FRAME_TAIL_SIZE);
    frame_len += FRAME_TAIL_SIZE;
    
    return hmi_send_command(hmi, frame, frame_len);
}

int hmi_set_icon_position(hmi_controller_t *hmi, uint16_t screen_id, uint16_t control_id, uint16_t x, uint16_t y) {
    uint8_t frame[32];
    uint16_t frame_len = 0;
    
    frame[frame_len++] = FRAME_HEADER;
    frame[frame_len++] = CMD_CONFIG_BASE;
    frame[frame_len++] = CMD_SET_ICON_POS;
    frame[frame_len++] = screen_id >> 8;
    frame[frame_len++] = screen_id & 0xFF;
    frame[frame_len++] = control_id >> 8;
    frame[frame_len++] = control_id & 0xFF;
    frame[frame_len++] = x >> 8;
    frame[frame_len++] = x & 0xFF;
    frame[frame_len++] = y >> 8;
    frame[frame_len++] = y & 0xFF;
    
    uint8_t tail[] = FRAME_TAIL;
    memcpy(frame + frame_len, tail, FRAME_TAIL_SIZE);
    frame_len += FRAME_TAIL_SIZE;
    
    return hmi_send_command(hmi, frame, frame_len);
}

int hmi_read_icon(hmi_controller_t *hmi, uint16_t screen_id, uint16_t control_id, uint8_t *frame_id) {
    uint8_t frame[32];
    uint16_t frame_len = 0;
    
    frame[frame_len++] = FRAME_HEADER;
    frame[frame_len++] = CMD_CONFIG_BASE;
    frame[frame_len++] = CMD_READ_CONTROL;
    frame[frame_len++] = screen_id >> 8;
    frame[frame_len++] = screen_id & 0xFF;
    frame[frame_len++] = control_id >> 8;
    frame[frame_len++] = control_id & 0xFF;
    
    uint8_t tail[] = FRAME_TAIL;
    memcpy(frame + frame_len, tail, FRAME_TAIL_SIZE);
    frame_len += FRAME_TAIL_SIZE;
    
    if (hmi_send_command(hmi, frame, frame_len) < 0) {
        return -1;
    }
    
    hmi_response_t response;
    if (hmi_receive_response(hmi, &response, 1000) == 0) {
        if (response.data && response.length >= 6) {
            *frame_id = response.data[5];
            free(response.data);
            return 0;
        }
        if (response.data) free(response.data);
    }
    
    return -1;
}

// ============================================================================
// 動畫控制
// ============================================================================

int hmi_start_animation(hmi_controller_t *hmi, uint16_t screen_id, uint16_t control_id) {
    uint8_t frame[32];
    uint16_t frame_len = 0;
    
    frame[frame_len++] = FRAME_HEADER;
    frame[frame_len++] = CMD_CONFIG_BASE;
    frame[frame_len++] = CMD_ANIM_START;
    frame[frame_len++] = screen_id >> 8;
    frame[frame_len++] = screen_id & 0xFF;
    frame[frame_len++] = control_id >> 8;
    frame[frame_len++] = control_id & 0xFF;
    
    uint8_t tail[] = FRAME_TAIL;
    memcpy(frame + frame_len, tail, FRAME_TAIL_SIZE);
    frame_len += FRAME_TAIL_SIZE;
    
    return hmi_send_command(hmi, frame, frame_len);
}

int hmi_stop_animation(hmi_controller_t *hmi, uint16_t screen_id, uint16_t control_id) {
    uint8_t frame[32];
    uint16_t frame_len = 0;
    
    frame[frame_len++] = FRAME_HEADER;
    frame[frame_len++] = CMD_CONFIG_BASE;
    frame[frame_len++] = CMD_ANIM_STOP;
    frame[frame_len++] = screen_id >> 8;
    frame[frame_len++] = screen_id & 0xFF;
    frame[frame_len++] = control_id >> 8;
    frame[frame_len++] = control_id & 0xFF;
    
    uint8_t tail[] = FRAME_TAIL;
    memcpy(frame + frame_len, tail, FRAME_TAIL_SIZE);
    frame_len += FRAME_TAIL_SIZE;
    
    return hmi_send_command(hmi, frame, frame_len);
}

int hmi_pause_animation(hmi_controller_t *hmi, uint16_t screen_id, uint16_t control_id) {
    uint8_t frame[32];
    uint16_t frame_len = 0;
    
    frame[frame_len++] = FRAME_HEADER;
    frame[frame_len++] = CMD_CONFIG_BASE;
    frame[frame_len++] = CMD_ANIM_PAUSE;
    frame[frame_len++] = screen_id >> 8;
    frame[frame_len++] = screen_id & 0xFF;
    frame[frame_len++] = control_id >> 8;
    frame[frame_len++] = control_id & 0xFF;
    
    uint8_t tail[] = FRAME_TAIL;
    memcpy(frame + frame_len, tail, FRAME_TAIL_SIZE);
    frame_len += FRAME_TAIL_SIZE;
    
    return hmi_send_command(hmi, frame, frame_len);
}

int hmi_set_animation_frame(hmi_controller_t *hmi, uint16_t screen_id, uint16_t control_id, uint8_t frame_id) {
    uint8_t frame[32];
    uint16_t frame_len = 0;
    
    frame[frame_len++] = FRAME_HEADER;
    frame[frame_len++] = CMD_CONFIG_BASE;
    frame[frame_len++] = CMD_ANIM_FRAME;
    frame[frame_len++] = screen_id >> 8;
    frame[frame_len++] = screen_id & 0xFF;
    frame[frame_len++] = control_id >> 8;
    frame[frame_len++] = control_id & 0xFF;
    frame[frame_len++] = frame_id;
    
    uint8_t tail[] = FRAME_TAIL;
    memcpy(frame + frame_len, tail, FRAME_TAIL_SIZE);
    frame_len += FRAME_TAIL_SIZE;
    
    return hmi_send_command(hmi, frame, frame_len);
}

// ============================================================================
// 基本繪圖
// ============================================================================

int hmi_draw_point(hmi_controller_t *hmi, uint16_t x, uint16_t y) {
    uint8_t data[4] = {x >> 8, x & 0xFF, y >> 8, y & 0xFF};
    return hmi_send_data(hmi, CMD_DRAW_POINT, data, sizeof(data));
}

int hmi_draw_line(hmi_controller_t *hmi, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    uint8_t data[8] = {
        x0 >> 8, x0 & 0xFF, y0 >> 8, y0 & 0xFF,
        x1 >> 8, x1 & 0xFF, y1 >> 8, y1 & 0xFF
    };
    return hmi_send_data(hmi, CMD_DRAW_LINE, data, sizeof(data));
}

int hmi_draw_rectangle(hmi_controller_t *hmi, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t filled) {
    uint8_t data[8] = {
        x0 >> 8, x0 & 0xFF, y0 >> 8, y0 & 0xFF,
        x1 >> 8, x1 & 0xFF, y1 >> 8, y1 & 0xFF
    };
    uint8_t cmd = filled ? CMD_DRAW_RECT_FILL : CMD_DRAW_RECT;
    return hmi_send_data(hmi, cmd, data, sizeof(data));
}

int hmi_draw_circle(hmi_controller_t *hmi, uint16_t x, uint16_t y, uint16_t radius, uint8_t filled) {
    uint8_t data[6] = {x >> 8, x & 0xFF, y >> 8, y & 0xFF, radius >> 8, radius & 0xFF};
    uint8_t cmd = filled ? CMD_DRAW_CIRCLE_FILL : CMD_DRAW_CIRCLE;
    return hmi_send_data(hmi, cmd, data, sizeof(data));
}

int hmi_display_text(hmi_controller_t *hmi, uint16_t x, uint16_t y, uint8_t background, 
                     font_type_t font, const char *text) {
    uint16_t text_len = strlen(text);
    uint8_t *data = malloc(6 + text_len);
    if (!data) return -1;
    
    data[0] = x >> 8;
    data[1] = x & 0xFF;
    data[2] = y >> 8;
    data[3] = y & 0xFF;
    data[4] = background;
    data[5] = (uint8_t)font;
    memcpy(data + 6, text, text_len);
    
    int result = hmi_send_data(hmi, CMD_TEXT_DISPLAY, data, 6 + text_len);
    free(data);
    return result;
} 