#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>

#define bool char
#define true 1
#define false 0

/*
&name	-- формальный параметр
name	-- фактический параметр

*/

/*
macro_name MACRO	&macro_name1, &macro_name2, ..., &macro_nameN

MEND
*/


#define hash_table_size 7

// hash functions
union {
	char ckey[4];
	unsigned int ikey;
} key;
unsigned short key_combine(const char* s) {
	unsigned short result = 0;
	int quadred_length = strlen(s) - strlen(s) % 4;
	int i;
	for (i = 0; i < quadred_length; i += 4) {
		for (int j = 0; j < 4; j++) {
			key.ckey[j] = s[i + j];
		}
		key.ikey = key.ikey >> 1;
		result ^= key.ikey;
	}
	for (int j = 0; j < strlen(s) % 4; j++) {
		key.ckey[j] = s[i + j];
	}
	key.ckey[strlen(s) % 4] = '\0';
	result ^= key.ikey >> 1;
	return result;
}
unsigned int hash(unsigned short key) {
	return key % hash_table_size;
}


// hash table structure
struct list_code {
	unsigned char* label;
	unsigned char* operator_asm;
	unsigned char* operand;
	struct list_code* next;
};
struct hash_table {
	unsigned char name[15];
	unsigned short int key;

	char** formal_operands;
	unsigned short count_of_arugments;
	struct list_code* begin_code;
	struct hash_table* begin_hash_table, * next;
} macro[hash_table_size];

//
unsigned short parse_string(char* buffer, char* label, char* operator_asm, char* operand, char* comment) {
	unsigned short count_of_args = 0;
	short int current_element = 0;
	short int pos_colon = 0;   // ':'
	short int pos_comment = 0; // '/'
	for (current_element = 0; current_element != strlen(buffer); current_element++) {
		if (buffer[current_element] == ':' && pos_colon == 0) {
			pos_colon = current_element;
		}
		if (buffer[current_element] == '/' && pos_comment == 0) {
			pos_comment = current_element;
		}
		if (buffer[current_element] == ',') {
			count_of_args++;
		}
	}
	/* очистка пробелов/табов до метки/команды */
	short int first_letter = 0;
	for (current_element = 0; current_element < pos_colon; current_element++) {
		if (buffer[current_element] != ' ' && buffer[current_element] != '\t') {
			first_letter = current_element;
			break;
		}
	}
	/* заполнение метки */
	if (pos_colon != 0) {
		for (current_element = 0; first_letter < pos_colon; first_letter++, current_element++) {
			label[current_element] = buffer[first_letter];
		}
		label[first_letter] = '\0';
		first_letter++;
	}
	/* Проверка на остальные элементы строки */
	if (pos_comment == 0)
		pos_comment = strlen(buffer) + 1;
	short int is_label_only = 1;
	for (current_element = first_letter; current_element < pos_comment - 1; current_element++) {
		if (buffer[current_element] != ' ' && buffer[current_element] != '\t') {
			first_letter = current_element;
			is_label_only = 0;
			break;
		}
	}
	/* если больше в строке ничего нет, то операторы и операнды не заполняются
	(строка является исключительно меткой) */
	if (!is_label_only) {
		/* заполнение оператора до первого пробела*/
		for (current_element = 0; first_letter < strlen(buffer); first_letter++, current_element++) {
			if (buffer[first_letter] == ' ' || buffer[first_letter] == '\t' ||
				buffer[first_letter] == '\n' || buffer[first_letter] == '\r') {
				operator_asm[current_element] = '\0';
				current_element++;
				break;
			}
			operator_asm[current_element] = buffer[first_letter];
		}

		/* очистка пробелов между оператором и операндом */
		for (current_element = first_letter; current_element < pos_comment; current_element++) {
			if (buffer[current_element] != ' ' && buffer[current_element] != '\t') {
				first_letter = current_element;
				break;
			}
		}

		short int old_pos_comment = pos_comment;
		/* очистка пробелов после операндов и до комментария */
		for (--pos_comment; pos_comment > current_element; pos_comment--) {
			if ((pos_comment - 1 >= 0) &&
				(buffer[pos_comment - 1] == ' ' || buffer[pos_comment - 1] == '\t' ||
					buffer[pos_comment - 1] == '\n' || buffer[pos_comment - 1] == '\r')) {
				;
			}
			else
				break;
		}
		/* заполнение операндов до комментария (конца строки) */
		for (current_element = 0; first_letter < pos_comment; first_letter++, current_element++)
		{
			if (buffer[first_letter] == '\n' || buffer[first_letter] == '\r') {
				operand[current_element] = '\0';
				break;
			}
			if (buffer[first_letter] != '\t')
				operand[current_element] = buffer[first_letter];
		}
		operand[current_element] = '\0';
		pos_comment = old_pos_comment;
		return count_of_args;
	}
	/* заполнение комментария происходит отдельно, т.к. возможна метка с комментарием */
	current_element = 0;
	while (pos_comment < strlen(buffer))
	{
		if (buffer[pos_comment] == '\r' || buffer[pos_comment] == '\n')
			break;
		comment[current_element] = buffer[pos_comment];
		pos_comment++;
		current_element++;
	}
	comment[current_element] = '\0';
}

