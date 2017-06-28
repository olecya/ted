#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <ctype.h>
#include <stdio_ext.h>

typedef struct block {
	char *p;
	size_t first;
	size_t count;
	size_t last;
	size_t current;
	char *endp;
	size_t len;
	size_t size;
	size_t num;
} Block;

typedef struct file {
	int d;
	int ds;
	int in;
} File;

char *untoa(int, char *); //перовод чисел в строку
char *getline_p(size_t line, char *); //получить указатель на строку под номером
void get_count(Block*); //посчитать строки
size_t get_size(size_t); //получить требуемый размер буфера с перекрытием в 1 страницу
void print_line(Block*); //вывод строки на печать
void phelp(void); //вывод справки
void del_line(Block*); //удаление строки
void ins_line(Block*, File*); //вставка строки

int main(int argc, char *argv[]) {
	Block t = {NULL, 0, 0, 0, 1, NULL, 0, 0, 1};
	File f;
	char suff;
	char duff[8] = {0};

        const char *wellcom = "Введите команду[m - справка]";
        const char *profi = "\033[0;33m--> \033[0m";
        int len_profi = 15;
        int len_wellcom = 49;
	size_t sparta = 1, range = 5;
	size_t tmp_size;

        struct stat staff;
        struct termios saved_attr;
        struct termios set_attr;

        f.in = open("/dev/tty", O_RDONLY);
        tcgetattr(f.in, &saved_attr);
        set_attr = saved_attr;
        set_attr.c_lflag &= ~(ICANON|ECHO|ISIG);
        set_attr.c_cc[VMIN] = 1;
        set_attr.c_cc[VTIME] = 0;
	tcsetattr(f.in, TCSAFLUSH, &set_attr);

        if(argc == 1) {
		t.len = 0;
		t.size = get_size(t.len);
		t.p = calloc(1, t.size);
        }
	else if(argc == 2){
                f.d = open(argv[1], O_RDWR|O_CREAT, 0664);
                stat(argv[1], &staff);
		t.len = staff.st_size;
		t.size = get_size(t.len);
		t.p = calloc(1, t.size);
		read(f.d, t.p, t.len);
		
        }
	else {
                write(STDERR_FILENO, "Использование: ted [имя_файла.txt]\n", 56);
                exit(EXIT_FAILURE);
        }
	get_count(&t);
        write(STDOUT_FILENO, wellcom, len_wellcom);
        write(STDOUT_FILENO, profi, len_profi);
        write(STDOUT_FILENO, "\n", 1);
    	print_line(&t);
        write(STDOUT_FILENO, profi, len_profi);

        while(read(f.in, &suff, 1) == 1 && suff != '\004') {
		if(suff == '\177') {
			write(STDOUT_FILENO, "\033[1G\033[2K", 8);
			write(STDOUT_FILENO, profi, len_profi);
			t.first = 0;
			t.last = 0;
			continue;
		}
                if(isdigit(suff) && !t.first ) { 
                        if(suff == '0' && strlen(duff) < 1)
                                continue;
			if(sparta)
				write(STDOUT_FILENO, &suff, 1); 
                        strncat(duff, &suff, 1); 
                        continue;
                }
                if(strlen(duff) > 0 && !t.first) {
                        t.first = atoi(duff);
                        duff[0] = 0;
                        if(t.first > t.count)
                                t.first = t.count;
                }
                if(suff == '$' && !t.first) {
			if(sparta)
				write(STDOUT_FILENO, &suff, 1); 
                        t.first = t.count;
                        continue;
                }
                if( suff == '-' && t.first) {
			if(sparta)
				write(STDOUT_FILENO, &suff, 1); 
                        continue;
                }
                if(isdigit((int)suff) && t.first) {
			if(sparta)
				write(STDOUT_FILENO, &suff, 1); 
                        strncat(duff, &suff, 1); 
                        continue;
                }
                if(suff == '$' && t.first) {
			if(sparta)
				write(STDOUT_FILENO, &suff, 1);
                        t.last = t.count;
                        continue;
                }
                if(t.first && strlen(duff) > 0) {
                        t.last = atoi(duff);
                        if(t.last > t.count)
                                t.last = t.count;
			if(t.last < t.first) //поставил
				t.last = t.first;
			memset(duff, 0, 8);
                }
                switch(suff) {
                        case 'p': //print
				if(sparta) {
					write(STDOUT_FILENO, &suff, 1);
					write(STDOUT_FILENO, "\n", 1);
				}
				print_line(&t);
				if(sparta)
					write(STDOUT_FILENO, profi, len_profi);
                                break;
			case 'j': //вниз на одну строку
				if(sparta) {
					write(STDOUT_FILENO, &suff, 1);
					write(STDOUT_FILENO, "\n", 1);
				}
				if(t.current < t.count)
					t.current++;
                                print_line(&t);
				if(sparta)
					write(STDOUT_FILENO, profi, len_profi);
                                break;
			case 'k': //вверх на одну строку
				if(sparta) {
					write(STDOUT_FILENO, &suff, 1);
					write(STDOUT_FILENO, "\n", 1);
				}
				if(t.current > 1)
					t.current--;
                                print_line(&t);
				if(sparta)
					write(STDOUT_FILENO, profi, len_profi);
                                break;
                        case '\040': // клавиша Space
				if(sparta) {
					write(STDOUT_FILENO, "Space", 5); 
					write(STDOUT_FILENO, "\n", 1); 
				}
                                if(t.first > 0)
                                       range = t.first;
                                if(t.current < t.count)
                                        t.current++;
				t.first = t.current;
				t.last = t.current + range - 1;
				if(t.last > t.count)
					t.last = t.count;
                                print_line(&t);
				if(sparta)
					write(STDOUT_FILENO, profi, len_profi);
                                break;
                        case 'c':
				if(t.first)
					t.current = t.first;
				else
					t.first = t.current;
				if(sparta) {
					write(STDOUT_FILENO, &suff, 1);
					write(STDOUT_FILENO, " [заменить]\033[0;36m\n", 27);
				}
				if(!t.last) {
					del_line(&t);
					get_count(&t);
				}
				t.current = t.first;
				ins_line(&t, &f);
				get_count(&t);
				tmp_size = get_size(t.len);
				if(t.size != tmp_size) {
					t.size = tmp_size;
					t.p = realloc(t.p, tmp_size);
				}
				if(sparta) {
					write(STDOUT_FILENO, "\n", 1);
					write(STDOUT_FILENO, profi, len_profi);
				}
                                break;

			case 'D': //групповушка
				if(t.last) {
					t.current = t.first;
					while(t.first <= t.last) {
						del_line(&t);
						get_count(&t);
						t.last--;
					}
				}
				else if(t.first)
					t.current = t.first;
                        case 'd':
				if(sparta) {
					write(STDOUT_FILENO, &suff, 1);
					write(STDOUT_FILENO, " [удалить]\n", 18);
				}
				if(!t.last) {
					del_line(&t);
					get_count(&t);
				}
				tmp_size = get_size(t.len);
				if(t.size != tmp_size) {
					t.size = tmp_size;
					t.p = realloc(t.p, tmp_size);
				}
				if(sparta)
					write(STDOUT_FILENO, profi, len_profi);
                                break;
			case 'A':
				t.current = t.count;
			case 'a':
				t.current++;
			case 'I':
				if(suff == 'I')
					t.current = t.first;
			case 'i':
				if(sparta) {
					write(STDOUT_FILENO, &suff, 1);
					if(suff == 'a' || suff == 'A')
						write(STDOUT_FILENO, " [добавить]", 19);
					else
						write(STDOUT_FILENO, " [вставить]", 19);
				}
				write(STDOUT_FILENO, "\n\033[0;36m", 8);
				ins_line(&t, &f);
				get_count(&t);
				tmp_size = get_size(t.len);
				if(t.size != tmp_size) {
					t.size = tmp_size;
					t.p = realloc(t.p, tmp_size);
				}
				write(STDOUT_FILENO, "\033[0m\n", 5);
				if(sparta)
					write(STDOUT_FILENO, profi, len_profi);
				break;
			case 'n': //нумерация строк
				if(sparta) {
					write(STDOUT_FILENO, &suff, 1);
					write(STDOUT_FILENO, "\n", 1);
				}
				if(t.num)
					t.num = 0;
				else
					t.num = 1;
				if(sparta)
					write(STDOUT_FILENO, profi, len_profi);
				break;
			case 's': //спартанский интерфейс
				if(sparta) {
					write(STDOUT_FILENO, &suff, 1);
					write(STDOUT_FILENO, "\n", 1);
				}
				if(sparta)
					sparta = 0;
				else
					sparta = 1;
				if(sparta)
					write(STDOUT_FILENO, profi, len_profi);
				break;
                        case 'm': //справка
				if(sparta) {
					write(STDOUT_FILENO, &suff, 1);
					write(STDOUT_FILENO, "\n", 1);
				}
                                phelp();
                                write(STDOUT_FILENO, profi, len_profi);
                                break;
			default:
				break;
		}
		t.first = 0;
		t.last = 0;
	}

	tcsetattr(f.in, TCSANOW, &saved_attr);
	exit(EXIT_SUCCESS);
}

