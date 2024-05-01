#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <locale.h>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#define HMAX 16
#define VMAX 16
#define BUFF 64

typedef struct
{
	double nums[VMAX][HMAX];	//Сама матрица чисел
	uint8_t hsize;		//Размер по горизонтали
	uint8_t vsize;		//Размер по вертикали
}
Matrix;

typedef struct
{
	int a;	//Верхний предел диапазона
	int b;	//Нижний предел диапазона
}
Range;

int8_t ReadFromBinary(Matrix* m, const char fname[]);
int8_t FormulaSolution(Matrix* m);
int8_t RandomSolution(Matrix* m);
char SetMatrixSize(Matrix* Matrix, const char text[]);
void FillWithFormula(Matrix* Matrix);
void FillWithRandom(Matrix* Matrix, Range* Range);
void PrintMatrix(Matrix* Matrix, FILE* sink);
double ProductOfOddElements(Matrix* Matrix);
char SelectSource(void);
uint8_t SelectOutput(void);
int8_t ReadFromTextFile(Matrix* m, const char fname[]);
int8_t PrintIntoFile(Matrix* m, const char fname[]);
int8_t WriteIntoBinary(Matrix* m, const char fname[]);
void ReadFromText(Matrix* m, FILE* source);
int8_t ReadFromKeyboard(Matrix* m);

int main(void)
{
	Matrix m1;						//Объявление матрицы
	char fname[BUFF];

	//Блок ввода
	switch (SelectSource())			//Выбор способа заполнения
	{
	default:						//Завершение работы по желанию пользователя
		puts("Завершение работы");
		return 0;
	case 'r':						//Заполнение случайными числами
	case 'R':
		if (RandomSolution(&m1))
			return -1;				//Завершение работы из-за неправильно заданного размера или диапазона
		break;
	case 'a':
	case 'A':						//Заполение согласно формуле
		if (FormulaSolution(&m1))
			return -2;				//Завершение работы из-за неправильно заданного размера
		break;
	case 'f':						//Заполнение из бинарного файла
	case 'F':
		printf("Введите название файла для чтения: ");
		scanf("%32s", fname);
		if (ReadFromBinary(&m1, fname))
		{
			printf("Не удалось открыть файл %s! Завершение работы\n", fname);
			return -1;
		}
		break;
	case 't':						//Заполнение из текстового файла
	case 'T':
		printf("Введите название файла для чтения: ");
		scanf("%32s", fname);
		if (ReadFromTextFile(&m1, fname))
		{
			return -3;
		}
		break;
	case 'k':						//Заполнение с клавиатуры
	case 'K':
		if (ReadFromKeyboard(&m1))
		{
			puts("Введены лишние символы! Завершение работы");
			return -4;
		}
		break;
	}

	//Блок вывода
	{
		char outputFlag = SelectOutput();

		if (!outputFlag)	//Завершение работы при неправильном флаге
		{
			puts("Завершение работы");
			return 0;
		}

		if (outputFlag & 1)	//Вывод произведения при соответствующем флаге
			printf("\nПроизведение нечётных элементов: %.2lf\n", ProductOfOddElements(&m1));

		if (outputFlag & 2)	//Вывод на экран при соответствующем флаге
		{
			puts("\nПолученная матрица:");
			PrintMatrix(&m1, stdout);
		}

		if (outputFlag & 4)	//Вывод в текстовый файл при соответствующем флаге
		{
			printf("Введите название файла для записи: ");
			scanf("%32s", fname);
			if (PrintIntoFile(&m1, fname))
			{
				printf("Не удалось открыть файл %s! Завершение работы\n", fname);
				return -1;
			}
			return 0;
		}

		if (outputFlag & 8)	//Вывод в бинарный файл при соответствующем флаге
		{
			printf("Введите название файла для записи: ");
			scanf("%32s", fname);
			if (WriteIntoBinary(&m1, fname))
			{
				printf("Не удалось открыть файл %s! Завершение работы\n", fname);
				return -1;
			}
		}
		return 0;

	};
};

