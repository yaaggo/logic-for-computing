@echo off

cls

echo Compilando o projeto...

if not exist "build" (
    echo Criando pasta build...
    mkdir build
)

cd build

cmake ..

cmake --build .

if %errorlevel% equ 0 (
    echo Compilacao concluida com sucesso!
    echo Executando o programa...
    cls
    .\Debug\logic-eval.exe
) else (
    echo Erro na compilacao. Verifique os erros acima.
)

pause