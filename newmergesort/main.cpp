#include <iostream>
#include "optimized_thread.h"

// Функция для обмена элементов
void swap(int* a, int* b) {
    int t = *a;
    *a = *b;
    *b = t;
}

// Функция для разделения массива и выбора опорного элемента
int partition(std::vector<int> &arr, int low, int high) {
    int pivot = arr[high]; // Опорный элемент
    int i = (low - 1); // Индекс меньшего элемента

    for (int j = low; j <= high - 1; j++) {
        // Если текущий элемент меньше или равен опорному
        if (arr[j] <= pivot) {
            i++; // Увеличиваем индекс меньшего элемента
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[high]);
    return (i + 1);
}


// Однопоточная функция быстрой сортировки
 void quicksort(std::vector<int>& arr, int low, int high) {
    if (low < high) {
        // Разделение массива на две части
        int pi = partition(arr, low, high);

        // Рекурсивный вызов функции quicksort для двух частей массива
        quicksort(arr, low, pi - 1);
        quicksort(arr, pi + 1, high);
    }
}

 //void quicksort_pre(std::vector<int> arr, int low, int high) { std:vector<int>(& arr_2) = arr; quicksort(arr_2, low, high); }


int main()
{
    std::vector<int> massive(18);
    std::vector<std::vector<int>> arrays;//хранит массивы под сортировку
    // Заполнение массива случайными числами
    for (int i = 0; i < massive.size(); ++i) {
        massive[i] = rand() % 20;
    }
    int size = massive.size();
    int i = 0;
    /*while (i < massive.size())//прокатываем пока не кончатся члены массива
    {
        if (size > 10)
        {
            std::vector<int> array;
            for (int j = 0; j < 10; ++j)
            {
                array.push_back(massive[i]);
                i++;
                size--;
            };
            arrays.push_back(array);
            
        }
        else
        {
            
            std::vector<int> array;
            for (int j = 0; j < size; ++j)
            {
                array.push_back(massive[i]);
                i++;
            };
            arrays.push_back(array);
        };

    };*/

    
    //однопоточная сортировка
    quicksort(massive, 0, massive.size() - 1);
    for (int j = 0; j < massive.size(); j++){ std::cout << massive[j] << " "; }

    //сортировка в пуле
    RequestHandler2 rh;

   rh.push_task(quicksort, arrays[0],0, arrays[0].size()-1);
   

    //for (int j = 0; j < arrays.size(); j++) { for (int k = 0; j < arrays[j].size(); k++) {cout} }

    return 0;
}