//Заполнение матрицы с клавиатуры
int8_t ReadFromKeyboard(Matrix* m)
{
	char buff[BUFF];				//Объявление буфера текста
	do
	{
		puts("Введите размер матрицы, количество строк и столбцов:");
		fgets(buff, BUFF - 1, stdin);	//Чтение размеров матрицы
	} while (SetMatrixSize(m, buff));		//Ввод размера будет прододжаться, пока не будет введёно корректное значение

	printf("Поочерёдно введите %d значений\n", (m->hsize * m->vsize));
	ReadFromText(m, stdin);			//Чтение

	//Проверка на лишний ввод
	char g;
	while (g = getchar())
	{
		if (g == '\n')				//Если после чисел был нажат Enter, проверку можно прекратить
			return 0;
		if (g != ' ' && g != '\t')	//Если после чисел был мусор, можно прекратить проверку и отправить флаг ошибки
			return -1;
		//Если был встречен пробел, проверка продолжается по новой
	}
}

//Чтение из бинарного файла
int8_t ReadFromBinary(Matrix* m, const char fname[])
{
	FILE* out = fopen(fname, "rb");		//Открытие файла; проверка, открылся ли он
	if (out == NULL)
		return -1;						//Возврат флага при неудачной попытке открыть файл

	fread(m, sizeof(Matrix), 1, out);	//Чтение из файла
	fclose(out);
	return 0;
}

//Запись матрицы в бинарный файл
int8_t WriteIntoBinary(Matrix* m, const char fname[])
{
	FILE* out = fopen(fname, "wb");	//Открытие файла; проверка, открылся ли он
	if (out == NULL)
		return -1;					//Возврат флага при неудачной попытке открыть файл

	fwrite(m, sizeof(Matrix), 1, out); //Запись в файл
	fclose(out);
	return 0;
}

//Вывод в текстовый файл
int8_t PrintIntoFile(Matrix* m, const char fname[])
{
	FILE* out = fopen(fname, "wt");	//Открытие файла; проверка, открылся ли он
	if (out == NULL)
		return -1;					//Возврат флага при неудачной попытке открыть файл

	fprintf(out, "%d %d\n", m->hsize, m->vsize);	//Вывод размера матрицы в файл
	PrintMatrix(m, out);						//Вывод самой матрицы в файл
	fclose(out);
	return 0;
}

//Чтение элементов матрицы
void ReadFromText(Matrix* m, FILE* source)
{
	for (int i = 0; i < m->vsize; ++i)				//Перебор строк
		for (int j = 0; j < m->hsize; ++j)			//Перебор столбцов в строках
			fscanf(source, "%lf", &m->nums[i][j]);	//Чтение каждого значения
}

//Чтение матрицы из некоторого текстового файла
int8_t ReadFromTextFile(Matrix* m, const char fname[])
{
	char buff[BUFF];

	FILE* source = fopen(fname, "rt");
	if (!source)
		return -1; //Возврат флага, если не открылся файл

	fgets(buff, BUFF - 1, source);	//Чтение заголовка файла с размерами матрицы
	if (SetMatrixSize(m, buff))
	{								//Закрытие файла и возврат флага, если размеры матрицы не были считаны
		fclose(source);
		printf("Не удалось открыть файл %s! Завершение работы\n", fname);
		return -1;
	}

	ReadFromText(m, source);			//Чтение

	//Проверка на лишний ввод
	char g;
	while (g = fgetc(source))
	{
		if (g == EOF)				//Если после чисел был встречен конец файла, проверку можно прекратить
		{
			fclose(source);
			return 0;
		}
		if (g != ' ' && (g != '\t' && g != '\n'))	//Если после чисел был мусор, можно прекратить проверку и отправить флаг ошибки
		{
			fclose(source);
			puts("Файл содержит лишние символы! Завершение работы!");
			return -1;
		}
		//Если был встречен пробел, табуляция, или новая строка, проверка продолжается по новой
	}
}

//Заполнение матрицы по формуле
int8_t FormulaSolution(Matrix* m)
{
	char buff[BUFF];//Объявление буфера текста
	puts("Введите размер матрицы, количество строк и столбцов:");
	fgets(buff, BUFF - 1, stdin);
	if (SetMatrixSize(m, buff))
	{
		puts("Неправильно указан размер матрицы. Завершение работы");
		return -2;
	}
	FillWithFormula(m);
	return 0;
}

//Заполнение матрицы случайными числами
int8_t RandomSolution(Matrix* m)
{
	Range r;		//Объявление диапазона чисел
	char buff[BUFF];//Объявление буфера текста
	puts("Введите размер матрицы, количество строк и столбцов:");
	fgets(buff, BUFF - 1, stdin);
	if (SetMatrixSize(m, buff))
	{
		puts("Неправильно указан размер матрицы. Завершение работы");
		return -2;
	}
	puts("Введите диапазон, из которого будет заполнятся матрица:");
	if (scanf("%d %d", &r.a, &r.b) == 2)
	{
		FillWithRandom(m, &r);
		return 0;
	}
	else
	{
		puts("Неправильно указан диапазон. Завершение работы");
		return -1;
	}
}

