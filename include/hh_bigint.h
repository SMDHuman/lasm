//-----------------------------------------------------------------------------
// Another big integer library to create dynamically sizing integers
// Use "#define HH_BIGINT_IMPLEMENTATION" ones in your .c file to
// implement the functions of the library
// functions return 255 on failure
//-----------------------------------------------------------------------------
// Author		: github.com/SMDHuman
// Last Update	: 17.09.2025
//-----------------------------------------------------------------------------
#ifndef HH_BIGINT_H
#define HH_BIGINT_H

#define ERR 255

#ifndef INITIAL_CAPACITY
    #define INITIAL_CAPACITY 4
#endif

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
//-----------------------------------------------------------------------------
#ifdef HH_BIGINT_SHORT_PREFIX
#define hbi_init hh_bigint_init
#define hbi_deinit hh_bigint_deinit
#define hbi_resize hh_bigint_resize
#define hbi_set_zero hh_bigint_set_zero
#define hbi_set_int32 hh_bigint_set_int32
#define hbi_set_uint32 hh_bigint_set_uint32
#define hbi_set_uint16 hh_bigint_set_uint16
#define hbi_set_buffer hh_bigint_set_buffer
#define hbi_get_uint32 hh_bigint_get_uint32
#define hbi_get_int32 hh_bigint_get_int32
#define hbi_get_uint64 hh_bigint_get_uint64
#define hbi_get_int64 hh_bigint_get_int64
#define hbi_set_at hh_bigint_set_at
#define hbi_get_at hh_bigint_get_at
#define hbi_print hh_bigint_print
#define hbi_print_hex hh_bigint_print_hex
#define hbi_add_int32 hh_bigint_add_int32
#define hbi_subtract_int32 hh_bigint_subtract_int32
#define hbi_add hh_bigint_add
#define hbi_subtract hh_bigint_subtract
#define hbi_is_bigger hh_bigint_is_bigger
#define hbi_is_smaller hh_bigint_is_smaller
#define hbi_is_equal hh_bigint_is_equal
#define hbi_copy hh_bigint_copy
#define hbi_convert_from_string hh_bigint_convert_from_string
#define hbi_multiply hh_bigint_multiply
#define hbi_shift_left hh_bigint_shift_left
#define hbi_shift_right hh_bigint_shift_right
#define hbi_normalize hh_bigint_normalize
#endif
//-----------------------------------------------------------------------------
// Big integer structure
typedef struct {
    uint8_t *data;     // Pointer to the data
    uint8_t sign;      // Sign of the number (0 = positive, 1 = negative, 255 = unsigned)
    size_t size;       // Size of the data
} hh_bigint_t;

