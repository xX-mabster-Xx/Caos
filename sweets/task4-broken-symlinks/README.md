Программе в аргументах командной строки передаются пути к файлам. Каждый путь PATH
            следует напечатать на stdout в следующем виде:
        


            
1) `PATH (missing)`, если файл не существует;
            
2) `PATH (broken symlink)`, если файл является символической ссылкой,
                указывающей на несуществующий путь;
            
3) `PATH` во всех остальных случаях.
        

        

Например:
        

```

$ ln -s /non/existent symlink_bad
$ ln -s regular symlink_good
$ touch regular
$ ./yourprog regular symlink_good symlink_bad /non/existent
regular
symlink_good
symlink_bad (broken symlink)
/non/existent (missing)
```
    
