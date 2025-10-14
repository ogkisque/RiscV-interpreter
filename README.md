# RiscV interpreter

Интерпретатор riscv32 с реализацией наборов инструкций I, M, F, Zbb.

## Сборка и запуск

```
cmake -S . -B build
cmake --build build
./build/src/main <test_name>
```

## Unit тесты

```
cmake -S . -B build -DWITH_TESTS=1 
cmake --build build --target tests
./build/tests/tests
```

## End to end тесты

```
cd end2end_tests
make
make test INTERPRETER=<>
```