## ODXT C++ Multi-threade implementation with Redis back-end

Change the database name variable in ```main.cpp``` (or ```main_single_thread.cpp```) file. The database file should formatted as an inverted-index with the first column containing keywords as 4-byte hex values, and the next columns containing associated document identifiers as 4-byte hex values (all separated by commas, even at the end).


Compile the single threaded ODXT impementation
```bash
make single_thread
```

Clean the project
```bash
make clean
```
This does not flush the Redis database. Do this manually from ```redis-cli```.
