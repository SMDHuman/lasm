<SNAKE_MAX_SIZE 64*2>

zp[0::256]{
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
    food{
        color: 0
        x:0
        y:0
    }
    zero: 0
}

SNAKE_BODY[0x0E00::SNAKE_BODY + SNAKE_MAX_SIZE]:

<WIDTH 20>
<HEIGHT 15>
<"crt_utils.l">

//-----------------------------------------------------------------------------
main[0x8000::0x9000]:
main_setup:{
    // Module Setup
        UTIL_SETUP
    // Initial values
        SET_MEM_IMM     zp.snake.color              , (0 & 0x7) << 5 | (7 & 0x7) << 2 | (0 & 0x3)
        SET_MEM_IMM     zp.snake.body_color         , (0 & 0x7) << 5 | (3 & 0x7) << 2 | (0 & 0x3)
        SET_MEM_IMM     zp.food.color               , (7 & 0x7) << 5 | (0 & 0x7) << 2 | (0 & 0x3)
        SET_MEM_IMM     zp.snake.head.dir_x         , 1
        SET_MEM_IMM     zp.snake.body_stack.cap     , 128
        SET_MEM_IMM     zp.snake.body_stack.ptr     , SNAKE_BODY[0]
        SET_MEM_IMM     zp.snake.body_stack.ptr+1   , SNAKE_BODY[1]
    // food
        jsr subs.erase_food
        jsr subs.pick_food_place
        jsr subs.draw_food
}
//-----------------------------------------------------------------------------
main_loop:{
    // Erase drawing
        jsr subs.erase_snake
    // Check Inputs
        jsr subs.handle_inputs
    // Move snake
        jsr subs.move_snake
    // Check food eaten 
        jsr subs.check_food_eaten
    // Draw
        jsr subs.draw_snake
        jsr subs.draw_food
    //Wait
        wait_set_cycle 0xffff
        jsr subs.wait
        jsr subs.wait
        jsr subs.wait
        jsr subs.wait
        jsr subs.wait
        jsr subs.wait
        jsr subs.wait
        jsr subs.wait
        jsr subs.wait
        jsr subs.wait
        jsr subs.wait
        jsr subs.wait
        jsr subs.wait
    jmp main_loop
};exit_main:brk

