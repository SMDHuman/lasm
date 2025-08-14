//-----------------------------------------------------------------------------
// Another big integer library to create dynamically sizing integers
// Use "#define HH_BIGINT_IMPLEMENTATION" ones in your .c file to
// implement the functions of the library
// functions return 255 on failure
//-----------------------------------------------------------------------------
// Author		: github.com/SMDHuman
// Last Update	: 14.08.2025
// Copilot used in this file
//-----------------------------------------------------------------------------
#ifndef HH_BIGINT_H
#define HH_BIGINT_H

#define ERR 255
#define INITIAL_CAPACITY 4

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Big integer structure
typedef struct {
    uint8_t *data;     // Pointer to the data
    uint8_t sign;      // Sign of the number (0 = positive, 1 = negative, 255 = unsigned)
    size_t size;       // Size of the data
    size_t capacity;   // Capacity of the data
} bigint_t;

// Function prototypes
bigint_t *bigint_create();
void bigint_destroy(bigint_t *bigint);
uint8_t bigint_resize(bigint_t *bigint, size_t new_capacity);
void bigint_set_int(bigint_t *bigint, const uint64_t value);
void bigint_set_buffer(bigint_t *bigint, const uint8_t *data, size_t size);
void bigint_set_at(bigint_t *bigint, const uint8_t value, size_t index);
void bigint_get_at(const bigint_t *bigint, uint8_t *value, size_t index);
void bigint_print(const bigint_t *bigint);
void bigint_print_hex(const bigint_t *bigint);
uint8_t bigint_add(const bigint_t *a, const bigint_t *b, bigint_t *result);
uint8_t bigint_add(const bigint_t *a, const bigint_t *b, bigint_t *result);
uint8_t bigint_subtract(const bigint_t *a, const bigint_t *b, bigint_t *result);
uint8_t bigint_multiply(const bigint_t *a, const bigint_t *b, bigint_t *result);
uint8_t bigint_divide(const bigint_t *a, const bigint_t *b, bigint_t *result);
uint8_t bigint_modulo(const bigint_t *a, const bigint_t *b, bigint_t *result);

#ifdef HH_BIGINT_IMPLEMENTATION

//-----------------------------------------------------------------------------
// Create a new big integer with a default initial capacity
bigint_t *bigint_create(){
  bigint_t *bigint = (bigint_t *)malloc(sizeof(bigint_t));
  if(!bigint) return NULL;
  bigint->data = (uint8_t *)calloc(INITIAL_CAPACITY, sizeof(uint8_t));
  bigint->sign = 0;  // Default to positive
  bigint->size = 0;
  bigint->capacity = INITIAL_CAPACITY;
  return bigint;
}

//-----------------------------------------------------------------------------
// Destroy a big integer
void bigint_destroy(bigint_t *bigint){
  if(!bigint) return;
  free(bigint->data);
  free(bigint);
}

//-----------------------------------------------------------------------------
// Set the value of a big integer
void bigint_set_int(bigint_t *bigint, const uint64_t value){
  if(!bigint) return;

  // Resize the bigint to hold the new value
  if(bigint_resize(bigint, sizeof(uint64_t)) == ERR) return;

  // Set the sign and size
  bigint->sign = (value & (1ULL << 63)) ? 1 : 0;
  bigint->size = sizeof(uint64_t);

  // Copy the value into the data buffer
  memcpy(bigint->data, &value, sizeof(uint64_t));
}

//-----------------------------------------------------------------------------
// Set the value of a big integer from a buffer
void bigint_set_buffer(bigint_t *bigint, const uint8_t *data, size_t size){
  if(!bigint || !data || size == 0) return;

  // Resize the bigint to hold the new buffer
  if(bigint_resize(bigint, size) == ERR) return;

  // Copy the data into the bigint
  memcpy(bigint->data, data, size);
  bigint->size = size;
}
//-----------------------------------------------------------------------------
// Set the value of a big integer at a specific index
void bigint_set_at(bigint_t *bigint, const uint8_t value, size_t index){
  if(!bigint || index >= bigint->capacity) return;

  bigint->data[index] = value;
}

//-----------------------------------------------------------------------------
// Get the value of a big integer at a specific index
void bigint_get_at(const bigint_t *bigint, uint8_t *value, size_t index){
  if(!bigint || !value || index >= bigint->capacity) return;

  *value = bigint->data[index];
}

