<SNAKE_MAX_SIZE 64*2>

zp{
    UTIL_ZP
    snake{
        color: 0
        body_color: 0
        head{
            x: 0
            y: 0
            dir_x: 0 // 1 right, 255 left
            dir_y: 0 // 1 down , 255 up
        }
        body_stack{
            temp_x: 0
            temp_y: 0
            read_head: 0
            ptr: 0 .(2)
            cap: 0
            size: 0
        }
    }
    zero: 0
}

SNAKE_BODY[0x0E00::SNAKE_BODY + SNAKE_MAX_SIZE]:

<"crt_utils.l">


main[0x8000::0x9000]:
main_setup:{
    // Initial values
        SET_MEM_IMM     zp.snake.color              , (0 & 0x7) << 5 | (7 & 0x7) << 2 | (0 & 0x3)
        SET_MEM_IMM     zp.snake.body_color         , (0 & 0x7) << 5 | (5 & 0x7) << 2 | (0 & 0x3)
        SET_MEM_IMM     zp.snake.head.dir_x         , 1
        SET_MEM_IMM     zp.snake.body_stack.cap     , 128
        SET_MEM_IMM     zp.snake.body_stack.ptr     , SNAKE_BODY[0]
        SET_MEM_IMM     zp.snake.body_stack.ptr+1   , SNAKE_BODY[1]
    // Screen Resolution
        lda# WIDTH
        sta RESOLUTION
        lda# HEIGHT
        sta RESOLUTION + 1
}
main_loop:{
    // Erase drawing
        set_pixel_abs zp.snake.head.x, zp.snake.head.y, zp.zero
    // Check Inputs
    {
        lda KEY_INPUT+1 // check upper byte
        cmp# 0x01
        bne not_arrow
            lda KEY_INPUT // check lower byte
            ldy# 0
            cmp# 0x06
            bne not_arrow_right
                ldx# 1
                stx zp.snake.head.dir_x
                sty zp.snake.head.dir_y
                jmp break
            not_arrow_right:
            cmp# 0x07
            bne not_arrow_left
                ldx# 255
                stx zp.snake.head.dir_x
                sty zp.snake.head.dir_y
                jmp break
            not_arrow_left:
            cmp# 0x08
            bne not_arrow_down
                ldx# 1
                stx zp.snake.head.dir_y
                sty zp.snake.head.dir_x
                jmp break
            not_arrow_down:
            cmp# 0x09
            bne not_arrow_up
                ldx# 255
                stx zp.snake.head.dir_y
                sty zp.snake.head.dir_x
                jmp break
            not_arrow_up:
        not_arrow:
        lda KEY_INPUT+1 // check upper byte
        cmp# 0x00 
        bne not_key
            lda KEY_INPUT // check lower byte
            cmp# 32
            bne not_spacebar
                clc;
                ldy zp.snake.body_stack.size
                lda zp.snake.head.y
                sta (zp.snake.body_stack.ptr),y
                iny
                lda zp.snake.head.x
                sta (zp.snake.body_stack.ptr),y
                iny
                sty zp.snake.body_stack.size
                jmp break
            not_spacebar:
        not_key:

    break:}
    // Move snake at X
        lda zp.snake.head.x
        clc; adc zp.snake.head.dir_x
        sta zp.snake.head.x
        cmp# WIDTH // Exited from right
        bne skip_set_left
            lda# 0
            sta zp.snake.head.x
        skip_set_left:
        cmp# 255 // Exited from left
        bne skip_set_right
            lda# WIDTH-1
            sta zp.snake.head.x
        skip_set_right:
    // Move snake at Y
        lda zp.snake.head.y
        clc; adc zp.snake.head.dir_y
        sta zp.snake.head.y
        cmp# HEIGHT // Exited from bottom
        bne skip_set_top
            lda# 0
            sta zp.snake.head.y
        skip_set_top:
        cmp# 255 // Exited from top
        bne skip_set_bottom
            lda# HEIGHT-1
            sta zp.snake.head.y
        skip_set_bottom:
    // Draw
        set_pixel_abs zp.snake.head.x, zp.snake.head.y, zp.snake.color
        // Draw body
        {   clc;
            ldy zp.snake.body_stack.size
            sty zp.snake.body_stack.read_head
            cpy# 0
            beq end_body_draw
            loop_body_draw:
                ldy zp.snake.body_stack.read_head
                dey
                    lda (zp.snake.body_stack.ptr),y
                    sta zp.snake.body_stack.temp_x
                dey
                    lda (zp.snake.body_stack.ptr),y
                    sta zp.snake.body_stack.temp_y
                    set_pixel_abs zp.snake.body_stack.temp_x, zp.snake.body_stack.temp_y, zp.snake.body_color
                sty zp.snake.body_stack.read_head
                cpy #0
            bne loop_body_draw
        end_body_draw:
        }
    //Wait
        lda# 0xff
        sta subs.wait.cycle
        lda# 0xff
        sta subs.wait.cycle + 1
        jsr subs.wait
        jsr subs.wait
        jsr subs.wait
        jsr subs.wait
        jsr subs.wait
        jsr subs.wait
        jsr subs.wait
    jmp main_loop
};exit_main:brk

subs[0xA000::0xB000]{
    UTIL_IMPLEMENTATION
    wait{cycle: 0.(2)}
    wait:{
        ldx wait.cycle
        ldy wait.cycle+1
        clc
     wait_loop:
        dex
        cpx 255
        bne not_inc_y
            dey
            beq exit_wait
        not_inc_y:
        jmp wait_loop
    exit_wait: rts
    }
}

reset_vectors[0xfffa]:
  0.(2);main;HALT_ADDRESS