int read(unsigned char* name) {
	FILE* fp;
	if (!(fp = fopen(name, "r"))) {
		printf("Unable to open %s\n", name);
		return 0;
	}
	// for string parse
	char buffer[256];
	char label[15];
	char operator_asm[15];
	char operand[100];
	char comment[100];
	memset(buffer, 0, sizeof(buffer));
	memset(label, 0, sizeof(label));
	memset(operator_asm, 0, sizeof(operator_asm));
	memset(operand, 0, sizeof(operand));
	memset(comment, 0, sizeof(comment));

	while (fgets(buffer, 256, fp)) {
		unsigned short count_of_args = parse_string(buffer, label, operator_asm, operand, comment);
		if (!strcmp(operator_asm, "MACRO") || !strcmp(operator_asm, "macro")) {
			int count = 0;
			printf("%__________ %d: \n", count_of_args);
			unsigned short key = key_combine(label);
			unsigned int hash_table_element_number = hash(key);
			// обработка коллизии
			struct hash_table* current = NULL;
			if (macro[hash_table_element_number].key != 0) {

				// коллизия встретилась впервые
				if (macro[hash_table_element_number].begin_hash_table == NULL) {
					/////////////////////////// *
					macro[hash_table_element_number].begin_hash_table = malloc(sizeof(struct hash_table));
					macro[hash_table_element_number].begin_hash_table->begin_code = NULL;
					macro[hash_table_element_number].begin_hash_table->begin_hash_table = NULL;
					macro[hash_table_element_number].begin_hash_table->key = key;
					memset(macro[hash_table_element_number].begin_hash_table->name, '0', 15);
					macro[hash_table_element_number].begin_hash_table->next = NULL;
					macro[hash_table_element_number].begin_hash_table->formal_operands = NULL;
					current = macro[hash_table_element_number].begin_hash_table;
				}
				// коллизия уже была раньше
				else {
					current = macro[hash_table_element_number].begin_hash_table;
					while (current->next != NULL) {
						current = current->next;
					}
					current->next = NULL;
					current->next = malloc(sizeof(struct hash_table));
					current->next->begin_code = NULL;
					current->next->begin_hash_table = NULL;
					current->next->count_of_arugments = 0;
					current->next->formal_operands = NULL;
					current->next->key = 0;
					//current->next->name = NULL;
					memset(current->next->name, 0, 15);
					current = current->next;
					current->next = NULL;
				}
			}
			else {
				current = &macro[hash_table_element_number];
			}
			////////////////////////////////////////////////////////////////////////////////////
			strcpy_s(current->name, strlen(label) + 1, label);
			current->key = key;
			unsigned int size_of_operand;
			/////////////////////////////////////////////////////////////////////////////////////
			current->formal_operands = NULL;
			current->formal_operands = (char**)realloc(current->formal_operands, sizeof(char*) * (count_of_args + 1));
			///////////////////////////////////////// ??????????????
			for (int i = 0; i < count_of_args + 1; i++) {
				current->formal_operands[i] = NULL;
			}
			current->count_of_arugments = 0;


			for (unsigned int prev_size = 0, i = 0; i < strlen(operand); i++) {
				if (operand[i] == ',' || (i == strlen(operand) - 1)) {
					size_of_operand = (i == strlen(operand) - 1) ? (i - prev_size + 1) : (i - prev_size);
					char* temp_word = (char*)malloc(sizeof(char) * (size_of_operand + 1));
					memset(temp_word, '0', size_of_operand + 1);
					for (int k = 0, j = prev_size; k < size_of_operand; k++, j++) {
						temp_word[k] = operand[j];
					}
					temp_word[size_of_operand] = '\0';
					prev_size = i + 2;
					current->formal_operands[count] = temp_word;
					printf("%s nnnnnnnnnnnnnnnnnnnnnnn\n", current->formal_operands[count]);
					count++;
					current->count_of_arugments++;
					//free(temp_word);
					temp_word = NULL;

				}
			}
			memset(label, 0, sizeof(label));
			memset(operator_asm, 0, sizeof(operator_asm));
			memset(operand, 0, sizeof(operand));
			memset(comment, 0, sizeof(comment));

			struct list_code* current_code = NULL;
			while (fgets(buffer, 256, fp)) {
				parse_string(buffer, label, operator_asm, operand, comment);
				if (!strcmp(operator_asm, "MEND") || !strcmp(operator_asm, "mend")) {
					break;
				}
				///////////////////////////////////
				if (current->begin_code == NULL) {
					current->begin_code = malloc(sizeof(struct list_code));
					current->begin_code->next = NULL;
					current_code = current->begin_code;
					//current_code->next = NULL;
				}
				else {
					current_code = current->begin_code;
					while (current_code->next != NULL) {
						current_code = current_code->next;
					}
					////////////////// *
					current_code->next = malloc(sizeof(struct list_code));
					current_code = current_code->next;
					current_code->next = NULL;
				}
				current_code->label = NULL;
				current_code->operand = NULL;
				current_code->operator_asm = NULL;
				if (strlen(label) > 1) {
					current_code->label = malloc(strlen(label));
				}
				current_code->operator_asm = malloc(strlen(operator_asm));
				
				if (strlen(label) > 1) {
					memset(current_code->label, '0', strlen(label));
				}
				memset(current_code->operator_asm, '0', strlen(operator_asm));
				
				if (strlen(label) > 1) {
					strcpy_s(current_code->label, strlen(label) + 1, label);
				}
				strcpy_s(current_code->operator_asm, strlen(operator_asm) + 1, operator_asm);
				//strcpy_s(current_code->operand, strlen(operand) + 1, operand);

				//current_code->next = NULL;
				bool operand_should_be_replaced = false;
				for (int i = 0; i < current->count_of_arugments; i++) {
					if (strstr(operand, current->formal_operands[i]) != NULL) {
						printf("triggered\n");
						char* temp_str = (char*)malloc(sizeof(char*) * 10);
						sprintf(temp_str, "$%d\0", i);
						current_code->operand = temp_str;
						operand_should_be_replaced = true;
						break;
					}
				}
				if (!operand_should_be_replaced) {
					//current->begin_code->operand = malloc(strlen(operand));
					//memset(current->begin_code->operand, '0', strlen(operand));
					current_code->operand = malloc(strlen(operand));
					strcpy_s(current_code->operand, strlen(operand) + 1, operand);
				}
				printf("^^^^^^^^^^^^^^^^^^^^^^ %s %s\n", current_code->operand, current_code->operator_asm);
				memset(label, 0, sizeof(label));
				memset(operator_asm, 0, sizeof(operator_asm));
				memset(operand, 0, sizeof(operand));
				memset(comment, 0, sizeof(comment));
			}
			////////////////////////////////////////////////////////////////////////////////////
		}
	}
	return 1;
}