// Function prototypes
uint8_t hh_bigint_init(hh_bigint_t *bigint, const int32_t init_number);
uint8_t hh_bigint_deinit(hh_bigint_t *bigint);
uint8_t hh_bigint_resize(hh_bigint_t *bigint, const size_t new_capacity);
uint8_t hh_bigint_set_zero(hh_bigint_t *bigint);
uint8_t hh_bigint_set_int64(hh_bigint_t *bigint, const int64_t value);
uint8_t hh_bigint_set_int32(hh_bigint_t *bigint, const int32_t value);
uint8_t hh_bigint_set_uint32(hh_bigint_t *bigint, const uint32_t value);
uint8_t hh_bigint_set_uint16(hh_bigint_t *bigint, const uint16_t value);
uint8_t hh_bigint_set_buffer(hh_bigint_t *bigint, const void *data, const size_t size);
uint32_t hh_bigint_get_uint32(const hh_bigint_t *bigint);
int32_t hh_bigint_get_int32(const hh_bigint_t *bigint);
uint64_t hh_bigint_get_uint64(const hh_bigint_t *bigint);
int64_t hh_bigint_get_int64(const hh_bigint_t *bigint);
uint8_t hh_bigint_set_at(hh_bigint_t *bigint, const uint8_t value, const size_t index);
uint8_t hh_bigint_get_at(const hh_bigint_t *bigint, const size_t index);
uint8_t hh_bigint_print(const hh_bigint_t *bigint);
uint8_t hh_bigint_print_hex(const hh_bigint_t *bigint);
uint8_t hh_bigint_add_int32(hh_bigint_t *bigint, const int32_t value);
uint8_t hh_bigint_subtract_int32(hh_bigint_t *bigint, const int32_t value);
uint8_t hh_bigint_subtract_int64(hh_bigint_t *bigint, const int64_t value);
uint8_t hh_bigint_add(const hh_bigint_t *a, const hh_bigint_t *b, hh_bigint_t *result);
uint8_t hh_bigint_subtract(const hh_bigint_t *a, const hh_bigint_t *b, hh_bigint_t *result);
// a > b
uint8_t hh_bigint_is_bigger(const hh_bigint_t *a, const hh_bigint_t *b);
// a < b
uint8_t hh_bigint_is_smaller(const hh_bigint_t *a, const hh_bigint_t *b);
// a == b
uint8_t hh_bigint_is_equal(const hh_bigint_t *a, const hh_bigint_t *b);
uint8_t hh_bigint_copy(hh_bigint_t *to, const hh_bigint_t *from);
uint8_t hh_bigint_convert_from_string(hh_bigint_t *bigint, const char *str);
uint8_t hh_bigint_multiply(const hh_bigint_t *a, const hh_bigint_t *b, hh_bigint_t *result);
uint8_t hh_bigint_shift_left(const hh_bigint_t *bigint, const uint64_t position, hh_bigint_t *result);
uint8_t hh_bigint_shift_right(const hh_bigint_t *bigint, const uint64_t position, hh_bigint_t *result);
uint8_t hh_bigint_bitwise_or(const hh_bigint_t *a, const hh_bigint_t *b, hh_bigint_t *result);
uint8_t hh_bigint_bitwise_xor(const hh_bigint_t *a, const hh_bigint_t *b, hh_bigint_t *result);
uint8_t hh_bigint_bitwise_and(const hh_bigint_t *a, const hh_bigint_t *b, hh_bigint_t *result);
uint8_t hh_bigint_normalize(hh_bigint_t *bigint);
//uint8_t bigint_divide(const bigint_t *a, const bigint_t *b, bigint_t *result);
//uint8_t bigint_modulo(const bigint_t *a, const bigint_t *b, bigint_t *result);

//-----------------------------------------------------------------------------
#ifdef HH_BIGINT_IMPLEMENTATION
#define MAX(a, b) ((a) > (b) ? (a) : (b))
//-----------------------------------------------------------------------------
uint8_t hh_bigint_init(hh_bigint_t *bigint, int32_t init_number){
    bigint->size = INITIAL_CAPACITY;
    bigint->sign = 0;
    bigint->data = calloc(INITIAL_CAPACITY, 1);
    if(bigint->data == NULL) return ERR;

    for(uint8_t i = 0; init_number > 0; i++){
        bigint->data[i] = (uint8_t)(init_number & 0xff);
        init_number >>= 8;
    }
    return 0;
}

//-----------------------------------------------------------------------------
uint8_t hh_bigint_deinit(hh_bigint_t *bigint){
    free(bigint->data);
    bigint->size = 0;
    bigint->sign = 0;    
    return 0;
}

