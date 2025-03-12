# Слоты

## Инструкция по сборке

### Необходимо

- CMake
- система сборки, поддерживаемая генератором CMake
- компилятор, поддерживающий стандарт языка C++20

### Шаги

1. Открыть консоль в папке проекта
2. Создать папку __`/build`__ и перейти в неё

    ```cmd
    mkdir build
    cd build
    ```

3. Сконфигурировать проектные файлы системы сборки используя CMake (при необходимости указав предпочитаемые систему сборки и компилятор)

      ```cmd
      cmake ..
      ```

4. Собрать проект в конфигурации и установить нужные файлы в папку __`/bin`__

    ```cmd
    cmake --build . --target install --
    ```

5. Перейти в папку __`/bin`__ и запустить приложение

    ```cmd
    cd ../bin
    ./EffectiveMobile.exe
    ```
