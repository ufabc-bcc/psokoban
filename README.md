# psokoban

Psokoban is a parallel brute force solver for Sokoban

## Relatório

O relatório está disponível dentro deste projeto e pode ser acessado aqui.

## Compile

Na raiz do projeto execute o seguinte comando

```bash
gcc -O3 -g -Wall -fopenmp -o psokoban.out src/main.c
```

## Rode

```bash
./psokoban.out < levels/level_(-1|00|01|A) <num_thread>
```

## Avalie o tempo de execução

```bash
time ./psokoban.out < levels/level_(-1|00|01|A) <num_thread>
```

## Considerações

Este algoritmo é baseado na implementação em C# do Rosetta Code que pode ser vista [aqui](https://rosettacode.org/wiki/Sokoban#C.23).

