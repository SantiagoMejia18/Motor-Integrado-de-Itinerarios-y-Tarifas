# Motor-Integrado-de-Itinerarios-y-Tarifas

SI1002-7 Lenguajes y Paradigmas de la Computación — Universidad EAFIT — 2026-2 — Segunda Práctica (20%)

## Integrantes

- Santiago Mejia
- Mariana Diaz

## Cómo compilar y ejecutar

```bash
g++ -std=c++17 -Wall -Wextra -o practice main.cpp
./practice
```

El archivo compila sin errores y sin advertencias con estas opciones. No lee nada de la entrada
estándar. En OnlineGDB: seleccionar C++17 como lenguaje, pegar `main.cpp` y presionar Run.

## Preguntas de la defensa

### Pregunta 1 — ¿Qué pasa si el destructor de `TransportMode` no es virtual?

`Network` es dueña de sus modos a través de `std::unique_ptr<TransportMode>`. Cuando la red se
destruye, cada objeto se elimina por medio de un puntero a la clase base, aunque su tipo real sea
`MetroLine`, `TouristCable`, etc.

- Si `~TransportMode()` es virtual, la llamada se resuelve dinámicamente: primero se ejecuta el
  destructor de la clase más derivada y luego el de cada clase base, hasta `TransportMode`.
- Si no es virtual, eliminar un objeto derivado a través de un puntero a la base es
  **comportamiento indefinido** según el estándar de C++. En la práctica solo se ejecuta
  `~TransportMode()`: la parte derivada nunca se destruye y cualquier recurso que posea se pierde
  (por ejemplo, un atributo `std::string` agregado a `TouristCable` nunca se liberaría).

Por eso el destructor virtual es un requisito de correctitud y no una formalidad: toda clase que se
use de forma polimórfica y se elimine a través de un puntero a la base debe declararlo. Lo mismo
aplica a `Passenger`.
