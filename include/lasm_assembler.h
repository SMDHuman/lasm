//-----------------------------------------------------------------------------
// lasm_assembler.h 24.07.2025
// github.com/SMDHuman
//-----------------------------------------------------------------------------
#ifndef LASM_ASSEMBLER_H
#define LASM_ASSEMBLER_H

#define DEFAULT_VECTOR_SIZE 2 // byte size

//-----------------------------------------------------------------------------
#include <stdint.h>
#include "hh_darray.h"
#include "lasm_tokenizer.h"
#include "hh_darray.h"

//-----------------------------------------------------------------------------
typedef struct{
	uint32_t address;
	uint32_t head;
	hh_darray_t *tokens;
}token_reader_t;

typedef struct{
	uint32_t value;
	uint32_t size;		// byte size
	token_t token;      // word or vector
}vector_t;

typedef struct{
	uint32_t address;
	vector_t vector;
}patch_vector_t; // Vectors to patch later

token_reader_t lasm_token_reader;
hh_darray_t lasm_vectors;
hh_darray_t lasm_patch_vectors;
hh_darray_t *lasm_byte_out;
hh_darray_t lasm_namespace_tokens;

//-----------------------------------------------------------------------------
void lasm_assemble_init();
uint8_t (*lasm_assemble_machine_next)(token_reader_t *token_reader, hh_darray_t *byte_out);
uint8_t lasm_assemble_next();
void token_reader_peek(int32_t index);
void token_reader_goto(uint32_t index);
void token_reader_next();
void token_reader_get(token_t *token);
uint8_t lasm_convert_number_token(token_t *token, uint32_t *number);

//-----------------------------------------------------------------------------
#ifdef LASM_ASSEMBLER_IMPLEMENTATION
    void lasm_assemble_init(hh_darray_t *tokens, hh_darray_t *byte_out){
        lasm_byte_out = byte_out;
        lasm_token_reader.address = 0;
        lasm_token_reader.head = 0;
        lasm_token_reader.tokens = tokens;
        hh_darray_init(&lasm_vectors, sizeof(vector_t));
        hh_darray_init(&lasm_patch_vectors, sizeof(patch_vector_t));
        hh_darray_init(&lasm_namespace_tokens, sizeof(token_t));
    }
	//----------------------------------------------------------------------------
	uint8_t lasm_assemble_next(){
	    token_t token_i0, token_i1, token_i2;
	    token_reader_get(&token_i0);
    	printf("token_i0: %s\n", token_i0.text);

    	if(lasm_token_reader.head >= hh_darray_get_item_fill(lasm_token_reader.tokens)){
    	    return 1;
    	}
        if(token_i0.id == NUMBER){
            uint32_t size = 0, value = 0;
    	    token_reader_next();
    	    token_reader_get(&token_i1);
    	    if(token_i1.id == SIZE){
                lasm_convert_number_token(&token_i1, &size);
    	    }
            lasm_convert_number_token(&token_i0, &value);
            if(size == 0){
                if(value < (1<<8)) size = 1;
                else if(value < (1<<16)) size = 2;
                else if(value < (1<<24)) size = 3;
                else size = 4;
            }
            printf("number %d size of %d\n", value, size);
            for(uint32_t i = 0; i < size; i++){
                if(i < 4){
                    uint8_t data = value >> (8*i);
                    hh_darray_append(lasm_byte_out, &data);   
                }else{
                    hh_darray_append(lasm_byte_out, 0);
                }
                lasm_token_reader.address ++;
            }
        }
	    // If current token is word 
	    else if(token_i0.id == WORD){
    	    token_reader_next();
    	    token_reader_get(&token_i1);
    	    // If next token is '{'
    	    if(token_i1.id == CBRAC_O){
    	        hh_darray_append(&lasm_namespace_tokens, &token_i0);
    	        printf("namepace: %s\n", token_i0.text);
    	    // If next token is [vector]
	        }else if (token_i1.id == VECTOR){
        	    token_reader_next();            
        	    token_reader_get(&token_i2);
        	    // If next next token is ':' then its branch vector
        	    if(token_i2.id == COLON){
                vector_t vector; vector.size= DEFAULT_VECTOR_SIZE;
	            lasm_convert_number_token(&token_i1, &vector.value);
	            token_t token_ns = {0};
	            for(uint8_t i = 0; i < hh_darray_get_item_fill(&lasm_namespace_tokens); i++){
                    token_t token_;
	                hh_darray_get(&lasm_namespace_tokens, i, &token_);
	            	strcat(token_ns.text, token_.text); strcat(token_ns.text, ".");
                }
	            strcat(token_ns.text, token_i0.text);
                strcpy(token_i0.text, token_ns.text);
	            memcpy(&vector.token, &token_i0, sizeof(token_t));
    	        hh_darray_append(&lasm_vectors, &vector);	  
    	        lasm_token_reader.address = vector.value;
     	        printf("vector: %s at %d\n", token_i0.text, vector.value);
        	    }else{
            	    token_reader_peek(-1);    	        
        	    }
	      
	        }else{
        	    token_reader_peek(-1);
	            
	        }
    	    token_reader_next();
	    }
    	else if(token_i0.id == CBRAC_C){
            if(hh_darray_get_item_fill(&lasm_namespace_tokens) > 0){
                hh_darray_popend(&lasm_namespace_tokens, 0);
            }
    	    token_reader_next();
    	}else{
    	    token_reader_next();
    	}
    	
        return 0;
	}
	//----------------------------------------------------------------------------
	void token_reader_peek(int32_t index){
	    lasm_token_reader.head += index;
	}
	//----------------------------------------------------------------------------
	void token_reader_goto(uint32_t index){
	    lasm_token_reader.head = index;
	}
	//----------------------------------------------------------------------------
	void token_reader_next(){
	    token_reader_peek(1);   
	}
	//----------------------------------------------------------------------------
	void token_reader_get(token_t *token){
	    hh_darray_get(lasm_token_reader.tokens, lasm_token_reader.head, token);
	}
	//----------------------------------------------------------------------------
	uint8_t lasm_convert_number_token(token_t *token, uint32_t *number){
	    if(token->id == NUMBER || token->id == VECTOR || token->id == SIZE){
	        if(strlen(token->text) >= 3){
	            if(memcmp(token->text, "0b", 2) == 0){ // If number binary
                    *number = strtol(&token->text[2], NULL, 2);	         
                    return 0;       
	            }else if(memcmp(token->text, "0x", 2) == 0){ // If number hexadecimal
                    *number = strtol(&token->text[2], NULL, 16);
                    return 0;
 	            }
 	        }
            // Else expect decimal
            *number = strtol(token->text, NULL, 10);
	    }else{
	        print_error_loc(token);
	        printf("Expected number but got '%s'\n", token->text);
	        return(ERR);
	    }
	    return 0;    
    }

#endif
#endif