//-----------------------------------------------------------------------------
uint8_t hh_bigint_resize(hh_bigint_t *bigint, const size_t new_capacity){
    if(new_capacity != bigint->size){
        uint8_t *new_data = malloc(new_capacity);
        if(new_data == NULL) return ERR;
        memcpy(new_data, bigint->data, (new_capacity < bigint->size ? new_capacity : bigint->size));
        free(bigint->data);
        bigint->data = new_data;
        if(new_capacity > bigint->size){
            memset(&bigint->data[bigint->size], 0, new_capacity - bigint->size);
        }
        bigint->size = new_capacity;
    }
    if(bigint->data == NULL){
        return ERR;
    }
    return 0;
}
//-----------------------------------------------------------------------------
// Set the value of a bigint to zero
uint8_t hh_bigint_set_zero(hh_bigint_t *bigint){
    if(bigint == NULL) return ERR;
    memset(bigint->data, 0, bigint->size);
    bigint->sign = 0;
    return 0;
}
//-----------------------------------------------------------------------------
uint8_t hh_bigint_set_int64(hh_bigint_t *bigint, int64_t value){
    if(value < 0){
        bigint->sign = 1;
        value = -value;
    }else if(value > 0){
        bigint->sign = 0;
    }
    if(hh_bigint_set_buffer(bigint, &value, 8) == ERR) return ERR;
    return 0;
}
//-----------------------------------------------------------------------------
uint8_t hh_bigint_set_int32(hh_bigint_t *bigint, int32_t value){
    if(value < 0){
        bigint->sign = 1;
        value = -value;
    }else if(value > 0){
        bigint->sign = 0;
    }
    if(hh_bigint_set_buffer(bigint, &value, 4) == ERR) return ERR;
    return 0;
}
//-----------------------------------------------------------------------------
uint8_t hh_bigint_set_uint32(hh_bigint_t *bigint, const uint32_t value){
    bigint->sign = 0;
    if(hh_bigint_set_buffer(bigint, &value, 4) == ERR) return ERR;
    return 0;
}
//-----------------------------------------------------------------------------
uint8_t hh_bigint_set_uint16(hh_bigint_t *bigint, const uint16_t value){
    bigint->sign = 0;
    if(hh_bigint_set_buffer(bigint, &value, 2) == ERR) return ERR;
    return 0;
}
//-----------------------------------------------------------------------------
uint8_t hh_bigint_set_buffer(hh_bigint_t *bigint, const void *data, const size_t size){
    if(bigint->size < size){
        if(hh_bigint_resize(bigint, size) == ERR) return ERR;
    }
    memcpy(bigint->data, data, size);
    if(bigint->size > size){ 
        memset(&bigint->data[size], 0, bigint->size - size);
    }
    return 0;
}
//-----------------------------------------------------------------------------
uint32_t hh_bigint_get_uint32(const hh_bigint_t *bigint){
    if(bigint == NULL) return 0;
    uint32_t value = 0;
    for(size_t i = 0; i < 4; i++){
        value |= ((uint32_t)hh_bigint_get_at(bigint, i) << (i * 8));
    }
    return value;
}
//-----------------------------------------------------------------------------
int32_t hh_bigint_get_int32(const hh_bigint_t *bigint){
    if(bigint == NULL) return 0;
    int32_t value = 0;
    for(size_t i = 0; i < 4; i++){
        value |= ((int32_t)hh_bigint_get_at(bigint, i) << (i * 8));
    }
    if(bigint->sign) value = -value;
    return value;
}
//-----------------------------------------------------------------------------
uint64_t hh_bigint_get_uint64(const hh_bigint_t *bigint){
    if(bigint == NULL) return 0;
    uint64_t value = 0;
    for(size_t i = 0; i < 8; i++){
        value |= ((uint64_t)hh_bigint_get_at(bigint, i) << (i * 8));
    }
    return value;
}
//-----------------------------------------------------------------------------
int64_t hh_bigint_get_int64(const hh_bigint_t *bigint){
    if(bigint == NULL) return 0;
    int64_t value = 0;
    for(size_t i = 0; i < 8; i++){
        value |= ((int64_t)hh_bigint_get_at(bigint, i) << (i * 8));
    }
    if(bigint->sign) value = -value;
    return value;
}
//-----------------------------------------------------------------------------
uint8_t hh_bigint_set_at(hh_bigint_t *bigint, const uint8_t value, const size_t index){
    if(bigint->size <= index){
        if(hh_bigint_resize(bigint, index+1) == ERR) return ERR;        
    }
    bigint->data[index] = value;
    return 0;
}
//-----------------------------------------------------------------------------
uint8_t hh_bigint_get_at(const hh_bigint_t *bigint, const size_t index){
    if(bigint->size <= index) return 0;    
    return bigint->data[index];    
}

