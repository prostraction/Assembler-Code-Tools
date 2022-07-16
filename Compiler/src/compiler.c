#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <locale.h>
#include <string.h>
#include <stdio.h>   
#include <stdlib.h>

#define M 27
#define word_size 3

int begin_adr = 0;
int end_adr = 0;

struct table
{
	char mnemonic[20];
	short int type_of_command; // 0 -- mashine c, 1 -- asm mnemonic
	short int size_of_command;
	unsigned short int hash;
	int cop;
	int machine_code;	
	int referenced_by;
	int address;
	void (*function_mnemonic) (int*, char* s);
	void (*perform_machine_code) (int* mash_code, char* s);
};

union 
{
	char ckey[4];
	unsigned int ikey;
} key;


unsigned short hash(unsigned short key)
{
	return key % M;
}

unsigned short key_combine(const char* s) 
{
	unsigned short result = 0;
	// длина строки кратная 4
	int quadred_length = strlen(s) - strlen(s) % 4;
	int i;
	// обработка строки по 4 символа
	for (i = 0; i < quadred_length; i += 4) 
	{
		// заполняем 4 символа
		for (int j = 0; j < 4; j++) 
		{
			key.ckey[j] = s[i + j];
		}
		// рассматриваем полученные символы как целое число
		key.ikey = key.ikey >> 1; // сдвиг полученных символов на 1
		result ^= key.ikey;		  // сложение по XOR с предыдущим полученным числом
	}
	// обработка остальной строки (меньше 4 символов)
	for (int j = 0; j < strlen(s) % 4; j++)
	{
		key.ckey[j] = s[i + j];
	}
	key.ckey[strlen(s) % 4] = '\0';
	result ^= key.ikey >> 1;
	return result;
}


int fix_collision(struct table t[M], int t_index)
{
	if (t[t_index].hash != 0)
	{
		int new_index = t_index;
		// поиск свободного элемента
		int table_is_full = 1;
		for (int j = 0; j < M; j++)
		{
			// элемент свободен
			if (t[j].hash == 0)
			{
				return j;
			}
		}
		if (table_is_full)
		{
			return -1;
		}
	}
		return -2;
}


int add_to_table(struct table t[M], struct table new_elem)
{
	int new_index = -3;
	int key = key_combine(new_elem.mnemonic);	// сжатие ключа
	int t_index = hash(key);	// получение хеша
	new_index = fix_collision(t, t_index);
	if (new_index == -1)
		return -1;
	// ячейка свободна для записи
	if (new_index == -2)
	{
		if (t[t_index].hash == key)
		{
			printf("This element is already in the table!\n");
			return -3;
		}
		t[t_index].hash = key;
		//t[t_index].mnemonic = new_elem.mnemonic;
		strcpy(t[t_index].mnemonic, new_elem.mnemonic);
		t[t_index].type_of_command = new_elem.type_of_command;
		t[t_index].machine_code = new_elem.machine_code;
		t[t_index].size_of_command = new_elem.size_of_command;
		t[t_index].function_mnemonic = new_elem.function_mnemonic;
		t[t_index].address = new_elem.address;
		t[t_index].cop = new_elem.cop;
		t[t_index].perform_machine_code = new_elem.perform_machine_code;
		return t_index;
	}
	else
	{
		if (t[t_index].hash == key)
		{
			printf("This element is already in the table!\n");
			return -3;
		}
		while (t[t_index].referenced_by != M + 1)
		{	
			// элемент уже есть в таблице
			if (t[t_index].hash == key)
			{
				printf("This element is already in the table!\n");
				return -3;
			}
			t_index = t[t_index].referenced_by;
		}
		t[t_index].referenced_by = new_index;
		t[new_index].hash = key;
		memset(t[new_index].mnemonic, 0, 20);
		strcpy(t[new_index].mnemonic, new_elem.mnemonic);
		t[new_index].type_of_command = new_elem.type_of_command;
		t[new_index].machine_code = new_elem.machine_code;
		t[new_index].size_of_command = new_elem.size_of_command;
		t[new_index].function_mnemonic = new_elem.function_mnemonic;
		t[new_index].address = new_elem.address;
		t[new_index].cop = new_elem.cop;
		t[new_index].perform_machine_code = new_elem.perform_machine_code;
		return new_index;
	}
}



