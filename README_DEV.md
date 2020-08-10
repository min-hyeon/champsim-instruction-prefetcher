# Branches and task flow

Git graph [2020-08-10]

```
master
 +- develop
     +- feature/structure
         +- feature/structure/graph
         +- feature/structure/buffer
     +- feature/hasher
         +- feature/hasher/debug
```

Plan for merge

```
feature/structure/graph
    -> feature/structure
feature/structure/buffer
    -> feature/structure
feature/hasher/debug
    -> *
feature/structure
    -> develop
feature/hasher
    -> develop
```