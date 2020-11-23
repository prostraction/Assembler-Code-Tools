#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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
	for (i = 0; i < quadred_length; i += 4)
	{
		for (int j = 0; j < 4; j++)
		{
			key.ckey[j] = s[i + j];
		}
		key.ikey = key.ikey >> 1;
		result ^= key.ikey;
	}
	for (int j = 0; j < strlen(s) % 4; j++)
	{
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
	unsigned char *operator_asm;
	unsigned char *operand;
	struct list_code* next;
};
struct list_definition {
	unsigned char* value;	// string
	bool type;
	struct list_definition* next;
};
struct hash_table {
	unsigned char name[15];
	unsigned short int key;

	struct list_code		*begin_code, *end_code;
	struct list_definition  *begin_def, *end_def;
	struct hash_table		*begin_hash_table, *end_hash_table, *next;
} macro[hash_table_size];

//
void parse_string(char* buffer, char* label, char* operator_asm, char* operand, char* comment) {
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
	for (current_element = first_letter; current_element < pos_comment - 1; current_element++)
	{
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
	memset(buffer, 0,	sizeof(buffer));
	memset(label, 0,	sizeof(label));
	memset(operator_asm, 0, sizeof(operator_asm));
	memset(operand, 0, sizeof(operand));
	memset(comment, 0, sizeof(comment));

	while (fgets(buffer, 256, fp)) {
		parse_string(buffer, label, operator_asm, operand, comment);
		//
		if (!strcmp(operator_asm, "MACRO") || !strcmp(operator_asm, "macro")) {
			
			unsigned short key = key_combine(label);
			unsigned int hash_table_element_number = hash(key_combine(label));
			// обработка коллизии
			if (macro[hash(key_combine(label))].key != 0) {
				struct hash_table* current = NULL;
				if (macro[hash_table_element_number].begin_hash_table == NULL) {
					macro[hash_table_element_number].begin_hash_table = malloc(sizeof(struct hash_table));

					macro[hash_table_element_number].begin_hash_table->begin_code		= NULL;
					macro[hash_table_element_number].begin_hash_table->begin_hash_table = NULL;
					macro[hash_table_element_number].begin_hash_table->begin_def		= NULL;

					macro[hash_table_element_number].begin_hash_table->end_code			= NULL;
					macro[hash_table_element_number].begin_hash_table->end_hash_table	= NULL;
					macro[hash_table_element_number].begin_hash_table->end_def			= NULL;

					macro[hash_table_element_number].begin_hash_table->key = key;
					memset(macro[hash_table_element_number].begin_hash_table->name, '0', 15);

					macro[hash_table_element_number].begin_hash_table->next = NULL;
					current = macro[hash_table_element_number].begin_hash_table;

				}
				else {
					current = macro[hash_table_element_number].begin_hash_table;
					while (current->next != NULL) {
						current = current->next;
					}
					current->next = malloc(sizeof(struct hash_table));
					current->next->begin_hash_table->begin_code			= NULL;
					current->next->begin_hash_table->begin_hash_table	= NULL;
					current->next->begin_hash_table->begin_def			= NULL;

					current->next->begin_hash_table->end_code			= NULL;
					current->next->begin_hash_table->end_hash_table		= NULL;
					current->next->begin_hash_table->end_def			= NULL;

					current->next->begin_hash_table->key = key;
					memset(current->next->begin_hash_table->name, '0', 15);
					current = current->next;

				}
				////////////////////////////////////////////////////////////////////////////////////
				strcpy_s(current->name, strlen(label) + 1, label);
				unsigned int size_of_operand;
				bool type = false;
				for (unsigned int prev_size = 0, i = 0; i < strlen(operand); i++) {
					if (!type && operand[i] == '&') {
						type = true;
					}
					if (operand[i] == ',' || (i == strlen(operand) - 1)) {
						size_of_operand = (i == strlen(operand) - 1) ? (i - prev_size + 1) : (i - prev_size);
						char* temp_word = (char*)malloc(size_of_operand + 1);
						memset(temp_word, '0', size_of_operand + 1);
						for (int k = 0, j = prev_size; k < size_of_operand; k++, j++) {
							temp_word[k] = operand[j];
						}
						temp_word[size_of_operand] = '\0';
						prev_size = i + 2;

						// первый обработанный операнд макроопределения
						if (current->begin_def == NULL) {
							current->begin_def = malloc(sizeof(struct list_definition));
							current->begin_def->value = malloc(size_of_operand + 1);
							current->begin_def->type = type;
							strcpy_s(current->begin_def->value, size_of_operand + 1, temp_word);

							current->begin_def->next = NULL;
						}
						else {
							struct list_definition* current_definition = current->begin_def;
							while (current_definition->next != NULL) {
								current_definition = current_definition->next;
							}
							current_definition->next = malloc(sizeof(struct list_definition));
							current_definition->next->value = malloc(size_of_operand + 1);
							current_definition->next->type = type;
							strcpy(current_definition->next->value, temp_word);
							current_definition->next->next = NULL;

						}
						type = false;
						free(temp_word);
					}
				}
				//fgets(buffer, 256, fp);
				//parse_string(buffer, label, operator_asm, operand, comment);

				memset(label, 0, sizeof(label));
				memset(operator_asm, 0, sizeof(operator_asm));
				memset(operand, 0, sizeof(operand));
				memset(comment, 0, sizeof(comment));

				while (fgets(buffer, 256, fp)) {
					parse_string(buffer, label, operator_asm, operand, comment);
					if (!strcmp(operator_asm, "MEND") || !strcmp(operator_asm, "mend")) {
						break;
					}
					if (current->begin_code == NULL) {
						current->begin_code = malloc(sizeof(struct list_code));
						current->begin_code->operator_asm = malloc(strlen(operator_asm));
						current->begin_code->operand = malloc(strlen(operand));
						memset(current->begin_code->operator_asm, '0', strlen(operator_asm));
						memset(current->begin_code->operand, '0', strlen(operand));
						strcpy_s(current->begin_code->operator_asm, strlen(operator_asm) + 1, operator_asm);
						strcpy_s(current->begin_code->operand, strlen(operand) + 1, operand);
						current->begin_code->next = NULL;
					}
					else {
						struct list_code* current_definition = current->begin_code;
						while (current_definition->next != NULL) {
							current_definition = current_definition->next;
						}

						current_definition->next = malloc(sizeof(struct list_code));
						current_definition->next->operator_asm = malloc(strlen(operator_asm));
						current_definition->next->operand = malloc(strlen(operand));
						memset(current_definition->next->operator_asm, '0', strlen(operator_asm));
						memset(current_definition->next->operand, '0', strlen(operand));

						strcpy_s(current_definition->next->operator_asm, strlen(operator_asm) + 1, operator_asm);
						strcpy_s(current_definition->next->operand, strlen(operand) + 1, operand);
						current_definition->next->next = NULL;
					}

					memset(label, 0, sizeof(label));
					memset(operator_asm, 0, sizeof(operator_asm));
					memset(operand, 0, sizeof(operand));
					memset(comment, 0, sizeof(comment));

				}
				////////////////////////////////////////////////////////////////////////////////////
			}
			else {
				strcpy_s(macro[hash_table_element_number].name, strlen(label) + 1, label);
				macro[hash_table_element_number].key = key;
				unsigned int size_of_operand;
				bool type = false;
				for (unsigned int prev_size = 0, i = 0; i < strlen(operand); i++) {
					if (!type && operand[i] == '&') {
						type = true;
					}
					if (operand[i] == ',' || (i == strlen(operand) - 1)) {
						size_of_operand = (i == strlen(operand) - 1) ? (i - prev_size + 1) : (i - prev_size);
						char* temp_word = (char*)malloc(size_of_operand + 1);
						memset(temp_word, '0', size_of_operand + 1);
						for (int k = 0, j = prev_size; k < size_of_operand; k++, j++) {
							temp_word[k] = operand[j];
						}
						temp_word[size_of_operand] = '\0';
						prev_size = i + 2;

						// первый обработанный операнд макроопределения
						if (macro[hash_table_element_number].begin_def == NULL) {
							macro[hash_table_element_number].begin_def = malloc(sizeof(struct list_definition));
							macro[hash_table_element_number].begin_def->value = malloc(size_of_operand + 1);
							macro[hash_table_element_number].begin_def->type = type;
							strcpy_s(macro[hash_table_element_number].begin_def->value, size_of_operand + 1, temp_word);

							macro[hash_table_element_number].begin_def->next = NULL;
						}
						else {
							struct list_definition* current = macro[hash_table_element_number].begin_def;
							while (current->next != NULL) {
								current = current->next;
							}
							current->next		 = malloc(sizeof(struct list_definition));
							current->next->value = malloc(size_of_operand + 1);
							current->next->type	 = type;
							strcpy(current->next->value, temp_word);
							current->next->next = NULL;
							
						}
						type = false;
						free(temp_word);
					}
				}

				memset(label, 0, sizeof(label));
				memset(operator_asm, 0, sizeof(operator_asm));
				memset(operand, 0, sizeof(operand));
				memset(comment, 0, sizeof(comment));

				while (fgets(buffer, 256, fp)) {
					parse_string(buffer, label, operator_asm, operand, comment);
					if (!strcmp(operator_asm, "MEND") || !strcmp(operator_asm, "mend")) {
						break;
					}
					
					if (macro[hash_table_element_number].begin_code == NULL) {
						macro[hash_table_element_number].begin_code					= malloc(sizeof(struct list_code));
						macro[hash_table_element_number].begin_code->operator_asm	= malloc(strlen(operator_asm));
						macro[hash_table_element_number].begin_code->operand		= malloc(strlen(operand));
						memset(macro[hash_table_element_number].begin_code->operator_asm, '0', strlen(operator_asm));
						memset(macro[hash_table_element_number].begin_code->operand, '0',	strlen(operand));
						strcpy_s(macro[hash_table_element_number].begin_code->operator_asm, strlen(operator_asm) + 1, operator_asm);
						strcpy_s(macro[hash_table_element_number].begin_code->operand,		strlen(operand) + 1,	  operand);
						macro[hash_table_element_number].begin_code->next = NULL;
					}
					else {
						struct list_code* current = macro[hash_table_element_number].begin_code;
						while (current->next != NULL) {
							current = current->next;
						}

						current->next = malloc(sizeof(struct list_code));
						current->next->operator_asm = malloc(strlen(operator_asm));
						current->next->operand		= malloc(strlen(operand));
						memset(current->next->operator_asm,	'0', strlen(operator_asm));
						memset(current->next->operand,		'0', strlen(operand));

						strcpy_s(current->next->operator_asm,	strlen(operator_asm) + 1, operator_asm);
						strcpy_s(current->next->operand,		strlen(operand) + 1, operand);
						current->next->next = NULL;
					}

					memset(label, 0, sizeof(label));
					memset(operator_asm, 0, sizeof(operator_asm));
					memset(operand, 0, sizeof(operand));
					memset(comment, 0, sizeof(comment));

				}
			}
		}
	}
	return 1;
}

void write() {

}

void debug_print() {
	for (int i = 0; i < hash_table_size; i++) {
		if (macro[i].key != 0) {
			struct list_definition* current_def = macro[i].begin_def;
			while (current_def != NULL) {
				printf("%s: %d\n", current_def->value, current_def->type);
				current_def = current_def->next;
			}
			printf("\ncode:\n");

			struct list_code* current_code = macro[i].begin_code;
			while (current_code != NULL) {
				printf("%s %s\n", current_code->operator_asm, current_code->operand);
				current_code = current_code->next;
			}

			if (macro[i].begin_hash_table != NULL) {
				printf("\n");
				struct hash_table* current_hash = macro[i].begin_hash_table;
				while (current_hash != NULL) {
					struct list_definition* current_def = current_hash->begin_def;
					while (current_def != NULL) {
						printf("%s: %d\n", current_def->value, current_def->type);
						current_def = current_def->next;
					}
					printf("\ncode:\n");

					struct list_code* current_code = current_hash->begin_code;
					while (current_code != NULL) {
						printf("%s %s\n", current_code->operator_asm, current_code->operand);
						current_code = current_code->next;
					}
					current_hash = current_hash->next;
					printf("\n");
				}
			}
		}
	}
}

void main() {
	for (unsigned int i = 0; i < hash_table_size; i++) {
		macro[i].begin_code			= NULL;
		macro[i].begin_hash_table	= NULL;
		macro[i].begin_def			= NULL;

		macro[i].end_code			= NULL;
		macro[i].end_hash_table		= NULL;
		macro[i].end_def			= NULL;

		macro[i].key = 0;
		memset(macro[i].name, '0', 15);
	}
	

	if (read("test.asm")) {
		debug_print();
		write();
	}
}