int search(struct table t[M], char* word)
{
	short int found = 0;
	short int temp_referance;
	int key = key_combine(word);
	int t_index = hash(key);
	if (t[t_index].hash == key)
	{
		return t_index;
	}
	else if (t[t_index].hash != key)
	{
		if (t[t_index].referenced_by != M + 1)
		{
			//temp_referance = t[t_index].referenced_by;
			while (t[t_index].referenced_by != M + 1)
			{
				if (t[t_index].hash == key)
				{
					found = 1;
					break;
				}
				t_index = t[t_index].referenced_by;
			}
			if (t[t_index].hash == key)
			{
				found = 1;
			}
			if (found)
			{
				int index = t_index;
				return index;
			}
			else
			{
				return -1;
			}
		}
		else
		{
			return -1;
		}
	}
	else if (t[t_index].hash == 0)
	{
		return -1;
	}
}

struct table add_to_element(char* s)
{
	struct table element;
	memset(element.mnemonic, 0, 20);
	strcpy(element.mnemonic, s);
	return element;
}
///////////////////////////////////////////////////////////////////////
// обработчик директивы BEGIN
void begin(int* c, char* operand) 
{
	*c = strtol(operand, NULL, 0);
	begin_adr = strtol(operand, NULL, 0);
}
// обработчик директивы END
void end(int* c, char* operand) 
{
	printf(">> END: First executed command: %s\n", operand);
}
// обработчик директивы DW
void dw(int* c, char* operand) 
{
	*c += word_size;
}
// обработчик директивы DB
void db(int* c, char* operand) 
{
	size_t n = strtol(operand, NULL, 0);
	while (n) 
	{
		n = n >> 8;
		++(*c);
	}
}
// обработчик директивы RB
void rb(int* c, char* operand) 
{
	*c += strtol(operand, NULL, 0);
}
// обработчик директивы RW
void rw(int* c, char* operand) 
{
	*c += word_size * strtol(operand, NULL, 0);
}
///////////////////////////////////////////////////////////////////////
void begin_mashine(int* c, char* operand)
{
	*c = -1;
}
void end_mashine(int* c, char* operand)
{
	*c = -1;
}
void dw_mashine(int* c, char* operand)
{
	*c += strtol(operand, NULL, 0);
}
void db_mashine(int* c, char* operand)
{
	int n = strtol(operand, NULL, 0);
	while (n)
	{
		n = n >> 8;
		++(*c);
	}
}
void rw_mashine(int* c, char* operand)
{
	*c = 0;
}
void rb_mashine(int* c, char* operand)
{
	*c = 0;
}
///////////////////////////////////////////////////////////////////////