//-----------------------------------------------------------------------------
uint8_t hh_bigint_print_hex(const hh_bigint_t *bigint){
    if(bigint->sign) printf("-");
    printf("0x");
    for(size_t i = 0; i < bigint->size; i++){
        printf("%02x", bigint->data[bigint->size - i - 1]);
    }
    printf("\n");
    return 0;
}

//-----------------------------------------------------------------------------
uint8_t hh_bigint_add_int32(hh_bigint_t *bigint, const int32_t value){
    hh_bigint_t b; hh_bigint_init(&b, value);    
    hh_bigint_t c; hh_bigint_init(&c, 0);    
    hh_bigint_add(bigint, &b, &c);
    hh_bigint_copy(bigint, &c);
    hh_bigint_deinit(&b);
    hh_bigint_deinit(&c);
    return 0;
}

//-----------------------------------------------------------------------------
uint8_t hh_bigint_subtract_int32(hh_bigint_t *bigint, const int32_t value){
    hh_bigint_t b; hh_bigint_init(&b, value);    
    hh_bigint_t c; hh_bigint_init(&c, 0);    
    hh_bigint_subtract(bigint, &b, &c);
    hh_bigint_copy(bigint, &c);
    hh_bigint_deinit(&b);
    hh_bigint_deinit(&c);
    return 0;
}
//-----------------------------------------------------------------------------
uint8_t hh_bigint_subtract_int64(hh_bigint_t *bigint, const int64_t value){
    hh_bigint_t b; hh_bigint_init(&b, 0);
    hh_bigint_set_int64(&b, value);
    hh_bigint_t c; hh_bigint_init(&c, 0);
    hh_bigint_subtract(bigint, &b, &c);
    hh_bigint_copy(bigint, &c);
    hh_bigint_deinit(&b);
    hh_bigint_deinit(&c);
    return 0;
}

//-----------------------------------------------------------------------------
uint8_t hh_bigint_add(const hh_bigint_t *a, const hh_bigint_t *b, hh_bigint_t *result){
    if(a->sign & b->sign){
        result->sign = 1;
    }
    else if(a->sign){
        hh_bigint_t new_a; hh_bigint_init(&new_a, 0);
        hh_bigint_copy(&new_a, a);
        new_a.sign = 0;
        uint8_t res = hh_bigint_subtract(b, &new_a, result);
        hh_bigint_deinit(&new_a);
        return res;
    }
    else if(b->sign){
        hh_bigint_t new_b; hh_bigint_init(&new_b, 0);
        hh_bigint_copy(&new_b, b);
        new_b.sign = 0;
        uint8_t res = hh_bigint_subtract(a, &new_b, result);
        hh_bigint_deinit(&new_b);
        return res;
    }
    else{
        result->sign = 0;
    }
    uint8_t carry = 0;
    uint8_t a_val, b_val;
    uint16_t q;
    hh_bigint_t res; hh_bigint_init(&res, 0);
    size_t biggest_size = (a->size > b->size ? a->size : b->size);
    for(size_t i = 0; i < biggest_size; i++){
        a_val = hh_bigint_get_at(a, i);
        b_val = hh_bigint_get_at(b, i);
        q = a_val + b_val + carry;
        carry = q >> 8;
        hh_bigint_set_at(&res, (uint8_t)(q & 0xff), i);
    }
    if(carry > 0){
        hh_bigint_set_at(&res, carry, biggest_size);
        biggest_size++;
    }
    hh_bigint_copy(result, &res);
    hh_bigint_deinit(&res);
    return 0;
}