//-----------------------------------------------------------------------------
// Print a big integer
void bigint_print(const bigint_t *bigint) {
  if (!bigint || !bigint->data || bigint->size == 0) {
    printf("0");
    return;
  }

  if (bigint->sign == 1) {
    printf("-");
  }

  for (size_t i = bigint->size; i > 0; i--) {
    printf("%u", bigint->data[i-1]);
  }
}

//-----------------------------------------------------------------------------
// Print a big integer in hexadecimal format
void bigint_print_hex(const bigint_t *bigint){
  if (!bigint || !bigint->data || bigint->size == 0) {
    printf("0x0");
    return;
  }

  if (bigint->sign == 1) {
    printf("-");
  }
  
  printf("0x");
  for (size_t i = bigint->size; i > 0; i--) {
    printf("%02x", bigint->data[i-1]);
  }
}

//-----------------------------------------------------------------------------
// Resize the capacity of a big integer
uint8_t bigint_resize(bigint_t *bigint, size_t new_capacity) {
  if (!bigint) return ERR;
  
  uint8_t *new_data = (uint8_t *)realloc(bigint->data, new_capacity * sizeof(uint8_t));
  if (!new_data && new_capacity > 0) return ERR;
  
  bigint->data = new_data;
  bigint->capacity = new_capacity;
  
  return 0;
}

//-----------------------------------------------------------------------------
// Compare two big integers (returns 1 if a > b, -1 if a < b, 0 if a == b)
// Only compares absolute values
static int8_t bigint_abs_compare(const bigint_t *a, const bigint_t *b) {
  if (a->size > b->size) return 1;
  if (a->size < b->size) return -1;
  
  for (size_t i = a->size; i > 0; i--) {
    if (a->data[i-1] > b->data[i-1]) return 1;
    if (a->data[i-1] < b->data[i-1]) return -1;
  }
  
  return 0;
}

//-----------------------------------------------------------------------------
// Compare two big integers considering sign (returns 1 if a > b, -1 if a < b, 0 if a == b)
static int8_t bigint_compare(const bigint_t *a, const bigint_t *b) {
  // Different signs
  if (a->sign < b->sign) return 1;  // a is positive, b is negative
  if (a->sign > b->sign) return -1; // a is negative, b is positive
  
  // Same signs, compare absolute values
  int8_t abs_cmp = bigint_abs_compare(a, b);
  
  // If negative, reverse comparison result
  return (a->sign == 0) ? abs_cmp : -abs_cmp;
}

//-----------------------------------------------------------------------------
// Add two big integers
uint8_t bigint_add(const bigint_t *a, const bigint_t *b, bigint_t *result) {
  if (!a || !b || !result) return ERR;
  
  // If signs are different, perform subtraction instead
  if (a->sign != b->sign) {
    bigint_t temp = *b;
    temp.sign = !b->sign;  // Flip the sign
    return bigint_subtract(a, &temp, result);
  }
  
  size_t max_size = (a->size > b->size) ? a->size : b->size;
  if (result->capacity < max_size + 1) {
    if (bigint_resize(result, max_size + 1) == ERR)
      return ERR;
  }
  
  uint8_t carry = 0;
  size_t i;
  
  for (i = 0; i < max_size; i++) {
    uint8_t a_digit = (i < a->size) ? a->data[i] : 0;
    uint8_t b_digit = (i < b->size) ? b->data[i] : 0;
    uint16_t sum = a_digit + b_digit + carry;
    
    result->data[i] = sum % 10;
    carry = sum / 10;
  }
  
  if (carry) {
    result->data[max_size] = carry;
    result->size = max_size + 1;
  } else {
    result->size = max_size;
  }
  
  // Set the result sign (same as inputs since signs are equal)
  result->sign = a->sign;
  
  return 0;
}

