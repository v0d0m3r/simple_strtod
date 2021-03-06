#ifndef SIMPLE_STRTOD_H
#define SIMPLE_STRTOD_H

/******************************************************************************/
/* Данная функция интерпритирует число с плавающей точкой из ASCII-строки
(Нуль-терминированной)
Состав входной строки:
- (необязательный) знак плюс и минус
- непустая последовательность десятичный цифр, необязательно содержащих
маркер десятичной дроби (для нашего случая символ '.')
- (необязательный) символ e или E (показатель степени), за которым следует
необязательный знак минус или плюс и непустая
последовательность десятичных цифр

Возвращаемое значение:
В случае успеха значение числа с плавающей запятой
Если сконвертированное значение не входит в диапазон требуемого типа диапазона
(для простоты, если значение экспоненты превышает 308),
тогда возвращается NAN и errno == ERANGE

TODO: выполнить больше отладки и написать тесты

Внимание: данная функция реализована только в учебных целях для
демонстрации относительно простой математики и простого распарсивания.
Не учтены понятия как -+Inf, nan во входной строки, проблемы потери точности
при арифметики для типа double и др. Для более близкого рассмотрения
вопроса, мне понравилась статья http://krashan.ppa.pl/articles/stringtofloat/
*/
double simple_strtod(const char* str);

/******************************************************************************/

#endif // SIMPLE_STRTOD_H