void get_count(Block *t) {
	char *end = t->p + t->len;
	if(t->len > 0) {
		t->endp = t->p;
		size_t leng = t->len;
		for(t->count = 0; (t->endp = memchr(t->endp, '\n', leng)) != NULL; t->count++) {
//			leng = strlen(++t->endp);
			leng = (size_t)(end - ++t->endp);
		}
	}
	t->endp = t->p + t->len;
}

size_t get_size(size_t len) {
	size_t size = sysconf(_SC_PAGESIZE);
	return (len + size) / size * size + size;
}

char *getline_p(size_t line, char *p) {
	char *d = p;
	for(; line > 1; line--, d++) {
		d = (char *)memchr(d, '\n', strlen(d));
	}
	return d;
}

char *untoa(int digit, char *buff) {
	buff += 7;
	do {
		*--buff = digit % 10 + '0';
	} while(digit /= 10);
	return buff;
}

void print_line(Block *t) {
	if(t->len == 0)
		t->current = 0;
	else {
		int i = 1;
		size_t arg = t->last;
		if(t->first == 0)
			i = t->current;
		if(t->first > i)
			i = t->first;
		if(i > arg)
			arg = i;
		char *buff = calloc(1, 8);
		char *d;
		char *run = getline_p(i, t->p);
		for(; i <= arg; i++) {
			if(t->num) {
				d = untoa(i, buff);
				write(STDOUT_FILENO, "\033[1;33m", 7);
				write(STDOUT_FILENO, d, strlen(d));
				write(STDOUT_FILENO, "\033[0m ", 5);
			}
			do {
				if(run[0] == '\n')
					break;
				write(STDOUT_FILENO, run++, 1);
			} while(run[0] != '\n' && run[0] != '\0');
			write(STDOUT_FILENO, run++, 1);
		}
		free(buff);
		t->current = arg;
	}
}