void write(char* name_input, char* name_output) {
	FILE* fp_in;
	if (!(fp_in = fopen(name_input, "r"))) {
		printf("Unable to open %s\n", name_input);
		return;
	}
	FILE* fp_out;
	if (!(fp_out = fopen(name_output, "w"))) {
		printf("Unable to open %s\n", name_output);
		return;
	}

	// for string parse
	char buffer[256];
	char label[15];
	char operator_asm[15];
	char operand[100];
	char comment[100];
	memset(buffer, 0, sizeof(buffer));
	memset(label, 0, sizeof(label));
	memset(operator_asm, 0, sizeof(operator_asm));
	memset(operand, 0, sizeof(operand));
	memset(comment, 0, sizeof(comment));
	unsigned short count_of_args = 0;

	while (fgets(buffer, 256, fp_in)) {
		count_of_args = parse_string(buffer, label, operator_asm, operand, comment);
		// skipping macro definition
		if (!strcmp(operator_asm, "MACRO") || !strcmp(operator_asm, "macro")) {
			memset(buffer, 0, sizeof(buffer));
			memset(label, 0, sizeof(label));
			memset(operator_asm, 0, sizeof(operator_asm));
			memset(operand, 0, sizeof(operand));
			memset(comment, 0, sizeof(comment));
			while (fgets(buffer, 256, fp_in)) {
				parse_string(buffer, label, operator_asm, operand, comment);
				if (!strcmp(operator_asm, "MEND") || !strcmp(operator_asm, "mend")) {
					memset(buffer, 0, sizeof(buffer));
					memset(label, 0, sizeof(label));
					memset(operator_asm, 0, sizeof(operator_asm));
					memset(operand, 0, sizeof(operand));
					memset(comment, 0, sizeof(comment));
					break;
				}
			}
		}
		// parsing other:
		//
		if (strlen(buffer) > 1) {
			printf("333333333333333333333 %d\n", count_of_args);
			unsigned short key = key_combine(operator_asm);
			unsigned int hash_table_element_number = hash(key_combine(operator_asm));
			bool found = false;
			if (macro[hash_table_element_number].key != 0) {
				// есть макроопределение, нет коллизии
				struct hash_table* current_hash = NULL;
				if (macro[hash_table_element_number].key == key) {
					found = true;
					current_hash = &macro[hash_table_element_number];
				}
				// поиск макроопределения, есть коллизия
				else {
					current_hash = macro[hash_table_element_number].begin_hash_table;
					while (current_hash != NULL) {

						if (current_hash->key == key) {
							found = true;
							break;
						}
						current_hash = current_hash->next;
					}
				}
					if (found) {
						fprintf(fp_out, "; ");
						fprintf(fp_out, label);
						fprintf(fp_out, ": ");
						fprintf(fp_out, operator_asm);
						fprintf(fp_out, " ");
						fprintf(fp_out, operand);
						fprintf(fp_out, "\n");

						////////////////// 
						// TO DO OPERAND
						/////////////////////////////


						struct list_code* current_code = current_hash->begin_code;
						while (current_code != NULL) {

							if (current_code->label != NULL) {
								fprintf(fp_out, current_code->label);
								fprintf(fp_out, ": ");
							}
							unsigned int index = -1;
							char* to_be_replaced = malloc(sizeof(char) * strlen(current_code->operand));
							memset(to_be_replaced, 0, strlen(current_code->operand));
							for (int i = 0, j = 0, k = 1; i < strlen(current_code->operand); i++) {
								if (current_code->operand[i] == '$') {
									index = 0;
									i++;
									while (current_code->operand[i] >= '0' && current_code->operand[i] <= '9') {
										to_be_replaced[j] = current_code->operand[i];
										index += (current_code->operand[i] - '0') * k;
										i++; j++; k += 10;
									}
									to_be_replaced[j] = '\0';
								}
							}
							printf("2222222222222222222222222222222222		%d		%s\n", index, to_be_replaced);
							fprintf(fp_out, current_code->operator_asm);
							fprintf(fp_out, " ");
							fprintf(fp_out, current_code->operand);
							fprintf(fp_out, "\n");
							current_code = current_code->next;
						}
						free(current_code);
					}
					if (!found) {
						if (strlen(label) > 0) {
							fprintf(fp_out, label);
							fprintf(fp_out, ": ");
						}
						fprintf(fp_out, operator_asm);
						fprintf(fp_out, " ");
						fprintf(fp_out, operand);
						fprintf(fp_out, "\n");
					}
					current_hash = NULL;
			}
			else {
				if (strlen(label) > 0) {
					fprintf(fp_out, label);
					fprintf(fp_out, ": ");
				}
				fprintf(fp_out, operator_asm);
				fprintf(fp_out, " ");
				fprintf(fp_out, operand);
				fprintf(fp_out, "\n");
			}
			// иначе нет макроопределения
			//printf("%s\n", operator_asm);
		}
		memset(buffer, 0, sizeof(buffer));
		memset(label, 0, sizeof(label));
		memset(operator_asm, 0, sizeof(operator_asm));
		memset(operand, 0, sizeof(operand));
		memset(comment, 0, sizeof(comment));
	}

}