void print_table(struct table t[M])
{
	for (int i = 0; i < M; i++)
	{
		if (t[i].hash != 0)
		{
			if (t[i].type_of_command == 1)
			{
				if (t[i].referenced_by == M + 1)
					printf("%d\t%d\tempty\t%s\tdirective: %d\tfunction: %p\n", i, t[i].hash, t[i].mnemonic, t[i].type_of_command, t[i].function_mnemonic);
				else
					printf("%d\t%d\t%d\t%s\tdirective: %d\tfunction: %p\n", i, t[i].hash, t[i].referenced_by, t[i].mnemonic, t[i].type_of_command, t[i].function_mnemonic);
			}
			else
			{
				if (t[i].referenced_by == M + 1)
					printf("%d\t%d\tempty\t%s\tdirective: %d\tcode: %d\tsize: %d\tCOP: %02x\n", i, t[i].hash, t[i].mnemonic, t[i].type_of_command, t[i].machine_code, t[i].size_of_command, t[i].cop);
				else
					printf("%d\t%d\t%d\t%s\tdirective: %d\tcode: %d\tsize: %d\tCOP: %02x\n", i, t[i].hash, t[i].referenced_by, t[i].mnemonic, t[i].type_of_command, t[i].machine_code, t[i].size_of_command, t[i].cop);
			}
		}
		else
		{
			printf("%d\t-----\t-----\t-----\t------------ \t------------------\n", i);
		}
	}
	printf("\n");
}
void print_name_table(struct table t[M])
{
	for (int i = 0; i < M; i++)
	{
		if (t[i].hash != 0)
		{
			printf("%d\t%s\taddress: %02x\n", i, t[i].mnemonic, t[i].address);
		}
		else
		{
			printf("%d\t---\t-------- ----\n", i);
		}
	}
}
void print_log(char* name)
{
	char bufer[256];
	memset(bufer, 0, sizeof(bufer));
	FILE* log;
	if (!(log = fopen(name, "r")))
	{
		printf("Unable to open %s\n", name);
		return;
	}
	while (fgets(bufer, 256, log))
	{
		printf("%s", bufer);
		memset(bufer, 0, sizeof(bufer));
	}
}
void parse_string(char* bufer, char* label, char* operator_asm, char* operand, char* comment)
{
	short int current_element = 0;
	short int pos_colon = 0;   // ':'
	short int pos_comment = 0; // '/'
	for (current_element = 0; current_element != strlen(bufer); current_element++)
	{
		if (bufer[current_element] == ':' && pos_colon == 0)
		{
			pos_colon = current_element;
		}
		if (bufer[current_element] == '/' && pos_comment == 0)
		{
			pos_comment = current_element;
		}
	}
	/* очистка пробелов/табов до метки/команды */
	short int first_letter = 0;
	for (current_element = 0; current_element < pos_colon; current_element++)
	{
		if (bufer[current_element] != ' ' && bufer[current_element] != '\t')
		{
			first_letter = current_element;
			break;
		}
	}
	/* заполнение метки */
	if (pos_colon != 0)
	{
		for (current_element = 0; first_letter < pos_colon; first_letter++, current_element++)
		{
			label[current_element] = bufer[first_letter];
		}
		label[first_letter] = '\0';
		first_letter++;
	}
	/* Проверка на остальные элементы строки */
	if (pos_comment == 0)
		pos_comment = strlen(bufer) + 1;
	short int is_label_only = 1;
	for (current_element = first_letter; current_element < pos_comment - 1; current_element++)
	{
		if (bufer[current_element] != ' ' && bufer[current_element] != '\t')
		{
			first_letter = current_element;
			is_label_only = 0;
			break;
		}
	}
	/* если больше в строке ничего нет, то операторы и операнды не заполняются
	(строка является исключительно меткой) */
	if (!is_label_only)
	{
		/* заполнение оператора до первого пробела*/
		for (current_element = 0; first_letter < strlen(bufer); first_letter++, current_element++)
		{
			if (bufer[first_letter] == ' ' || bufer[first_letter] == '\t' ||
				bufer[first_letter] == '\n' || bufer[first_letter] == '\r')
			{
				operator_asm[current_element] = '\0';
				current_element++;
				break;
			}
			operator_asm[current_element] = bufer[first_letter];
		}

		/* очистка пробелов между оператором и операндом */
		for (current_element = first_letter; current_element < pos_comment; current_element++)
		{
			if (bufer[current_element] != ' ' && bufer[current_element] != '\t')
			{
				first_letter = current_element;
				break;
			}
		}

		short int old_pos_comment = pos_comment;
		/* очистка пробелов после операндов и до комментария */
		for (--pos_comment; pos_comment > current_element; pos_comment--)
		{
			if ((pos_comment - 1 >= 0) &&
				(bufer[pos_comment - 1] == ' ' || bufer[pos_comment - 1] == '\t' ||
					bufer[pos_comment - 1] == '\n' || bufer[pos_comment - 1] == '\r'))
			{
				;
			}
			else
				break;
		}
		/* заполнение операндов до комментария (конца строки) */
		for (current_element = 0; first_letter < pos_comment; first_letter++, current_element++)
		{
			if (bufer[first_letter] == '\n' || bufer[first_letter] == '\r')
			{
				operand[current_element] = '\0';
				break;
			}
			if (bufer[first_letter] != '\t')
				operand[current_element] = bufer[first_letter];
		}
		operand[current_element] = '\0';
		pos_comment = old_pos_comment;
	}
	/* заполнение комментария происходит отдельно, т.к. возможна метка с комментарием */
	current_element = 0;
	while (pos_comment < strlen(bufer))
	{
		if (bufer[pos_comment] == '\r' || bufer[pos_comment] == '\n')
			break;
		comment[current_element] = bufer[pos_comment];
		pos_comment++;
		current_element++;
	}
	comment[current_element] = '\0';
}
void parse_file(char* name, int* counter, struct table t[M], struct table name_t[M])
{
	FILE* fp;
	if (!(fp = fopen(name, "r")))
	{
		printf("Unable to open %s\n", name);
		return;
	}
	FILE* log;
	if (!(log = fopen("log", "w")))
	{
		printf("Unable to open %s\n", "log");
		return;
	}
	int line = 1;
	char bufer[256];
	char label[40];
	char operator_asm[15];
	char operand[100];
	char comment[100];
	memset(bufer, 0, sizeof(bufer));
	memset(label, 0, sizeof(label));
	memset(operator_asm, 0, sizeof(operator_asm));
	memset(operand, 0, sizeof(operand));
	memset(comment, 0, sizeof(comment));

	while (fgets(bufer, 256, fp))
	{
		parse_string(bufer, label, operator_asm, operand, comment);
		if (strlen(operator_asm) < 4)
		{
			if (strlen(operand) < 5)
			{
				printf("Label: %s \t Operator: %s \t Operand: %s \t  Comment: %s\t Counter: %02x\n", label, operator_asm, operand, comment, *counter);
			}
			else
				printf("Label: %s \t Operator: %s \t Operand: %s  Comment: %s\t Counter: %02x\n", label, operator_asm, operand, comment, *counter);
		}

		else
		{
			if (strlen(operand) < 4)
				printf("Label: %s \t Operator: %s  Operand: %s \t  Comment: %s\t Counter: %02x\n", label, operator_asm, operand, comment, *counter);
			else
				printf("Label: %s \t Operator: %s Operand: %s    Comment: %s\t Counter: %02x\n", label, operator_asm, operand, comment, *counter);
		}

		int index = search(t, operator_asm);
		if (index == -1)
		{
			fprintf(log, "line %d: unknown operator \"%s\"\n", line, operator_asm);
			int empty = 0;
			for (int i = 0; i < strlen(label); i++)
			{
				if (label[i] != '\n' && label[i] != ' ')
				{
					struct table element;
					element = add_to_element(label);
					element.address = *counter;
					element.function_mnemonic = NULL;
					element.type_of_command = 0;
					add_to_table(name_t, element);
					empty = 1;
					break;
				}
			}
			if (empty == 0)
			{
				;//fprintf(log, "line %d: unknown operator \"%s\"\n", line, operator_asm);
			}
		}
		else
		{
			int l_index = search(name_t, label);
			if (l_index == -1)
			{
				
				for (int i = 0; i < strlen(label); i++)
				{
					if (label[i] != '\n' && label[i] != ' ')
					{	
						struct table element;
						element = add_to_element(label);
						element.type_of_command = t[index].type_of_command;
						element.function_mnemonic = t[index].function_mnemonic;
						element.address = *counter;
						add_to_table(name_t, element);
						break;
					}
				}
				if (t[index].type_of_command == 1)
					t[index].function_mnemonic(counter, operand);
				else
					*counter += 3;
				
			}
			else
			{
				fprintf(log,"line %d: \"%s\" is a dublicate name\n", line, label);
			}
			
		}
		
		memset(label, 0, sizeof(label));
		memset(operator_asm, 0, sizeof(operator_asm));
		memset(operand, 0, sizeof(operand));
		memset(comment, 0, sizeof(comment));
		line++;
	}
	fclose(fp);
	fclose(log);
}
void parse_file_second_time(char* name, int* counter, struct table t[M], struct table name_t[M], int* counter_for_second_parse)
{
	FILE* fp;
	if (!(fp = fopen(name, "r")))
	{
		printf("Unable to open %s\n", name);
		return;
	}
	FILE* out;
	if (!(out = fopen("../output/out", "w")))
	{
		printf("Unable to open %s\n", "out");
		return;
	}
	char bufer[256];
	char label[40];
	char operator_asm[15];
	char operand[100];
	char comment[100];	
	int line = 1;


	while (fgets(bufer, 256, fp))
	{	
		memset(label, 0, sizeof(label));
		memset(operator_asm, 0, sizeof(operator_asm));
		memset(operand, 0, sizeof(operand));
		memset(comment, 0, sizeof(comment));
		
		parse_string(bufer, label, operator_asm, operand, comment);
		memset(bufer, 0, sizeof(bufer));
		
		int index = search(t, operator_asm);
		// найдем оператор в таблице мнемоник
		if (index != -1)
		{
			if (t[index].type_of_command == 0)
			{
				
				// ищем операнд в таблице имен
				int l_index = search(name_t, operand);
				// нет запятой в операнде
				if (l_index != -1)
				{
					if (strlen(operand) > 2)
						printf("line: %d \tlabel: %s \tcommand: %s %s \tmashine code: %02x%02x \taddress: ", line, label, operator_asm, operand, t[index].cop, name_t[l_index].address);
					else
						printf("line: %d \tlabel: %s \tcommand: %s %s\t \tmashine code: %02x%02x \taddress: ", line, label, operator_asm, operand, t[index].cop, name_t[l_index].address);
					printf("%02x\n", *counter_for_second_parse);
					//t[index].function_mnemonic(counter_for_second_parse, operand);
					
					fprintf(out, ":0%d%02x00%02x%02x##\n", t[index].size_of_command, *counter_for_second_parse, t[index].cop, name_t[l_index].address);
					*counter_for_second_parse += word_size;
				}
				// есть запятая в операнде
				else
				{
					char operand_first_arg[100];
					strcpy(operand_first_arg, operand);
					int comma = 0;
					for(comma = 0; comma < strlen(operand_first_arg); comma++)
					{
						if (operand_first_arg[comma] == ',')
						{
							break;
						}
					}
					if (comma != 0)
					{
						for (comma; comma < strlen(operand_first_arg); comma++)
						{
							operand_first_arg[comma] = 0;
						}
						int l_index = search(name_t, operand_first_arg);
						// нет запятой в операнде, ищем в таблице имен
						if (l_index != -1)
						{
							int temp = name_t[l_index].address + 32768;
							//printf("%s: \t%02x%02x \n", t[index].mnemonic, t[index].cop, temp);
							printf("line: %d \tlabel: %s \tcommand: %s %s \tmashine code: %02x%02x \taddress: ", line, label, operator_asm, operand, t[index].cop, name_t[l_index].address);
							printf("%02x\n", *counter_for_second_parse);
							
							fprintf(out, ":0%d%02x00%02x%02x##\n", t[index].size_of_command, *counter_for_second_parse, t[index].cop, temp);
							*counter_for_second_parse += word_size;
						}
					
					}
					else
					{
						// без операнда
						//printf("%s: \t%02x0000 \n", t[index].mnemonic,t[index].cop);
						*counter_for_second_parse += word_size;
						printf("line: %d \tlabel: %s \tcommand: %s \t \tmashine code: %02x0000 \taddress: ", line, label, operator_asm, t[index].cop, name_t[l_index].address);
						printf("%02x\n", *counter_for_second_parse);
						
						fprintf(out, ":0%d%02x00%02x0000##\n", t[index].size_of_command, *counter_for_second_parse, t[index].cop);
						
					}
				}
			}
			// иначе перед нами директива
			else
			{
				int a = 0;
				int *i = &a;
				t[index].perform_machine_code(i, operand);

				if (t[index].function_mnemonic == begin)
				{
					printf("line: %d \tlabel: %s \tcommand: %s %s \tmashine code: ", line, label, operator_asm, operand);
					*counter_for_second_parse = begin_adr;
				}
				else if (t[index].function_mnemonic == end)
				{
					//printf("%d \t %s %s %s \t", line, label, t[index].mnemonic, operand);
					printf("line: %d \tlabel: %s \tcommand: %s %s \tmashine code: ", line, label, operator_asm, operand);
					*counter_for_second_parse = begin_adr;
				}
				else
				{
					//printf("%d \t %s %s %s \t\t", line, label, t[index].mnemonic, operand);
					printf("line: %d \tlabel: %s \tcommand: %s %s \t\tmashine code: ", line, label, operator_asm, operand);
				}
				fprintf(out, ":0%d%02x", t[index].size_of_command, *counter_for_second_parse);
				if (t[index].function_mnemonic == begin)
				{
					if (end_adr - begin_adr < 256)
						fprintf(out, "0200%02x##\n", end_adr - begin_adr);
					else if (end_adr - begin_adr < 4096)
						fprintf(out, "020%02x##\n", end_adr - begin_adr);
					else if (end_adr - begin_adr < 65536)
						fprintf(out, "02%02x##\n", end_adr - begin_adr);
				}
				else if (t[index].function_mnemonic == end)
				{
					fprintf(out, "01##\n");
				}
				else
				{
					fprintf(out, "00");
				}

				if (t[index].function_mnemonic != begin && t[index].function_mnemonic != end)
					*counter_for_second_parse += 3;
				int ttt = *i;
				if (ttt < 0)
				{
					printf("------");
					if (t[index].function_mnemonic == end)
					{
						printf(" \taddress: %02x\n", end_adr);
					}
					else
						printf("\n");
					
				}
				else if (ttt < 256)
				{
					printf("0000");
					if (t[index].function_mnemonic != begin && t[index].function_mnemonic != end)
						fprintf(out, "0000");
					
				}
				else if (ttt < 4096)
				{
					printf("00");
					if (t[index].function_mnemonic != begin && t[index].function_mnemonic != end)
						fprintf(out, "00");
				}
				else if (ttt < 65536)
				{
					printf("0");
					if (t[index].function_mnemonic != begin && t[index].function_mnemonic != end)
						fprintf(out, "0");
				}
				if (*i >= 0)
				{
					printf("%02x \taddress: %02x\n", *i, *counter_for_second_parse);
					if (t[index].function_mnemonic != begin && t[index].function_mnemonic != end)
						fprintf(out, "%02x##\n", *i);
				}
				memset(i, 0, sizeof(i));
				
			}
		}
		line++;

	}
	fclose(fp);
	fclose(out);
}