//-----------------------------------------------------------------------------
// Subtract two big integers
uint8_t bigint_subtract(const bigint_t *a, const bigint_t *b, bigint_t *result) {
  if (!a || !b || !result) return ERR;
  
  // If signs are different, perform addition instead
  if (a->sign != b->sign) {
    bigint_t temp = *b;
    temp.sign = !b->sign;  // Flip the sign
    return bigint_add(a, &temp, result);
  }
  
  // At this point, signs are the same
  int8_t cmp = bigint_abs_compare(a, b);
  const bigint_t *larger, *smaller;
  
  if (cmp >= 0) {
    larger = a;
    smaller = b;
    result->sign = a->sign;  // Keep the sign of a
  } else {
    larger = b;
    smaller = a;
    result->sign = !a->sign;  // Flip the sign since we're doing b-a
  }
  
  if (result->capacity < larger->size) {
    if (bigint_resize(result, larger->size) == ERR)
      return ERR;
  }
  
  uint8_t borrow = 0;
  size_t i;
  
  for (i = 0; i < larger->size; i++) {
    int16_t diff = larger->data[i] - borrow;
    borrow = 0;
    
    if (i < smaller->size)
      diff -= smaller->data[i];
      
    if (diff < 0) {
      diff += 10;
      borrow = 1;
    }
    
    result->data[i] = diff;
  }
  
  // Determine the actual size by removing leading zeros
  result->size = larger->size;
  while (result->size > 0 && result->data[result->size-1] == 0) {
    result->size--;
  }
  
  // If result is zero, always set sign to positive
  if (result->size == 0) {
    result->sign = 0;
  }
  
  return 0;
}

//-----------------------------------------------------------------------------
// Multiply two big integers
uint8_t bigint_multiply(const bigint_t *a, const bigint_t *b, bigint_t *result) {
  if (!a || !b || !result) return ERR;
  
  size_t result_size = a->size + b->size;
  
  if (result->capacity < result_size) {
    if (bigint_resize(result, result_size) == ERR)
      return ERR;
  }
  
  // Initialize result to 0
  memset(result->data, 0, result_size * sizeof(uint8_t));
  result->size = 0;
  
  for (size_t i = 0; i < a->size; i++) {
    uint8_t carry = 0;
    
    for (size_t j = 0; j < b->size || carry; j++) {
      uint16_t product = result->data[i+j] + a->data[i] * (j < b->size ? b->data[j] : 0) + carry;
      result->data[i+j] = product % 10;
      carry = product / 10;
    }
  }
  
  // Determine the actual size by removing leading zeros
  result->size = result_size;
  while (result->size > 0 && result->data[result->size-1] == 0) {
    result->size--;
  }
  
  // Handle result of 0
  if (result->size == 0) {
    result->size = 1;
    result->data[0] = 0;
    result->sign = 0;  // Zero is always positive
  } else {
    // Set sign: positive if signs are the same, negative otherwise
    result->sign = (a->sign == b->sign) ? 0 : 1;
  }
  
  return 0;
}

//-----------------------------------------------------------------------------
// Helper function for division: a -= b * multiplier
static void subtract_multiple(bigint_t *a, const bigint_t *b, uint8_t multiplier, size_t offset) {
  uint8_t borrow = 0;
  
  for (size_t i = 0; i < b->size || borrow; i++) {
    int16_t product = (i < b->size ? b->data[i] * multiplier : 0) + borrow;
    uint8_t digit = product % 10;
    borrow = product / 10;
    
    if (digit > a->data[offset + i]) {
      a->data[offset + i] += 10 - digit;
      borrow++;
    } else {
      a->data[offset + i] -= digit;
    }
  }
}