//Устанавливает размер матрицы из текстового источника
char SetMatrixSize(Matrix* Matrix, const char text[])
{
	if (sscanf(text, "%d %d", &Matrix->hsize, &Matrix->vsize) != 2)
		return -1;
	if (Matrix->hsize <= 0 || Matrix->vsize <= 0)
		return -2;
	if (Matrix->hsize >= HMAX || Matrix->vsize >= HMAX)
		return -3;

	return 0;
}

//Функция выбора способа заполнения
char SelectSource(void)
{
	printf("Выберите способ наполнения массива\n%s%s%s%s%s%s",
		"r -- для заполнения случайными числами\n",
		"a -- для автозаполнения согласно формуле\n",
		"f -- для чтения массива из бинарного файла\n",
		"k -- для ручного наполнения\n",
		"t -- для чтения массива из текстового файла\n",
		"любую другую клавишу для завершения работы: ");

	char buff[BUFF];
	fgets(buff, BUFF - 1, stdin);	//Запись в буфер с ввода
	return buff[0];					//Возврат символа
}

//Заполнение матрицы случайными числами из диапазона
void FillWithRandom(Matrix* Matrix, Range* Range)
{
	srand(time(NULL)); //создаёт псевдослучайный сид
	if (Range->a > Range->b)	//"Переворачивание" диапазона, если он задан наоборот
	{
		int tmp = Range->b;
		Range->b = Range->a;
		Range->a = tmp;
	}
	for (int i = 0; i < Matrix->vsize; ++i)		//Перебор строк
		for (int j = 0; j < Matrix->hsize; ++j) //Перебор столбцов в строках
			Matrix->nums[i][j] = rand() % (Range->b - Range->a) + Range->a;
}

//Заполнение матрицы согласно формуле
void FillWithFormula(Matrix* Matrix)
{
	for (int i = 0; i < Matrix->vsize; ++i)		//Перебор строк
		for (int j = 0; j < Matrix->hsize; ++j) //Перебор столбцов в строках
		{
			if (i > j)
				Matrix->nums[i][j] = i + j;
			else if (i == j)
				Matrix->nums[i][j] = 1;
			//Matrix->nums[i][j] = i / (j + 1);
			else //if i < j
				Matrix->nums[i][j] = i - j;
		}
}

//Произведение нечётных элементов матрицы
double ProductOfOddElements(Matrix* Matrix)
{
	double sum = 1.0;
	for (int i = 0; i < Matrix->vsize; ++i)		//Перебор строк
		for (int j = 0; j < Matrix->hsize; ++j) //Перебор столбцов в строках
			if ((i + j) % 2)
				sum *= Matrix->nums[i][j];
	return sum;
}

//Вывод матрицы в текстовом виде в данный исток
void PrintMatrix(Matrix* Matrix, FILE* sink)
{
	for (int i = 0; i < Matrix->vsize; ++i)		//Перебор строк
	{
		for (int j = 0; j < Matrix->hsize; ++j)	//Перебор столбцов в строках
			fprintf(sink, "%6.2lf ", Matrix->nums[i][j]);
		fputc('\n', sink);
	}
	fputc('\n', sink);
}

//Осуществление выбора, что и куда будет выводиться 
uint8_t SelectOutput(void) //UNDONE
{
	char c = 10;
	uint8_t flag = 0;

	printf("Чтобы вывести произведение нечётных элеметов матрицы, введите y: ");
	while ((c = fgetc(stdin)) == '\n'); //Ввод будет игнорировать новые строки, оставшиеся с прошлого ввода
	if (c == 'y')
		flag += 1;

	printf("Чтобы вывести матрицу на экран, введите y: ");
	while ((c = fgetc(stdin)) == '\n'); //Ввод будет игнорировать новые строки, оставшиеся с прошлого ввода
	if (c == 'y')
		flag += 2;

	printf("Чтобы вывести матрицу на текстовый файл, введите t.\n%s",
		"Чтобы вывести матрицу в бинарный файл, введите b: ");
	while ((c = fgetc(stdin)) == '\n');
	if (c == 't')
		return flag += 4;
	if (c == 'b')
		flag += 8;

	return flag;
}
