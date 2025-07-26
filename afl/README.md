# AFL++ fuzzing exediff

## build instrumented executable

```sh
# pwd is exediff/afl
CC=/usr/bin/afl-cc cmake ..
mkdir output
cmake --build .. -j
cp ../exediff .
```

## start fuzzing

```sh
afl-fuzz -i input -o output -- ./exediff @@ input/test
```

## run failed case

```sh
../exediff output/default/crashes/$SOMEFILE input/test
```

## to check if the output is right against diff

```sh
python flatten.py input/test > reference
python flatten.py output/default/crashes/$SOMEFILE > crash
diff -u crash reference
```
