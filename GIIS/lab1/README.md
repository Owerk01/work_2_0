# В адекватном состоянии не запускать

Установлен cmake и QT 6.10.2

```bash
#
mkdir build
cd build
# вместо path - свой путь до gcc_64, например: /home/n25/Qt/6.10.2/gcc_64
cmake .. -DCMAKE_PREFIX_PATH=path
cmake --build .
# \/ запуск
./super_editor
```