//-----------------------------------------------------------------------------
subs[0xA000::0xB000]{
    UTIL_IMPLEMENTATION
    //-------------------------------------------------------------------------
    <wait_set_cycle <val>;lda# <val> & 0xff;sta subs.wait.cycle;lda# <val> >> 8;sta subs.wait.cycle + 1;>
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
            beq break
        not_inc_y:
        jmp wait_loop
    break: rts
    }
    //-------------------------------------------------------------------------
    grow_snake:{
        //check if snake has capacity
        ldy zp.snake.body_stack.size
        cpy zp.snake.body_stack.cap
        beq break
        // grow
        lda zp.snake.head.y
        sta (zp.snake.body_stack.ptr),y
        iny
        lda zp.snake.head.x
        sta (zp.snake.body_stack.ptr),y
        iny
        sty zp.snake.body_stack.size
        break: rts
    }
    //-------------------------------------------------------------------------
    draw_snake:{
        // Draw body
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
        // Draw Head
            set_pixel_abs zp.snake.head.x, zp.snake.head.y, zp.snake.color
        rts
    }
    //-------------------------------------------------------------------------
    move_snake:{
        // Move body parts
        {   clc;
            lda zp.snake.body_stack.size
            cmp# 3
            bcc break // size < 3, means needs least 2 bodies
            lda zp.snake.body_stack.size
            sbc# 2 // start from one before
            sta zp.snake.body_stack.read_head
            loop_body_read:
                ldy zp.snake.body_stack.read_head
                dey
                    lda (zp.snake.body_stack.ptr),y
                    sta zp.snake.body_stack.temp_x
                dey
                    lda (zp.snake.body_stack.ptr),y
                    sta zp.snake.body_stack.temp_y
                // write positions to previus one
                ldy zp.snake.body_stack.read_head
                iny
                    lda zp.snake.body_stack.temp_x
                    sta (zp.snake.body_stack.ptr),y
                dey
                    lda zp.snake.body_stack.temp_y
                    sta (zp.snake.body_stack.ptr),y
                dey;dey
                sty zp.snake.body_stack.read_head
                cpy #0
            bne loop_body_read
                // read head
                move_one_body:
                    iny
                    lda zp.snake.head.x
                    sta (zp.snake.body_stack.ptr),y
                    dey
                    lda zp.snake.head.y
                    sta (zp.snake.body_stack.ptr),y
                jmp real_break
        break:
            lda zp.snake.body_stack.size
            cmp# 1
            ldy# 0
            bcs move_one_body // size < 1, means needs least 1 bodies
        real_break:
        }
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
    rts
    }
    //-------------------------------------------------------------------------
    handle_inputs:{
        lda KEY_INPUT+1 // check upper byte
        cmp# 0x01
        bne not_arrow
            lda KEY_INPUT // check lower byte
            ldy# 0
            cmp# 0x06
            bne not_arrow_right
                ldx zp.snake.head.dir_x
                cpx# 255
                beq break // check if its going other side
                ldx# 1
                stx zp.snake.head.dir_x
                sty zp.snake.head.dir_y // set zero
                jmp break
            not_arrow_right:
            cmp# 0x07
            bne not_arrow_left
                ldx zp.snake.head.dir_x
                cpx# 1
                beq break // check if its going other side
                ldx# 255
                stx zp.snake.head.dir_x
                sty zp.snake.head.dir_y // set zero
                jmp break
            not_arrow_left:
            cmp# 0x08
            bne not_arrow_down
                ldx zp.snake.head.dir_y
                cpx# 255
                beq break // check if its going other side
                ldx# 1
                stx zp.snake.head.dir_y
                sty zp.snake.head.dir_x // set zero
                jmp break
            not_arrow_down:
            cmp# 0x09
            bne not_arrow_up
                ldx zp.snake.head.dir_y
                cpx# 1
                beq break // check if its going other side
                ldx# 255
                stx zp.snake.head.dir_y
                sty zp.snake.head.dir_x // set zero
                jmp break
            not_arrow_up:
        not_arrow:
        lda KEY_INPUT+1 // check upper byte
        cmp# 0x00 
        bne not_key
            lda KEY_INPUT // check lower byte
            cmp# 32
            bne not_spacebar
                jsr subs.grow_snake 
        jsr subs.erase_food
        jsr subs.pick_food_place
        jsr subs.draw_food
                jmp break
            not_spacebar:
        not_key:
    break:
    rts
    }
    //-------------------------------------------------------------------------
    erase_snake:{
        // Erase Head
        set_pixel_abs zp.snake.head.x, zp.snake.head.y, zp.zero
        // Erase last body part
        {
            lda zp.snake.body_stack.size
            cmp# 1
            bcc break // size < 1, means needs least 1 bodies
            ldy zp.snake.body_stack.size
            dey
                lda (zp.snake.body_stack.ptr),y
                sta zp.snake.body_stack.temp_x
            dey
                lda (zp.snake.body_stack.ptr),y
                sta zp.snake.body_stack.temp_y
            set_pixel_abs zp.snake.body_stack.temp_x, zp.snake.body_stack.temp_y, zp.zero
            break:
        }
    rts
    }
    //-------------------------------------------------------------------------
    pick_food_place:{
        jsr subs.rand_8
            cmp# WIDTH
            bcc not_again_x
        again_x:
            sbc# WIDTH
            cmp# WIDTH
            bcs again_x
        not_again_x:
            sta zp.food.x

        jsr subs.rand_8
            cmp# HEIGHT
            bcc not_again_y
        again_y:
            sbc# HEIGHT
            cmp# HEIGHT
            bcs again_y
        not_again_y:
            sta zp.food.y

        rts
    }
    //-------------------------------------------------------------------------
    draw_food:{
        set_pixel_abs zp.food.x, zp.food.y, zp.food.color
        rts
    }
    //-------------------------------------------------------------------------
    erase_food:{
        set_pixel_abs zp.food.x, zp.food.y, zp.zero
        rts
    }
    //-------------------------------------------------------------------------
    check_food_eaten:{
        lda zp.snake.head.x
        cmp zp.food.x
        bne break
            lda zp.snake.head.y
            cmp zp.food.y
            bne break
                jsr erase_food
                jsr pick_food_place
                jsr draw_food
                jsr grow_snake

        break:
        rts
    }
    // http://www.6502.org/users/mycorner/6502/code/prng.html
    // returns pseudo random 8 bit number in A. Affects A. (r_seed) is the
    // byte from which the number is generated and MUST be initialised to a
    // non zero value or this function will always return zero. Also r_seed		
    // must be in RAM, you can see why......
    rand_8:{
            LDA	zp.r_seed   // get seed
            ASL A           // shift byte
            BCC	no_eor      // branch if no carry

            EOR	#0xCF        // else EOR with $CF
        no_eor:
            STA	zp.r_seed   // save number as next seed
            RTS             // done
    }
    is_inside_snake{
        res: x: 0; y: 0;
    }
    is_inside_snake:{
        // TODO
        rts
    }
}

//-----------------------------------------------------------------------------
reset_vectors[0xfffa]:
  0.(2);main;HALT_ADDRESS
