#include <math.h>
#include <stdio.h>
#include <string.h>
#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>
void usleep(__int64 usec)
{
    HANDLE timer;
    LARGE_INTEGER ft;

    ft.QuadPart = -(10 * usec); // Convert to 100 nanosecond interval, negative value indicates relative time

    timer = CreateWaitableTimer(NULL, TRUE, NULL);
    SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
    WaitForSingleObject(timer, INFINITE);
    CloseHandle(timer);
}
#endif

// глобальные переменные для рендеринга
float A = 0, B = 0, C = 0;

float cubeWidth = 20;
int width = 160, height = 44;
float zBuffer[160 * 44];
char buffer[160 * 44];
int backgroundASCIICode = '.';  // символ фона
int distanceFromCam = 100;
float K1 = 40;                  // коэффициент масштаба
float incrementSpeed = 0.4;      // шаг для кубов

float x, y, z;
float ooz;
int xp, yp;
int idx;

// функции для оптимизации вычислений вращения куба
float cosA, sinA, cosB, sinB, cosC, sinC;

// функции для расчета координат
float calculateX(int i, int j, int k) {
    return j * sinA * sinB * cosC - k * cosA * sinB * cosC +
           j * cosA * sinC + k * sinA * sinC + i * cosB * cosC;
}

float calculateY(int i, int j, int k) {
    return j * cosA * cosC + k * sinA * cosC -
           j * sinA * sinB * sinC + k * cosA * sinB * sinC -
           i * cosB * sinC;
}

float calculateZ(int i, int j, int k) {
    return k * cosA * cosB - j * sinA * cosB + i * sinB;
}

void calculateForSurface(float cubeX, float cubeY, float cubeZ, int ch) {
    // предварительные расчеты для каждой поверхности куба
    x = calculateX(cubeX, cubeY, cubeZ);
    y = calculateY(cubeX, cubeY, cubeZ);
    z = calculateZ(cubeX, cubeY, cubeZ) + distanceFromCam;

    ooz = 1 / z; // обратная глубина для Z-буфера

    // Вычисление позиции на экране
    xp = (int)(width / 2 + K1 * ooz * x * 2);
    yp = (int)(height / 2 + K1 * ooz * y);

    // Проверка на выход за границы экрана
    if (xp >= 0 && xp < width && yp >= 0 && yp < height) {
        idx = xp + yp * width;
        if (ooz > zBuffer[idx]) {
            zBuffer[idx] = ooz;
            buffer[idx] = ch;
        }
    }
}

int main() {
    printf("\x1b[2J"); // очистка экрана
    while (1) {
        // предварительные вычисления для тригонометрических функций
        cosA = cos(A); sinA = sin(A);
        cosB = cos(B); sinB = sin(B);
        cosC = cos(C); sinC = sin(C);

        // очистка буферов
        memset(buffer, backgroundASCIICode, width * height);
        memset(zBuffer, 0, width * height * sizeof(float));

        cubeWidth = 20;

        // рендеринг куба в центре экрана
        for (float cubeX = -cubeWidth; cubeX < cubeWidth; cubeX += incrementSpeed) {
            for (float cubeY = -cubeWidth; cubeY < cubeWidth; cubeY += incrementSpeed) {
                // каждая сторона куба
                calculateForSurface(cubeX, cubeY, -cubeWidth, '@');   // передняя сторона
                calculateForSurface(cubeWidth, cubeY, cubeX, '$');    // правая сторона
                calculateForSurface(-cubeWidth, cubeY, -cubeX, '~');  // левая сторона
                calculateForSurface(-cubeX, cubeY, cubeWidth, '#');   // задняя сторона
                calculateForSurface(cubeX, -cubeWidth, -cubeY, ';');  // нижняя сторона
                calculateForSurface(cubeX, cubeWidth, cubeY, '+');    // верхняя сторона
            }
        }

        // вывод на экран
        printf("\x1b[H");
        for (int k = 0; k < width * height; k++) {
            putchar(k % width ? buffer[k] : 10);
        }

        // обновление углов вращения
        A += 0.05;
        B += 0.05;
        C += 0.01;
        usleep(4000 * 2); // задержка
    }
    return 0;
}