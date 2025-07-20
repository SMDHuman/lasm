#include <stdint.h>
#include "../../include/tokenizer.h"
#include "../../include/hh_darray.h"

void assemble_6502(hh_darray_t *tokens, char out_name[]){
	FILE *outf = fopen(out_name, "w");
	
	for(uint32_t i = 0; i < hh_darray_get_item_fill(tokens); i++){
		token_t token;
		hh_darray_get(tokens, i, &token);
		if(token.id == NEWLINE) fprintf(outf, "\n");
		else if(token.id == STRING_DB) fprintf(outf, "\"%s\" ", token.text);
		else if(token.id == STRING_SG) fprintf(outf,"\'%s\' ", token.text);
		else if(token.id == VECTOR) fprintf(outf,"[%s] ", token.text);
		else if(token.id == SIZE) fprintf(outf,".%s ", token.text);
		else fprintf(outf, "%s ", token.text);
		//printf("id: %d, line: %d, col: %d, text: '%s'\n", 
		//		token.id, token.line, token.col, token.text);
	}
	
	fclose(outf);
}
