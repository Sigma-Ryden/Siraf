#include <fstream> // ifstream, ofstream
#include <iostream> // cout

#include <Siraf/Core.hpp> // ReadArchive, WriteArchive
#include <Siraf/Memory/Span.hpp>

using siraf::iarchive;
using siraf::oarchive;

using siraf::common::span;

void print(int** arr, int h, int w)
{
    for (int i = 0; i < h; ++i, std::cout << '\n')
        for (int j = 0; j < w; ++j)
            std::cout << arr[i][j] << ' ';

    std::cout << '\n';
}

void test_scope()
{
    {
        std::ofstream file("test_scope.bin", std::ios::binary);

        if (not file.is_open()) return;

        auto ar = oarchive(file);

        int height = 3;
        unsigned int width = 4;

        int** tensor = new int* [height];
        for (int i = 0; i < width; ++i)
            tensor[i] = new int [width] { i, i + 1, i + 2, i + 3 };

        print(tensor, height, width);
        auto r = siraf::memory::make_span(tensor, height, width);

        try
        {
            ar & span(tensor, height, width); // or just using: span(ar, tensor, height, width);
        }
        catch (const char* e)
        {
            std::cout << e << '\n';
            return;
        }
        // implicit delete pointer
    }
    {
        std::ifstream file("test_scope.bin", std::ios::binary);

        if (not file.is_open()) return;

        auto ar = iarchive(file);

        int height;
        int width;

        int** tensor = nullptr;

        try
        {
            ar & span(tensor, height, width);
        }
        catch (const char* e)
        {
            std::cout << e << '\n';
            return;
        }

        print(tensor, height, width);

        // implicit delete pointer
    }
}

int main()
{
    test_scope();

    return 0;
}