void del_line(Block *t) {
	char *tmp = calloc(1, t->size);
	char *sin, *din;
	size_t size;
	if(t->count > 1) {
		sin = getline_p(t->current, t->p);
		din = getline_p(t->current + 1, t->p);
		size = (size_t)(t->endp - din);
		memcpy(tmp, din, size);
		memcpy(sin, tmp, size);
		size = (size_t)(din - sin);
		t->len = t->len - size;
		memset(t->p + t->len, 0, size);
		if(t->current == t->count)
			t->current--;
	
	}
	else if(t->count == 1 && t->len != 1) {
		sin = getline_p(1, t->p);
		t->len = strlen(sin);
		memset(sin, 0, t->len);
		t->current = 1;
	}
	else {
		memset(t->p, 0, t->len);
		t->len = 0;
		t->current = 0;
		t->count = 0;
	}
	free(tmp);
}

void ins_line(Block *t, File *f) {
	size_t count = 0;
	char *buff = calloc(1, 8);
	char guff;
	char *ins, *line, *temp;
	int i, j;
	char *tmp = calloc(1, t->size);
	if(t->current <= t->count) {
		ins = getline_p(t->current, t->p); //вставляем в текущую позицию 
	}
	else {
		ins = t->endp;
	}
	temp = ins;
	size_t size = (size_t)(t->endp - ins);
	memcpy(tmp, ins, size);
	while(read(f->in, &guff, 1) && guff != '\004') { //ввести защиту от переполнения
		if(guff == 27) {
			read(f->in, &guff, 1);
			if(guff == 91) {
				read(f->in, &guff, 1);
				read(f->in, &guff, 1);
			}
			read(f->in, &guff, 1);
			continue;
		}
		if(guff == '\n') {
			count++;
		}
		if(guff == '\177') {
			if(ins > temp) {
				ins--;
				if(*ins == '\n') {
					memset(ins, 0, 1);
					count--;
					if(*(ins - 1) != '\n') {
						for(i = 0, j = 0; ins - i >= temp && *(ins - i) != '\n';) {
							if((*(ins - i) & 0xF0) == 208) //находим первый байт кирилицы
								j++;
							i++;
						}
						i -= j;
						line = untoa(i - 1, buff);
						write(STDOUT_FILENO, "\033[A", 3);
						write(STDOUT_FILENO, "\033[", 2);
						write(STDOUT_FILENO, line, strlen(line));
						write(STDOUT_FILENO, "C", 1);
						memset(buff, 0, 8);

					}
					else
						write(STDOUT_FILENO, "\033[A", 3);
				}
				else {
					memset(ins, 0, 1);
					write(STDOUT_FILENO, "\033[D\033[X", 6);
					if((*(ins - 1) & 0xF0) == 208) {
						ins--;
						memset(ins, 0, 1);
					}

				}
			}
		}
		if(guff == '\177')
			continue;
		write(STDOUT_FILENO, &guff, 1); 
		memcpy(ins++, &guff, 1); 
	}
	if(ins > temp) {
		memcpy(ins++, "\n", 1);
	}
	memcpy(ins, tmp, size);
	free(tmp);
	free(buff);
	t->current+=count;
	t->len+=strlen(t->endp);
	if(t->len > 0 && t->count == 0)
		t->current = 1;
}

void phelp(void) {
        printf("%s\n", "p - вывод текущей строки\n\
 1-$p - вывод с 1 по последнюю строку \n\
 4-8p - вывод с 4 по 8 строку\n\
j - вывод следующей строки\n\
k - вывод предыдущей строки\n\
Space - вывод блока строк, по умолчанию 5\n\
 <n>Space - установка размера блока\n\
a - добавление с новой строки\n\
A - добавление в конец файла\n\
i - вставка текста\n\
d - удалить текущую строку\n\
<n>D - адресное или групповое удаление строк\n\
 $D - удалить последнюю строку\n\
 1-$D - удалить все строки\n\
Ctrl+d - закончить ввод текста\n\
c - замена строки\n\
w - сохранение документа\n\
q - выход\n\
n - номера строк.\n\
s - спартанский интерфейс\n\
<-- отмена\n\
m - справка");
}