int main()
{
	struct table t[M];	// хеш-таблица [значение хеша] [наложение]
	struct table element;
	for (int i = 0; i < M; i++)
	{
		t[i].hash = 0;
		t[i].referenced_by = M + 1;
		memset(t[i].mnemonic, 0, 20);
	}
	struct table name_t[M];
	for (int i = 0; i < M; i++)
	{
		name_t[i].hash = 0;
		name_t[i].referenced_by = M + 1;
		memset(name_t[i].mnemonic, 0, 20);
	}
	char s[20];
	int start_counter = 0;
	int *counter = &start_counter;

	int counter_for_second_parse_value = 0;
	int* counter_for_second_parse = &counter_for_second_parse_value;
	
	element = add_to_element("begin");
	element.type_of_command = 1;
	element.function_mnemonic = begin;
	element.perform_machine_code = begin_mashine;
	element.size_of_command = 2;
	add_to_table(t, element);

	element = add_to_element("end");
	element.type_of_command = 1;
	element.function_mnemonic = end;
	element.perform_machine_code = end_mashine;
	element.size_of_command = 0;
	add_to_table(t, element);

	element = add_to_element("db");
	element.type_of_command = 1;
	element.function_mnemonic = db;
	element.perform_machine_code = db_mashine;
	element.size_of_command = 3;
	add_to_table(t, element);

	element = add_to_element("dw");
	element.type_of_command = 1;
	element.function_mnemonic = dw;
	element.perform_machine_code = dw_mashine;
	element.size_of_command = 3;
	add_to_table(t, element);

	element = add_to_element("rb");
	element.type_of_command = 1;
	element.function_mnemonic = rb;
	element.perform_machine_code = rb_mashine;
	element.size_of_command = 3;
	add_to_table(t, element);

	element = add_to_element("rw");
	element.type_of_command = 1;
	element.function_mnemonic = rw;
	element.perform_machine_code = rw_mashine;
	element.size_of_command = 3;
	add_to_table(t, element);

	element = add_to_element("ldx");
	element.type_of_command = 0;
	element.size_of_command = 3;
	element.machine_code = 2;
	element.cop = 34; // 22h
	add_to_table(t, element);

	element = add_to_element("lda");
	element.type_of_command = 0;
	element.size_of_command = 3;
	element.machine_code = 2;
	element.cop = 33; // 21h
	add_to_table(t, element);

	element = add_to_element("addx");
	element.type_of_command = 0;
	element.size_of_command = 3;
	element.machine_code = 2;
	element.cop = 18; // 12h
	add_to_table(t, element);

	element = add_to_element("add");
	element.type_of_command = 0;
	element.size_of_command = 3;
	element.machine_code = 2;
	element.cop = 17; // 11h
	add_to_table(t, element);

	element = add_to_element("sta");
	element.type_of_command = 0;
	element.size_of_command = 3;
	element.machine_code = 2;
	element.cop = 49; // 31h
	add_to_table(t, element);

	element = add_to_element("hlt");
	element.type_of_command = 0;
	element.size_of_command = 3;
	element.machine_code = 2;
	element.cop = 15; // ofh
	add_to_table(t, element);

	element = add_to_element("comp");
	element.type_of_command = 0;
	element.size_of_command = 3;
	element.machine_code = 2;
	element.cop = 40; // 28h
	add_to_table(t, element);

	element = add_to_element("jlt");
	element.type_of_command = 0;
	element.size_of_command = 3;
	element.machine_code = 2;
	element.cop = 136; // 88h
	add_to_table(t, element);

	element = add_to_element("jeq");
	element.type_of_command = 0;
	element.size_of_command = 3;
	element.machine_code = 2;
	element.cop = 56; // 38h
	add_to_table(t, element);

	element = add_to_element("mul");
	element.type_of_command = 0;
	element.size_of_command = 3;
	element.machine_code = 2;
	element.cop = 32; // 20h
	add_to_table(t, element);

	element = add_to_element("sub");
	element.type_of_command = 0;
	element.size_of_command = 3;
	element.machine_code = 2;
	element.cop = 112; // 70h
	add_to_table(t, element);

	element = add_to_element("rsub");
	element.type_of_command = 0;
	element.size_of_command = 3;
	element.machine_code = 2;
	element.cop = 76; // 4ch
	add_to_table(t, element);

	printf("\n---	print table	---\n");
	print_table(t);
	printf("\n---	parse file	---\n");
	parse_file("../asm_files/input.asm", counter, t, name_t);
	printf("\n");
	printf("\n---	print name table	---\n");
	print_name_table(name_t);
	printf("\n");
	printf("---	print first time	---\n");
	print_log("log");
	end_adr = *counter + 3;
	*counter = 0;
	printf("---	parse second file	---\n");
	parse_file_second_time("../asm_files/input.asm", counter, t, name_t, counter_for_second_parse);
	printf("\n");
	print_log("../output/out");
	printf("\n");

    remove("log");

	while (1)
	{
		
		printf("search input: ");
		memset(s, 0, 20);
		fgets(s, 20, stdin);
		for (int i = 0; i < 20; i++)
		{
			if (s[i] == '\n')
			{
				s[i] = 0;
			}
		}
		int index = search(t, s);
		if (index == -1)
			printf("no hash found\n");
		else
			printf("Value: %d(hash_index) = %d(key).  Test: %d\n", index, t[index].hash, (key_combine(s)));
		memset(s, 0, 20);
		
	}
}