void debug_print() {
	for (int i = 0; i < hash_table_size; i++) {
		if (macro[i].key != 0) {

			printf("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx \n");
			for (int j = 0; j < macro->count_of_arugments; j++) {
				if (macro[i].formal_operands != NULL)
					printf("%s \n", macro[i].formal_operands[j]);
			}
			printf("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx \n");
			printf("\ncode:\n");

			struct list_code* current_code = macro[i].begin_code;
			while (current_code != NULL) {
				printf("+++++++++ %s %s\n", current_code->operator_asm, current_code->operand);
				current_code = current_code->next;
			}

			if (macro[i].begin_hash_table != NULL) {
				printf("111111111111111111111111111111111111111111111111111111111111111111111 %p\n", macro[i].begin_hash_table);
				struct hash_table* current_hash = macro[i].begin_hash_table;
				while (current_hash != NULL) {
					printf("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx \n");
					for (int j = 0; j < current_hash->count_of_arugments; j++) {
						if (current_hash->formal_operands != NULL)
							printf("%s \n", macro[i].formal_operands[j]);
					}
					printf("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx \n");

					printf("\ncode:\n");
					struct list_code* current_code = current_hash->begin_code;
					while (current_code != NULL) {
						printf("----------- %s %s\n", current_code->operator_asm, current_code->operand);
						current_code = current_code->next;
					}
					free(current_code);
					current_hash = current_hash->next;
					printf("\n");

				}
				free(current_hash);
			}
		}
	}
}

void main() {
	for (unsigned int i = 0; i < hash_table_size; i++) {
		macro[i].begin_code = NULL;
		macro[i].begin_hash_table = NULL;
		macro[i].next = NULL;

		macro[i].key = 0;
		memset(macro[i].name, '0', 15);
	}


	if (read("test.asm")) {
		debug_print();
		write("test.asm", "m_test.asm");
	}
}