//-----------------------------------------------------------------------------
uint8_t hh_bigint_subtract(const hh_bigint_t *a, const hh_bigint_t *b, hh_bigint_t *result){
    if(hh_bigint_is_equal(a, b)){
        hh_bigint_set_at(result, 0, 0);
        return 0;
    }
    if(b->sign){
        hh_bigint_t new_b; hh_bigint_init(&new_b, 0);
        hh_bigint_copy(&new_b, b);
        new_b.sign = 0;
        uint8_t res = hh_bigint_add(a, &new_b, result);
        hh_bigint_deinit(&new_b);
        return res;
    }
    else if(a->sign){
        hh_bigint_t new_a; hh_bigint_init(&new_a, 0);
        hh_bigint_copy(&new_a, a);
        new_a.sign = 0;
        uint8_t res = hh_bigint_add(&new_a, b, result);
        result->sign = 1;
        hh_bigint_deinit(&new_a);
        return res;
    }
    if(hh_bigint_is_smaller(a, b)){
        uint8_t res = hh_bigint_subtract(b, a, result);
        result->sign = 1;
        return res;
    }
    uint8_t sub_carry = 0;
    uint8_t a_val, b_val;
    uint8_t q;
    size_t biggest_size = (a->size > b->size ? a->size : b->size);
    for(size_t i = 0; i < biggest_size; i++){
        a_val = hh_bigint_get_at(a, i);
        b_val = hh_bigint_get_at(b, i);
        if(a_val < b_val){
            q = (256 - b_val) + a_val - sub_carry;
            sub_carry = 1;
        }
        else{            
            q = a_val - b_val - sub_carry;
            sub_carry = 0;
        }
        hh_bigint_set_at(result, q, i);
    }
    if(biggest_size < result->size){
        memset(&result->data[biggest_size], 0, result->size-biggest_size);
    }
    return 0;
}

//-----------------------------------------------------------------------------
uint8_t hh_bigint_is_bigger(const hh_bigint_t *a, const hh_bigint_t *b){
    uint8_t a_val, b_val;
    size_t biggest_size = (a->size > b->size ? a->size : b->size);
    for(size_t i = 0; i < biggest_size; i++){
        a_val = hh_bigint_get_at(a, biggest_size - i - 1);
        b_val = hh_bigint_get_at(b, biggest_size - i - 1);
        if(a_val > b_val) return 1;
        if(a_val < b_val) return 0;
    }
    return 0;
}

//-----------------------------------------------------------------------------
uint8_t hh_bigint_is_smaller(const hh_bigint_t *a, const hh_bigint_t *b){
    uint8_t a_val, b_val;
    size_t biggest_size = (a->size > b->size ? a->size : b->size);

    for(size_t i = 0; i < biggest_size; i++){
        a_val = hh_bigint_get_at(a, biggest_size - i - 1);
        b_val = hh_bigint_get_at(b, biggest_size - i - 1);
        if(a_val > b_val){return 0;}
        if(a_val < b_val){return 1;}
    }
    return 0;
}

//-----------------------------------------------------------------------------
uint8_t hh_bigint_is_equal(const hh_bigint_t *a, const hh_bigint_t *b){
    if(a->sign != b->sign) return 0;
    size_t biggest_size = (a->size > b->size ? a->size : b->size);
    for(size_t i = 0; i < biggest_size; i++){
        if(hh_bigint_get_at(a, i) != hh_bigint_get_at(b, i)) return 0;
    }
    return 1;
}

//-----------------------------------------------------------------------------
uint8_t hh_bigint_copy(hh_bigint_t *to, const hh_bigint_t *from){
    hh_bigint_resize(to, from->size);
    to->sign = from->sign;
    hh_bigint_set_buffer(to, from->data, from->size);
    return 0;
}

