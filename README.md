# Quack Lang
<img src="GoldDuckIcon.png" style="width : 150px"> Quack Quack !1!

---

### Quack is the programming language developed for the GoldDuck Engine.

Please wait for building this repo...

## Quick Start

Clone the repo:
```bash
git clone https://github.com/ItIsMrNJ/quack-lang.git
cd quack-lang/src
```

Compile (requires a C++17 compiler):
```bash
g++ -std=c++17 -o quack quack.cpp
```

Run a script (a plain text file, any extension — `.gde` used here by convention):
```bash
quack script.gde
```

Example `script.gde`:
```
let name = "GoldDuck";
quack("Hello, " + name + "!");
```

## License

MIT License
