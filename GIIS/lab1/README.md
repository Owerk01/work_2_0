Assuming you have cmake and QT 6.10.2 installed.
In CMakePresets.json configure path to your QT (for example: /home/n25/Qt/6.10.2/gcc_64).

Build:

```bash
mkdir build
cmake --preset qt6
cmake --build --preset qt6
```

Execute program:

```bash
./build/editoR
```