//-----------------------------------------------------------------------------
// Divide two big integers
uint8_t bigint_divide(const bigint_t *a, const bigint_t *b, bigint_t *result) {
  if (!a || !b || !result) return ERR;
  
  // Check for division by zero
  if (b->size == 0 || (b->size == 1 && b->data[0] == 0))
    return ERR;
  
  // If |a| < |b|, result is 0
  if (bigint_abs_compare(a, b) < 0) {
    if (result->capacity < 1) {
      if (bigint_resize(result, 1) == ERR)
        return ERR;
    }
    result->data[0] = 0;
    result->size = 1;
    result->sign = 0;  // Zero is always positive
    return 0;
  }
  
  // Create temporary copy of a (absolute value)
  bigint_t *temp = bigint_create();
  if (!temp) return ERR;
  
  if (temp->capacity < a->size) {
    if (bigint_resize(temp, a->size) == ERR) {
      bigint_destroy(temp);
      return ERR;
    }
  }
  
  memcpy(temp->data, a->data, a->size * sizeof(uint8_t));
  temp->size = a->size;
  temp->sign = 0;  // Work with absolute values
  
  // Ensure result has enough capacity
  size_t result_capacity = a->size - b->size + 1;
  if (result->capacity < result_capacity) {
    if (bigint_resize(result, result_capacity) == ERR) {
      bigint_destroy(temp);
      return ERR;
    }
  }
  
  // Initialize result to 0
  memset(result->data, 0, result_capacity * sizeof(uint8_t));
  result->size = 0;
  
  // Create a copy of b with positive sign
  bigint_t b_copy = *b;
  b_copy.sign = 0;
  
  // Perform long division
  size_t remainder_pos = temp->size - 1;
  
  while (remainder_pos >= b_copy.size - 1) {
    // Find quotient digit
    uint8_t quotient = 0;
    while (temp->data[remainder_pos] >= b_copy.data[b_copy.size - 1]) {
      subtract_multiple(temp, &b_copy, 1, remainder_pos - b_copy.size + 1);
      quotient++;
    }
    
    result->data[remainder_pos - b_copy.size + 1] = quotient;
    remainder_pos--;
  }
  
  // Determine the actual size of result
  result->size = a->size - b_copy.size + 1;
  while (result->size > 0 && result->data[result->size - 1] == 0) {
    result->size--;
  }
  
  // Handle result of 0
  if (result->size == 0) {
    result->size = 1;
    result->data[0] = 0;
    result->sign = 0;  // Zero is always positive
  } else {
    // Set sign: positive if signs are the same, negative otherwise
    result->sign = (a->sign == b->sign) ? 0 : 1;
  }
  
  bigint_destroy(temp);
  return 0;
}

//-----------------------------------------------------------------------------
// Compute modulo (remainder) of a / b
uint8_t bigint_modulo(const bigint_t *a, const bigint_t *b, bigint_t *result) {
  if (!a || !b || !result) return ERR;
  
  // Check for division by zero
  if (b->size == 0 || (b->size == 1 && b->data[0] == 0))
    return ERR;
  
  // If |a| < |b|, result is a (preserving a's sign)
  if (bigint_abs_compare(a, b) < 0) {
    if (result->capacity < a->size) {
      if (bigint_resize(result, a->size) == ERR)
        return ERR;
    }
    memcpy(result->data, a->data, a->size * sizeof(uint8_t));
    result->size = a->size;
    result->sign = a->sign;
    return 0;
  }
  
  // Create temporary quotient and copy of a
  bigint_t *quotient = bigint_create();
  bigint_t *temp = bigint_create();
  
  if (!quotient || !temp) {
    if (quotient) bigint_destroy(quotient);
    if (temp) bigint_destroy(temp);
    return ERR;
  }
  
  // Copy a to temp
  if (temp->capacity < a->size) {
    if (bigint_resize(temp, a->size) == ERR) {
      bigint_destroy(quotient);
      bigint_destroy(temp);
      return ERR;
    }
  }
  
  memcpy(temp->data, a->data, a->size * sizeof(uint8_t));
  temp->size = a->size;
  temp->sign = 0;  // Work with absolute values
  
  // Create a copy of b with positive sign
  bigint_t b_copy = *b;
  b_copy.sign = 0;
  
  // Divide |a| by |b| to get quotient
  if (bigint_divide(a, &b_copy, quotient) == ERR) {
    bigint_destroy(quotient);
    bigint_destroy(temp);
    return ERR;
  }
  
  // Calculate quotient * |b|
  bigint_t *product = bigint_create();
  if (!product) {
    bigint_destroy(quotient);
    bigint_destroy(temp);
    return ERR;
  }
  
  if (bigint_multiply(quotient, &b_copy, product) == ERR) {
    bigint_destroy(quotient);
    bigint_destroy(temp);
    bigint_destroy(product);
    return ERR;
  }
  
  // Remainder = |a| - (quotient * |b|)
  temp->sign = 0;  // Ensure we're working with absolute values
  product->sign = 0;
  
  if (bigint_subtract(temp, product, result) == ERR) {
    bigint_destroy(quotient);
    bigint_destroy(temp);
    bigint_destroy(product);
    return ERR;
  }
  
  // For modulo, result has the same sign as the dividend (a)
  if (result->size > 0 && result->data[0] != 0) {
    result->sign = a->sign;
  } else {
    result->sign = 0;  // Zero is always positive
  }
  
  bigint_destroy(quotient);
  bigint_destroy(temp);
  bigint_destroy(product);
  
  return 0;
}
#endif // HH_BIGINT_IMPLEMENTATION

#endif // HH_BIGINT_H