//-----------------------------------------------------------------------------
// Convert a string to a bigint. Also hex for 0x, and binary for 0b works
uint8_t hh_bigint_convert_from_string(hh_bigint_t *bigint, const char *str){
    size_t len = strlen(str);
    if(len == 0) return ERR;
    hh_bigint_set_zero(bigint);

    // Check for sign
    if(str[0] == '-'){
        bigint->sign = 1;
        str++;
        len--;
    }else{
        bigint->sign = 0;
    }
    
    uint8_t base = 10;
    // Check for base 
    if(memcmp(str, "0x", 2) == 0){
        str += 2; // Skip "0x"
        len -= 2;
        base = 16;
    }else if(memcmp(str, "0b", 2) == 0){
        str += 2; // Skip "0b"
        len -= 2;
        base = 2;
    }

    // Convert string to bigint
    for(size_t i = 0; i < len; i++){
        char c = str[len - i - 1]; // Reverse order

        hh_bigint_t value; 
        if(base == 10){
            if(c >= '0' && c <= '9'){
                hh_bigint_init(&value, c - '0');
            }else{
                return ERR;
            }
        }else if(base == 16){
            if(c >= '0' && c <= '9'){
                hh_bigint_init(&value, c - '0');
            }else if(c >= 'a' && c <= 'f'){
                hh_bigint_init(&value, c - 'a' + 10);
            }else if(c >= 'A' && c <= 'F'){
                hh_bigint_init(&value, c - 'A' + 10);
            }else{
                return ERR; // Invalid character
            }
        }else if(base == 2){
            if(c == '0'){
                hh_bigint_init(&value, 0);
            }else if(c == '1'){
                hh_bigint_init(&value, 1);
            }else{
                return ERR; // Invalid character
            }
        }
        for(size_t j = 0; j < i; j++){
            hh_bigint_t base_bigint; hh_bigint_init(&base_bigint, base);
            hh_bigint_multiply(&value, &base_bigint, &value); // Shift left
        }
        hh_bigint_add(&value, bigint, bigint);
    }
    hh_bigint_normalize(bigint);
    return 0;
}
//-----------------------------------------------------------------------------
uint8_t hh_bigint_multiply(const hh_bigint_t *a, const hh_bigint_t *b, hh_bigint_t *result){
    if(a->sign != b->sign) result->sign = 1;
    else result->sign = 0;
    hh_bigint_t res; hh_bigint_init(&res, 0);

    // Perform multiplication for big numbers
    hh_bigint_t q; hh_bigint_init(&q, 0);
    for(size_t i = 0; i < b->size; i++){
        hh_bigint_t row; hh_bigint_init(&row, 0);
        for(size_t j = 0; j < a->size; j++){
            uint16_t product = hh_bigint_get_at(b, i) * hh_bigint_get_at(a, j);
            hh_bigint_t product_bigint; hh_bigint_init(&product_bigint, 0);
            hh_bigint_set_at(&product_bigint, (uint8_t)(product & 0xff), j);
            hh_bigint_set_at(&product_bigint, (uint8_t)(product >> 8), j+1);
            hh_bigint_add(&row, &product_bigint, &row);
        }
        hh_bigint_resize(&row, row.size + i);
        memcpy(&row.data[i], &row.data[0], row.size - i);
        memset(&row.data[0], 0, i);
        hh_bigint_add(&row, &res, &res);
    }
    hh_bigint_normalize(&res);
    hh_bigint_copy(result, &res);
    return 0;
}
//-----------------------------------------------------------------------------
uint8_t hh_bigint_shift_left(const hh_bigint_t *bigint, const uint64_t position, hh_bigint_t *result){
    if(position == 0){
        hh_bigint_copy(result, bigint);
        return 0;
    }
    size_t byte_shift = position / 8;
    uint8_t bit_shift = position % 8;

    hh_bigint_t res; hh_bigint_init(&res, 0);
    hh_bigint_resize(&res, bigint->size + byte_shift + 1); // +1 for potential overflow
    hh_bigint_set_zero(&res);

    for(size_t i = 0; i < bigint->size; i++){
        uint16_t shifted_value = ((uint16_t)hh_bigint_get_at(bigint, i) << bit_shift);
        hh_bigint_set_at(&res, (uint8_t)(shifted_value & 0xff), i + byte_shift);
        hh_bigint_set_at(&res, (uint8_t)(shifted_value >> 8), i + byte_shift + 1);
    }
    res.sign = bigint->sign;
    hh_bigint_normalize(&res);
    hh_bigint_copy(result, &res);
    hh_bigint_deinit(&res);
    return 0;
}
//-----------------------------------------------------------------------------
uint8_t hh_bigint_shift_right(const hh_bigint_t *bigint, const uint64_t position, hh_bigint_t *result){
    if(position == 0){
        hh_bigint_copy(result, bigint);
        return 0;
    }
    size_t byte_shift = position / 8;
    uint8_t bit_shift = position % 8;

    if(byte_shift >= bigint->size){
        hh_bigint_set_zero(result);
        return 0;
    }

    hh_bigint_t res; hh_bigint_init(&res, 0);
    hh_bigint_resize(&res, bigint->size - byte_shift);
    hh_bigint_set_zero(&res);

    for(size_t i = byte_shift; i < bigint->size; i++){
        uint16_t shifted_value = ((uint16_t)hh_bigint_get_at(bigint, i) >> bit_shift);
        if(i - byte_shift < res.size){
            hh_bigint_set_at(&res, (uint8_t)(shifted_value & 0xff), i - byte_shift);
        }
        if(bit_shift > 0 && i - byte_shift - 1 < res.size && i > byte_shift){
            uint8_t carry_over = (uint8_t)(hh_bigint_get_at(bigint, i) << (8 - bit_shift));
            uint8_t existing_value = hh_bigint_get_at(&res, i - byte_shift - 1);
            hh_bigint_set_at(&res, existing_value | carry_over, i - byte_shift - 1);
        }
    }
    res.sign = bigint->sign;
    hh_bigint_normalize(&res);
    hh_bigint_copy(result, &res);
    hh_bigint_deinit(&res);
    return 0;
}
//-----------------------------------------------------------------------------
uint8_t hh_bigint_bitwise_or(const hh_bigint_t *a, const hh_bigint_t *b, hh_bigint_t *result){
    if(a == NULL || b == NULL || result == NULL) return ERR;

    hh_bigint_init(result, 0);
    hh_bigint_resize(result, MAX(a->size, b->size));

    for(size_t i = 0; i < result->size; i++){
        uint8_t a_bit = (i < a->size) ? hh_bigint_get_at(a, i) : 0;
        uint8_t b_bit = (i < b->size) ? hh_bigint_get_at(b, i) : 0;
        hh_bigint_set_at(result, a_bit | b_bit, i);
    }
    return 0;
}
//-----------------------------------------------------------------------------
uint8_t hh_bigint_bitwise_xor(const hh_bigint_t *a, const hh_bigint_t *b, hh_bigint_t *result){
    if(a == NULL || b == NULL || result == NULL) return ERR;

    hh_bigint_init(result, 0);
    hh_bigint_resize(result, MAX(a->size, b->size));

    for(size_t i = 0; i < result->size; i++){
        uint8_t a_bit = (i < a->size) ? hh_bigint_get_at(a, i) : 0;
        uint8_t b_bit = (i < b->size) ? hh_bigint_get_at(b, i) : 0;
        hh_bigint_set_at(result, a_bit ^ b_bit, i);
    }
    return 0;
}
//-----------------------------------------------------------------------------
uint8_t hh_bigint_bitwise_and(const hh_bigint_t *a, const hh_bigint_t *b, hh_bigint_t *result){
    if(a == NULL || b == NULL || result == NULL) return ERR;

    hh_bigint_init(result, 0);
    hh_bigint_resize(result, MAX(a->size, b->size));

    for(size_t i = 0; i < result->size; i++){
        uint8_t a_bit = (i < a->size) ? hh_bigint_get_at(a, i) : 0;
        uint8_t b_bit = (i < b->size) ? hh_bigint_get_at(b, i) : 0;
        hh_bigint_set_at(result, a_bit & b_bit, i);
    }
    return 0;
}
//-----------------------------------------------------------------------------
uint8_t hh_bigint_normalize(hh_bigint_t *bigint){
    // Remove leading zeros
    size_t new_size = bigint->size;
    while(new_size > 0 && bigint->data[new_size - 1] == 0){
        new_size--;
    }
    if(new_size == 0){
        bigint->sign = 0; // Reset sign for zero
        new_size = 1; // At least one byte for zero
    }
    if(new_size < bigint->size){
        hh_bigint_resize(bigint, new_size);
    }
    return 0;
}

#endif // HH_BIGINT_IMPLEMENTATION

#endif // HH_BIGINT_H
