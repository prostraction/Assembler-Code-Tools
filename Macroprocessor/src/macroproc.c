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
	unsigned short count_of_arguments;
	struct list_code* begin_code;
	struct hash_table* begin_hash_table, *next;
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
		// встетилось макроопределение
		if (!strcmp(operator_asm, "MACRO") || !strcmp(operator_asm, "macro")) {
			int count = 0;
			unsigned short key = key_combine(label);
			unsigned int hash_table_element_number = hash(key);
			// обработка коллизии
			struct hash_table* current_hash = NULL;
			if (macro[hash_table_element_number].key != 0) {

				// коллизия встретилась впервые
				if (macro[hash_table_element_number].begin_hash_table == NULL) {
					macro[hash_table_element_number].begin_hash_table = malloc(sizeof(struct hash_table));
					macro[hash_table_element_number].begin_hash_table->begin_code = NULL;
					macro[hash_table_element_number].begin_hash_table->begin_hash_table = NULL;
					macro[hash_table_element_number].begin_hash_table->key = key;
					memset(macro[hash_table_element_number].begin_hash_table->name, '0', 15);
					macro[hash_table_element_number].begin_hash_table->next = NULL;
					macro[hash_table_element_number].begin_hash_table->formal_operands = NULL;
					current_hash = macro[hash_table_element_number].begin_hash_table;
				}
				// коллизия уже была раньше
				else {
					current_hash = macro[hash_table_element_number].begin_hash_table;
					while (current_hash->next != NULL) {
						current_hash = current_hash->next;
					}
					current_hash->next = NULL;
					current_hash->next = malloc(sizeof(struct hash_table));
					current_hash->next->begin_code = NULL;
					current_hash->next->begin_hash_table = NULL;
					current_hash->next->count_of_arguments = 0;
					current_hash->next->formal_operands = NULL;
					current_hash->next->key = 0;
					memset(current_hash->next->name, 0, 15);
					current_hash = current_hash->next;
					current_hash->next = NULL;
				}
			}
			else {
				current_hash = &macro[hash_table_element_number];
			}
			// сохранение имени и параметров макроопределения
			strcpy(current_hash->name, label);
			current_hash->key = key;
			unsigned int size_of_operand;
			current_hash->formal_operands = NULL;
			current_hash->formal_operands = (char**)realloc(current_hash->formal_operands, sizeof(char*) * (count_of_args + 1));
			for (int i = 0; i < count_of_args + 1; i++) {
				current_hash->formal_operands[i] = NULL;
			}
			current_hash->count_of_arguments = 0;

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
					current_hash->formal_operands[count] = temp_word;
					count++;
					current_hash->count_of_arguments++;
					temp_word = NULL;
				}
			}
			memset(label, 0, sizeof(label));
			memset(operator_asm, 0, sizeof(operator_asm));
			memset(operand, 0, sizeof(operand));
			memset(comment, 0, sizeof(comment));

			// сохранение кода макроопределения
			struct list_code* current_code = NULL;
			while (fgets(buffer, 256, fp)) {
				parse_string(buffer, label, operator_asm, operand, comment);
				if (!strcmp(operator_asm, "MEND") || !strcmp(operator_asm, "mend")) {
					break;
				}
				if (current_hash->begin_code == NULL) {
					current_hash->begin_code = malloc(sizeof(struct list_code));
					current_hash->begin_code->next = NULL;
					current_code = current_hash->begin_code;
				}
				else {
					current_code = current_hash->begin_code;
					while (current_code->next != NULL) {
						current_code = current_code->next;
					}
					current_code->next = malloc(sizeof(struct list_code));
					current_code = current_code->next;
					current_code->next = NULL;
				}
				current_code->label = NULL;
				current_code->operand = NULL;
				current_code->operator_asm = NULL;

				current_code->operator_asm = malloc(strlen(operator_asm));
				memset(current_code->operator_asm, '0', strlen(operator_asm));
				strcpy(current_code->operator_asm, operator_asm);

				if (strlen(label) > 0) {
					current_code->label = malloc(strlen(label));
					memset(current_code->label, '0', strlen(label));
					strcpy(current_code->label, label);
				}

				// сохранение формального параметра в виде $n
				bool operand_should_be_replaced = false;
				for (int i = 0; i < current_hash->count_of_arguments; i++) {
					if (strstr(operand, current_hash->formal_operands[i]) != NULL) {
						char* temp_str = (char*)malloc(sizeof(char*) * 10);
						sprintf(temp_str, "$%d", i);
						char* new_operand = malloc(sizeof(char) * strlen(operand + 1));
						memset(new_operand, 0, strlen(operand + 1));
						// замена формального параметра на запись $n
						int j, z;
						for (j = 0, z = 0; j < strlen(operand); j++, z++) {
							if (operand[z] != '&') {
								new_operand[z] = operand[j];
							}
							else {
								new_operand[z++] = '$';
								int k = 1;
								if (j == 0)
									j++;
								while (temp_str[k] >= '0' && temp_str[k] <= '9') {
									new_operand[z] = temp_str[k];
									j++; k++;
								}
								while (operand[j] != '\'' && !(operand[j] >= '0' && operand[j] <= '9') && j < strlen(operand)) {
									j++;
								}
								j--;
								
							}
						}
						new_operand[z++] = '\0';
						current_code->operand = new_operand;
						operand_should_be_replaced = true;
						free(temp_str);
						break;
					}
				}
				if (!operand_should_be_replaced) {

					current_code->operand = malloc(strlen(operand));
					strcpy(current_code->operand, operand);
				}
				memset(label, 0, sizeof(label));
				memset(operator_asm, 0, sizeof(operator_asm));
				memset(operand, 0, sizeof(operand));
				memset(comment, 0, sizeof(comment));
			}
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
		// пропуск макроопределения
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
		// обработка остального
		if (strlen(buffer) > 1) {
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
					fprintf(fp_out, label);
					fprintf(fp_out, ": ");
					char** fact_operands = NULL;
					// объявить фактические параметры макроопределения
					if (count_of_args > 0) {
						fact_operands = (char**)realloc(fact_operands, sizeof(char*) * (count_of_args + 1));
						for (int i = 0; i < count_of_args + 1; i++) {
							fact_operands[i] = NULL;
						}
						int size_of_operand = 0;
						int count = 0;
						// заполнить фактические параметры макроопределения
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
								fact_operands[count] = temp_word;
								count++;
								//free(temp_word);
								temp_word = NULL;
							}
						}
					}

					// написать код макроопределения
					struct list_code* current_code = current_hash->begin_code;
					while (current_code != NULL) {
						if (current_code->label != NULL) {
							fprintf(fp_out, current_code->label);
							fprintf(fp_out, ": ");
						}
						fprintf(fp_out, current_code->operator_asm);
						fprintf(fp_out, " ");
						int index = -1;
						// обработка фактических параметров
						for (int i = 0, j = 0, k = 1; i < strlen(current_code->operand); i++) {
							if (current_code->operand[i] == '$') {
								index = 0;
								i++;
								// замена операндов
								while (current_code->operand[i] >= '0' && current_code->operand[i] <= '9') {
									index += (current_code->operand[i] - '0') * k;
									i++; j++; k += 10;
								}
								if (fact_operands != NULL) {
									fprintf(fp_out, fact_operands[index]);
									i--;
								}
							}
							else {
								char* temp = malloc(sizeof(char) * 2);
								temp[0] = current_code->operand[i];
								temp[1] = '\0';
								fprintf(fp_out, temp);
								free(temp);
							}
						}
						fprintf(fp_out, "\n");
						current_code = current_code->next;
					}
					free(current_code);
					if (fact_operands != NULL) {
						for (int i = 0; i < count_of_args + 1; i++) {
							if (fact_operands[i] != NULL) {
								free(fact_operands[i]);
							}
						}
						free(fact_operands);
					}
				}
				// нет макроопределения
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

			printf("Arguments: (");
			for (int j = 0; j < macro->count_of_arguments; j++) {
				if (macro[i].formal_operands != NULL)
					if (j == macro->count_of_arguments - 1)
						printf("%s", macro[i].formal_operands[j]);
					else
						printf("%s, ", macro[i].formal_operands[j]);
			}
			printf(")\n");
			printf("Code:\n");

			struct list_code* current_code = macro[i].begin_code;
			while (current_code != NULL) {
				printf("--\t  %s %s\n", current_code->operator_asm, current_code->operand);
				current_code = current_code->next;
			}
			printf("\n");

			if (macro[i].begin_hash_table != NULL) {
				struct hash_table* current_hash = macro[i].begin_hash_table;
				while (current_hash != NULL) {
					printf("Arguments: (");
					for (int j = 0; j < current_hash->count_of_arguments; j++) {
						if (current_hash->formal_operands != NULL) {
							if (j == current_hash->count_of_arguments - 1)
								printf("%s", macro[i].formal_operands[j]);
							else
								printf("%s, ", macro[i].formal_operands[j]);
						}
					}
					printf(")");

					printf("\nCode:\n");
					struct list_code* current_code = current_hash->begin_code;
					while (current_code != NULL) {
						printf("--\t %s %s\n", current_code->operator_asm, current_code->operand);
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


	if (read("../asm_files/input.asm")) {
		debug_print();
		write("../asm_files/input.asm", "../asm_files/output.asm");
	}

	// Очистить память
	for (unsigned int i = 0; i < hash_table_size; i++) {
		struct hash_table* current_hash = NULL;
		if (macro[i].begin_hash_table != NULL) {
			current_hash = macro[i].begin_hash_table;
		}
		else {
			current_hash = &macro[i];
		}
		while (current_hash->next != NULL) {
			if (current_hash->formal_operands != NULL) {
				for (int i = 0; i < current_hash->count_of_arguments; i++) {
					if (current_hash->formal_operands[i] != NULL) {
						free(current_hash->formal_operands[i]);
						current_hash->formal_operands[i] = NULL;
					}
				}
				free(current_hash->formal_operands);
				current_hash->formal_operands = NULL;
			}
			if (current_hash->begin_code != NULL) {
				struct list_code* current_code = current_hash->begin_code;
				struct list_code* by_current_code = current_hash->begin_code;
				while (current_code != NULL) {
					if (current_code->next != NULL) {
						by_current_code = current_code->next;
					}
					else {
						by_current_code = NULL;
					}
					current_code->label = NULL;
					current_code->operand = NULL;
					current_code->operator_asm = NULL;
					if (by_current_code != NULL) {
						free(current_code);
						current_code = NULL;
						current_code = by_current_code;
					}
					else {
						current_code = NULL;
					}
				}
			}
			current_hash->begin_code = NULL;
			current_hash->count_of_arguments = 0;
			current_hash->key = 0;
			
			if (current_hash->next != NULL) {
				struct hash_table* temp = current_hash->next;
				free(current_hash);
				current_hash = NULL;
				current_hash = temp;
				temp = NULL;
			}
			else {
				free(current_hash);
				current_hash = NULL;
			}
		}

	}

}